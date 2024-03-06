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


int event_compare(void* e1, void* e2) {
    event_t* event1 = (event_t*) e1;
    event_t* event2 = (event_t*) e2;
    if (event1->sweep_event.y > event2->sweep_event.y) return 1;
    return -1;
}

/**
 * @brief tests if a given circle event at a given sweep depth is valid or
 *        if it has otherwise been made redundant by an earlier event
 * 
 * @param e circle event to be tested 
 * @param beachline binary tree representing the beachline
 * @param sweep the y-value of the sweepline
 * @return int 0 if valid, 1 if not valid
 */
int is_circle_event_stale(event_t* e, bst_t* beachline, double sweep) {
    double beachline_x, beachline_y;
    point_t* focus;
    circle_t event_circle;
    boundary_t bound, *left = NULL, *right = NULL;
    void* arg = DOUBLE2VOID(sweep);

    beachline_x = e->sweep_event.x;

    /* we want to find under which arc does the circle center lie */
    init_boundary(&bound, beachline_x, sweep, beachline_x, sweep,
                  SINGLETON);
    bst_interval(beachline, &bound, (void**) &left,
                 (void**) &right, arg);

    /* the invariant maintains that the left intersection's right will always
       be the right intersection's left, and at least one of them will be 
       non-NULL */
    if (left) focus = &left->right_point;
    if (right) focus = &right->left_point;

    /* we compute the y value on the beachline at that point, so that we can
       tell if the circumcircle's center is behind or on the beachline, if it 
       is behind it means that this circle event is no longer valid */
    beachline_y = compute_parabola_value(focus, sweep, beachline_x);
    compute_circumcircle(e->triplet.left, e->triplet.mid, e->triplet.right,
                         &event_circle);


    return (event_circle.center.y > beachline_y);
}

void new_circle_event(pqueue_t* events, boundary_t* neighbour, point_t* site,
                      char side, char original_event, double sweep) {
    point_t point;
    event_t* event;

    compute_circle_tangent(&neighbour->left_point, &neighbour->right_point,
                           site, &point);

    if (original_event == CIRCLE_EVENT && point.y >= sweep) return;

    if (side == LEFT_SIDE) {
        event = new_event(CIRCLE_EVENT, point.x, point.y,
                          &neighbour->left_point, &neighbour->right_point,
                          site);
    } else {
        event = new_event(CIRCLE_EVENT, point.x, point.y, site,
                          &neighbour->left_point, &neighbour->right_point);
    }
    pqueue_insert(events, (void*) event);
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

    if (intersect1.x == intersect2.x) return 0;
    return SYMMETRIC_LEQ(intersect1.x, intersect2.x);
}



int process_site(bst_t* beachline, pqueue_t* events,
                     point_t* site, double sweep) {
    boundary_t temp;
    boundary_t *left, *right, *new_bound;

    double arc_x, arc_y;

    void* arg = DOUBLE2VOID(sweep);

    init_boundary(&temp, site->x, site->y, site->x, site->y, SINGLETON);
   
    /* if bst insert return -1, means first boundary */
    if (bst_interval(beachline, &temp, (void**) &left, (void**) &right, arg)) {
        return -1;
    }

    /* INVARIANT: if left and right not NULL, left->right == right->left */
    /* the invariant maintains that the left intersection's right will always
       be the right intersection's left, and at least one of them will be 
       non-NULL */
    if (left != NULL) {
        arc_x = left->right_point.x;
        arc_y = left->right_point.y;
        new_circle_event(events, left, site, LEFT_SIDE, SITE_EVENT, sweep);
    }
    
    if (right != NULL) {
        arc_x = right->left_point.x;
        arc_y = right->left_point.y;
        new_circle_event(events, right, site, RIGHT_SIDE, SITE_EVENT, sweep);
    }
    
    new_bound = new_boundary(arc_x, arc_y, site->x, site->y, INTERSECT);
    bst_insert(beachline, new_bound, arg);
    new_bound = new_boundary(site->x, site->y, arc_x, arc_y, INTERSECT);
    bst_insert(beachline, new_bound, arg);
    return 0;
}

