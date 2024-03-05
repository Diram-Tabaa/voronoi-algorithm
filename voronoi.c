/**
 * @file voronoi.c
 * @author Diram Tabaa (dtabaa@andrew.cmu.edu)
 * @brief 
 * @version 0.1
 * @date 2024-02-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "voronoi.h"


/***************/
/* BOUNDARY    */
/***************/

void init_boundary(boundary_t* bound, double left_x, double left_y, 
                   double right_x, double right_y, char label) {
    bound->left_point.x = left_x;
    bound->right_point.x = right_x;
    bound->left_point.y = left_y;
    bound->right_point.y = right_y;
    bound->label = label;
}


boundary_t* new_boundary(double left_x, double left_y, double right_x,
                         double right_y, char label) {
    boundary_t* bound;
    if (!(bound = malloc(sizeof(boundary_t)))) return NULL;
    init_boundary(bound, left_x, left_y, right_x, right_y, label);
    return bound;
}



void boundary_print(void* elem) {
    boundary_t* bound = (boundary_t*) elem;
    printf("(%f, %f) -- (%f, %f) %p\n", bound->left_point.x,
            bound->left_point.y, bound->right_point.x, 
            bound->right_point.y, elem);
}

/***********/
/*  EVENTS */
/***********/

void init_event(event_t* e, char label, double x, double y, point_t* left, 
                point_t* mid, point_t* right) {
    e->label = label;
    e->sweep_event.x = x;
    e->sweep_event.y = y;
    e->triplet.left = left;
    e->triplet.mid = mid;
    e->triplet.right = right;
}

event_t* new_event(char label, double x, double y, point_t* left, 
                point_t* mid, point_t* right) {
    event_t* e;
    if ((e = malloc(sizeof(event_t))) == NULL) return NULL;
    init_event(e, label, x, y, left, mid, right);
    return e;
}

int is_circle_event_old(event_t* e, bst_t* beachline, double sweep) {
    boundary_t bound, *left = NULL, *right = NULL;
    point_t* focus;
    double beachline_y;
    void* arg = DOUBLE2VOID(sweep);
    init_boundary(&bound, e->sweep_event.x, sweep,
                         e->sweep_event.x, sweep, SINGLETON);
    bst_interval(beachline, &bound, (void**) &left,
                 (void**) &right, arg);

    if (left) focus = &left->right_point;
    if (right) focus = &right->left_point;

    beachline_y = compute_parabola_value(focus, sweep, e->sweep_event.x);
    circle_t v;
    compute_circumcircle(e->triplet.left, e->triplet.mid, e->triplet.right, &v);

    //printf("BE %f ACt %f\n", beachline_y, v.center.y);
    return v.center.y > beachline_y;
}


int is_circle_event_stale(event_t* e, bst_t* beachline, double sweep) {
    assert(e->label == CIRCLE_EVENT);

    boundary_t bound;
    boundary_t *left, *right;
    point_t left_point, right_point, mid_point;
    void* arg = DOUBLE2VOID(sweep);


    /* in order to test that the circle event is still valid, we compute 
       an arbitrary middle point betweeen the intersections of the three
       arcs that are about to meet at a point */
    compute_arc_intersection(e->triplet.left, e->triplet.mid,
                             sweep, &left_point);
    compute_arc_intersection(e->triplet.mid, e->triplet.right,
                             sweep, &right_point);
    compute_midpoint(&left_point, &right_point, &mid_point);

    /* the idea then is to test that this midpoint indeed lies between the two
       arcs on the beachline, otherwise it means it is no longer valid*/
    init_boundary(&bound, mid_point.x, mid_point.y, 
                          mid_point.x, mid_point.y, SINGLETON);
    bst_interval(beachline, &bound, (void**) &left,
                 (void**) &right, arg);
    // printf("___\n");
    //boundary_print(left);
    //boundary_print(right);
    //printf("___\n");
    return (!point_equality(&left->right_point, e->triplet.mid) ||
            !point_equality(&right->left_point, e->triplet.mid) ||
            !point_equality(&left->left_point, e->triplet.left) ||
            !point_equality(&right->right_point, e->triplet.right));
}


int event_compare(void* e1, void* e2) {
    event_t* event1 = (event_t*) e1;
    event_t* event2 = (event_t*) e2;
    if (event1->sweep_event.y > event2->sweep_event.y) return 1;
    return -1;
}