int process_circle_event(bst_t* beachline, pqueue_t* events, event_t* e, 
                         double sweep) {
    circle_t voronoi_vertex;
    boundary_t left, right, *dummy, *new_left, *new_right, *new_bound;
    point_t *leftp, *midp, *rightp;
    void* arg = DOUBLE2VOID(sweep);

    /* if the circle event is stale, i.e. another site came before in between,
        then just ignore */
   if (is_circle_event_stale(e, beachline, sweep)) return -1;

    leftp = e->triplet.left;
    rightp = e->triplet.right;
    midp = e->triplet.mid;

    /* computes the vertex that is to be added to the voronoi diagram */
    compute_circumcircle(leftp, midp, rightp, &voronoi_vertex);

    /* we need to remove the two pairs containing the middle point since
      the arc between has now dissolved */
    init_boundary(&left, leftp->x, leftp->y, midp->x, midp->y, INTERSECT);
    init_boundary(&right, midp->x, midp->y, rightp->x, rightp->y, INTERSECT);
    bst_delete(beachline, (void*) &left, arg);
    bst_delete(beachline, (void*) &right, arg);

        printf("(%f, %f)\n ", voronoi_vertex.center.x, 
                        voronoi_vertex.center.y);
    printf("ENDS\n");
    boundary_print(&left);
    boundary_print(&right);
    
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


    printf("Starts for: ");
    boundary_print(new_bound);


    /* if the neighbouring left actually exists and that is not the 
       midpoint itself, then add a new circle event */
    if (new_left && !point_equality(&new_left->left_point, midp)) {
        new_circle_event(events, new_left, rightp, LEFT_SIDE,
                         CIRCLE_EVENT, sweep);
    }
     /* if the neighbouring right actually exists and that is not the 
       midpoint itself, then add a new circle event */
    if (new_right && !point_equality(&new_right->right_point, midp)) {
        new_circle_event(events, new_right, leftp, RIGHT_SIDE,
                         CIRCLE_EVENT, sweep);
    }


    return 0;
}

void preprocess_beachline(pqueue_t* points, bst_t* beachline) {
    double x1, y1, x2, y2, sweep;
    event_t* event;
    void* arg;
    pqueue_pop(points, (void**) &event);
    x1 = event->sweep_event.x;
    y1 = event->sweep_event.y;
    pqueue_pop(points, (void**) &event);
    x2 = event->sweep_event.x;
    y2 = event->sweep_event.y; 
    sweep = y2 - EPSILON;   
    arg = DOUBLE2VOID(sweep);
    bst_insert(beachline, new_boundary(x1, y1, x2, y2, INTERSECT), arg);
    bst_insert(beachline, new_boundary(x2, y2, x1, y1, INTERSECT), arg);
    printf("%f %f %f %f\n", x1, y1, x2, y2);
}

void compute_voronoi(pqueue_t* points) {
    event_t* event;
    bst_t *tree = bst_new(*beachline_compare);
    double sweep;
    preprocess_beachline(points, tree);
     while (pqueue_size(points) > 0) {
        pqueue_pop(points, (void**) &event);
        sweep = event->sweep_event.y;
        if (event->label == SITE_EVENT) {
          // printf("SITE (%f, %f)\n", etmp->sweep_event.x, etmp->sweep_event.y);
            process_site(tree, points, &event->sweep_event, sweep - EPSILON);
        } else {
           // printf("CIRCLE (%f, %f)\n", etmp->sweep_event.x, etmp->sweep_event.y);
            process_circle_event(tree, points, event, sweep + EPSILON);
        }
        bst_print(tree, *boundary_print);
    }
}

int main2(int argc, char** argv) {

    bst_t *tree = bst_new(*beachline_compare);
    boundary_t *p;
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
    pqueue_insert(pq, new_event(SITE_EVENT, 4, -2.3, NULL, NULL, NULL));
    pqueue_insert(pq, new_event(SITE_EVENT, 2, -3.97, NULL, NULL, NULL));
    pqueue_insert(pq, new_event(SITE_EVENT, 4.38, -4.03, NULL, NULL, NULL));
    pqueue_insert(pq, new_event(SITE_EVENT, 6.27, -4.58, NULL, NULL, NULL));
    pqueue_insert(pq, new_event(SITE_EVENT, 3.3, -5.83, NULL, NULL, NULL));
    while (pqueue_size(pq) > 0) {
        pqueue_pop(pq,(void**) &etmp);
        sweep = etmp->sweep_event.y;
        if (etmp->label == SITE_EVENT) {
          // printf("SITE (%f, %f)\n", etmp->sweep_event.x, etmp->sweep_event.y);
            process_site(tree, pq, &etmp->sweep_event, sweep - EPSILON);
        } else {
           // printf("CIRCLE (%f, %f)\n", etmp->sweep_event.x, etmp->sweep_event.y);
            process_circle_event(tree, pq, etmp, sweep + EPSILON);
        }
        //bst_print(tree, *boundary_print);
    }
    return 0;
}