/**
 * @brief 
 * 
 * @param elem1 
 * @param elem2 
 * @param args 
 * @return int 0 if (elem1 == elem2), 1 if (elem1 < elem2) and -1 if (elem1 > elem2)
 */
int beachline_compare(void* elem1, void* elem2, void* args) {
    point_t intersect1, intersect2;
    boundary_t* bound1 = (boundary_t*) elem1;
    boundary_t* bound2 = (boundary_t*) elem2;
    double* t = (double*) &args;
    double sweepline =  *t;
    if (bound1->label == SINGLETON) {
        intersect1.x = bound1->left_point.x;
    } else {
        compute_arc_intersection(&bound1->left_point, &bound1->right_point,
                             sweepline, &intersect1);
    }

    if (bound2->label == SINGLETON) {
        intersect2.x = bound2->left_point.x;
    } else {          
        compute_arc_intersection(&bound2->left_point, &bound2->right_point,
                             sweepline, &intersect2);
    }
    //printf("INTERSECTS %f %f\n", intersect1.x, intersect2.x);
    if (intersect1.x == intersect2.x) return 0;
    return SYMMETRIC_LEQ(intersect1.x, intersect2.x);
}

int process_site(bst_t* beachline, pqueue_t* events,
                     point_t* site, double sweep) {
    boundary_t temp;
    boundary_t *left, *right, *new_bound;

    double arc_x, arc_y;
    point_t temp_point;
    event_t *event_left, *event_right;

    void* arg = DOUBLE2VOID(sweep);

    init_boundary(&temp, site->x, site->y, site->x, site->y, SINGLETON);
   
    /* if bst insert return -1, means first boundary */
    if (bst_interval(beachline, &temp, (void**) &left, (void**) &right, arg)) {
        return -1;
    }

    /* INVARIANT: if left and right not NULL, left->right == right->left */


    if (left != NULL) {
        arc_x = left->right_point.x;
        arc_y = left->right_point.y;

        compute_circle_tangent(&left->left_point, &left->right_point,
                            site, &temp_point);

        event_left = new_event(CIRCLE_EVENT, temp_point.x, temp_point.y,
                            &left->left_point, &left->right_point, site);
        pqueue_insert(events, (void*) event_left);
    }
    
    if (right != NULL) {

        arc_x = right->left_point.x;
        arc_y = right->left_point.y;

        compute_circle_tangent(&right->left_point, &right->right_point,
                            site, &temp_point);

        event_right = new_event(CIRCLE_EVENT, temp_point.x, temp_point.y,
                            site, &right->left_point, &right->right_point);
        pqueue_insert(events, (void*) event_right);
    }
    

    new_bound = new_boundary(arc_x, arc_y, site->x, site->y, INTERSECT);
    bst_insert(beachline, new_bound, arg);
    new_bound = new_boundary(site->x, site->y, arc_x, arc_y, INTERSECT);
    bst_insert(beachline, new_bound, arg);
    //bst_print(beachline, *boundary_print);
    return 0;
}

int process_circle_event(bst_t* beachline, pqueue_t* events, event_t* e, 
                         double sweep) {
    circle_t voronoi_vertex;
    boundary_t left, right, *dummy, *new_left, *new_right, *new_bound;
    point_t *leftp, *midp, *rightp;
    event_t *event_left, *event_right;
    point_t temp_point;
    void* arg = DOUBLE2VOID(sweep);

    /* if the circle event is stale, i.e. another site came before in between,
        then just ignore */
    //is_circle_event_old(e, beachline, sweep);
   if (is_circle_event_old(e, beachline, sweep)) return -1;

    leftp = e->triplet.left;
    rightp = e->triplet.right;
    midp = e->triplet.mid;
    //printf("IT IS A CIRCLE EVENT\n");
    //point_print(leftp, "THE LEFT POINT");
    //point_print(midp, "THE MIDDLE POINT (REMOVAL)");
    //point_print(rightp, "THE RIGHT POINT");
    /* computes the vertex that is to be added to the voronoi diagram */
    compute_circumcircle(leftp, midp, rightp, &voronoi_vertex);

    /* we need to remove the two pairs containing the middle point since
      the arc between has now dissolved */
    init_boundary(&left, leftp->x, leftp->y, midp->x, midp->y, INTERSECT);
    init_boundary(&right, midp->x, midp->y, rightp->x, rightp->y, INTERSECT);
    bst_delete(beachline, (void*) &left, arg);
    bst_delete(beachline, (void*) &right, arg);
    
    /* inserting the new pair (arc intersection) after the middle point is 
      removed, there is only one such pair */

    /* now for the left and right points, we need to find their left and right
      points respectively, so that we add the two potential circle events*/
    init_boundary(&left, e->sweep_event.x - EPSILON, sweep,
                         e->sweep_event.x - EPSILON, sweep, SINGLETON);
    init_boundary(&right, e->sweep_event.x + EPSILON, sweep,
                          e->sweep_event.x + EPSILON, sweep, SINGLETON);

    /* we only care about the left boundary */
    bst_interval(beachline, &left, (void**) &new_left,
                 (void**) &dummy, arg);
    /* we only care about the right boundary */
    bst_interval(beachline, &right, (void**) &dummy,
                (void**) &new_right, arg);
    
    new_bound = new_boundary(leftp->x, leftp->y, rightp->x, rightp->y,
                             INTERSECT);
    bst_insert(beachline, new_bound, arg);
    //boundary_print(new_left);
    //printf("%f %f\n", midp->x, midp->y);
    /* if the neighbouring left actually exists and that is not the 
       midpoint itself, then add a new circle event */
    if (new_left && !point_equality(&new_left->left_point, midp)) {

        compute_circle_tangent(&new_left->left_point, &new_left->right_point,
                            rightp, &temp_point);
        //printf("FOR THE NEIGHBOUR LEFT\n");
        //point_print(&new_left->left_point, "LEFTMOST");
        //point_print(&new_left->right_point, "MID");
        //point_print(rightp, "RIGHTMOST");
        //printf("new %f %f\n", temp_point.y, sweep);
        if (temp_point.y < sweep) {
        event_left = new_event(CIRCLE_EVENT, temp_point.x, temp_point.y,
                            &new_left->left_point, &new_left->right_point,
                            rightp);
        pqueue_insert(events, (void*) event_left);  
        }
    }



     /* if the neighbouring right actually exists and that is not the 
       midpoint itself, then add a new circle event */
    if (new_right && !point_equality(&new_right->right_point, midp)) {

        compute_circle_tangent(leftp, &new_right->left_point,
                            &new_right->right_point, &temp_point);
        
       //printf("FOR THE NEIGHBOUR RIGHT\n");
        //point_print(leftp, "LEFTMOST");
        //point_print(&new_right->left_point, "MID");
        //point_print(&new_right->right_point, "RIGHTMOST");

        //printf("new %f %f\n", temp_point.y, sweep);
        if (temp_point.y < sweep) {
        event_right = new_event(CIRCLE_EVENT, temp_point.x, temp_point.y,
                            leftp, &new_right->left_point,
                            &new_right->right_point);
        pqueue_insert(events, (void*) event_right);  
        }
    }

    printf("v (%f, %f)\n", voronoi_vertex.center.x, 
                        voronoi_vertex.center.y);
    return 0;
}

int main(int argc, char** argv) {

    bst_t *tree = bst_new(*beachline_compare);
    boundary_t *p, *p2, *p3, *p4;
    double sweep = -1.501;
    void* arg = DOUBLE2VOID(sweep);
    pqueue_t* pq;
    event_t* etmp;
    pq = pqueue_new(*event_compare);


    p = new_boundary(7, -1, 1, -1.5, INTERSECT);
    bst_insert(tree, (void*) p, arg);
    p = new_boundary(1, -1.5, 7, -1, INTERSECT);
    bst_insert(tree, (void*) p, arg);

    bst_print(tree, *boundary_print);
    pqueue_insert(pq, new_event(SITE_EVENT, 2.4, -5.45, NULL, NULL, NULL));
    pqueue_insert(pq, new_event(SITE_EVENT, 3.3, -5.83, NULL, NULL, NULL));
    pqueue_insert(pq, new_event(SITE_EVENT, 4.2, -5.55, NULL, NULL, NULL));

    while (pqueue_size(pq) > 0) {
        pqueue_pop(pq, &etmp);
        sweep = etmp->sweep_event.y;
        if (etmp->label == SITE_EVENT) {
           printf("SITE (%f, %f)\n", etmp->sweep_event.x, etmp->sweep_event.y);
            process_site(tree, pq, &etmp->sweep_event, sweep - EPSILON);
        } else {
            printf("CIRCLE (%f, %f)\n", etmp->sweep_event.x, etmp->sweep_event.y);
            process_circle_event(tree, pq, etmp, sweep + EPSILON);
        }
        //bst_print(tree, *boundary_print);
    }

}
