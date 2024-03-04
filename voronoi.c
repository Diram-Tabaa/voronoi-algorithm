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

int is_circle_event_stale(event_t* e, bst_t* beachline, double sweep) {
    assert(e->label == CIRCLE_EVENT);

    boundary_t temp;
    boundary_t *left, *right;
    void* arg = DOUBLE2VOID(sweep);


    init_boundary(&temp, e->sweep_event.x, e->sweep_event.y, 
                  e->sweep_event.x, e->sweep_event.y, SINGLETON);
    printf("%f %f\n", e->triplet.mid->x, e->triplet.mid->y);
    bst_interval(beachline, &temp, (void**) &left, (void**) &right, arg);
    boundary_print(left);
    boundary_print(right);
    return (!point_equality(&left->right_point, e->triplet.mid) ||
            !point_equality(&right->left_point, e->triplet.mid));
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
    arc_x = left->right_point.x;
    arc_y = left->right_point.y;

    compute_circle_tangent(&left->left_point, &left->right_point,
                           site, &temp_point);
    
    event_left = new_event(CIRCLE_EVENT, temp_point.x, temp_point.y,
                           &left->left_point, &left->right_point, site);
    pqueue_insert(events, (void*) event_left);

    compute_circle_tangent(&right->left_point, &right->right_point,
                           site, &temp_point);

    event_right = new_event(CIRCLE_EVENT, temp_point.x, temp_point.y,
                           site, &right->left_point, &right->right_point);
    pqueue_insert(events, (void*) event_right);

    new_bound = new_boundary(arc_x, arc_y, site->x, site->y, INTERSECT);
    bst_insert(beachline, new_bound, arg);
    new_bound = new_boundary(site->x, site->y, arc_x, arc_y, INTERSECT);
    bst_insert(beachline, new_bound, arg);
    
    return 0;
}

int process_circle_event(bst_t* beachline, pqueue_t* events, event_t* e, 
                         double sweep) {
    circle_t voronoi_vertex;
    boundary_t left, right, *new_left, *new_right, *new_bound;
    point_t *leftp, *midp, *rightp;
    event_t *event_left, *event_right;
    point_t temp_point;
    void* arg = DOUBLE2VOID(sweep);

    /* if the circle event is stale, i.e. another site came before in between,
        then just ignore */
    if (is_circle_event_stale(e, beachline, sweep)) return -1;

    leftp = e->triplet.left;
    rightp = e->triplet.right;
    midp = e->triplet.mid;

    /* computes the vertex that is to be added to the voronoi diagram */
    compute_circumcircle(leftp, midp, rightp, &voronoi_vertex);

    init_boundary(&left, leftp->x, leftp->y, midp->x, midp->y, INTERSECT);
    init_boundary(&right, midp->x, midp->y, rightp->x, rightp->y, INTERSECT);
    bst_delete(beachline, (void*) &left, arg);
    bst_delete(beachline, (void*) &right, arg);
    
    new_bound = new_boundary(leftp->x, leftp->y, rightp->x, rightp->y,
                             INTERSECT);
    bst_insert(beachline, new_bound, arg);

    init_boundary(&left, leftp->x, sweep, leftp->x, sweep, SINGLETON);
    init_boundary(&right, rightp->x, sweep, rightp->x, sweep, SINGLETON);

    bst_interval(beachline, &left, (void**) &new_left, (void**) &new_right, arg);
    compute_circle_tangent(&new_left->left_point, &new_left->right_point,
                           rightp, &temp_point);
    
    event_left = new_event(CIRCLE_EVENT, temp_point.x, temp_point.y,
                           &new_left->left_point, &new_left->right_point,
                           rightp);
    pqueue_insert(events, (void*) event_left);  
    bst_interval(beachline, &right, (void**) &new_left,  (void**) &new_right, arg);
    compute_circle_tangent(leftp, &new_right->left_point,
                           &new_right->right_point, &temp_point);
    
    event_right = new_event(CIRCLE_EVENT, temp_point.x, temp_point.y,
                           leftp, &new_right->left_point,
                           &new_right->right_point);
    pqueue_insert(events, (void*) event_left);  

    printf("v %f %f\n", voronoi_vertex.center.x ,voronoi_vertex.center.y);
    return 0;
}

int main(int argc, char** argv) {

    bst_t *tree = bst_new(*beachline_compare);
    boundary_t *p, *p2, *p3, *p4;
    double sweep = -0.6;
    double* sweepp = &sweep;
    void* temp = *((void**) sweepp);
    point_t test = {12, -0.59999};
    pqueue_t* pq;
    event_t* etmp;
    pq = pqueue_new(*event_compare);

    double x[] = {-1, 11, 24.66, 2};
    double y[] = {3, 4.8, 15.07};
    
   // for(int i = 0; i < 2; i++) {
   //     p = new_boundary(x[i], y[i], x[i+1], y[i+1], INTERSECT);
   //     bst_insert(tree, (void*) p, (void*) temp);
   //}

    p = new_boundary(9.34, -0.3, 11, 4.8, INTERSECT);
    bst_insert(tree, (void*) p, (void*) temp);
    p = new_boundary(11, 4.8, 9.34, -0.3, INTERSECT);
    bst_insert(tree, (void*) p, (void*) temp);
   // bst_print(tree, *boundary_print);
    p = new_boundary(12, -0.59999, 12, -0.59999, SINGLETON);
    pqueue_insert(pq, new_event(SITE_EVENT, 12, -0.59999, NULL, NULL, NULL));
    while (pqueue_size(pq) > 0) {
        pqueue_pop(pq, (void**) &etmp);
        printf("EVENT %f", etmp->sweep_event.y);
        if (etmp->label == SITE_EVENT) {
            process_site(tree, pq, &etmp->sweep_event, etmp->sweep_event.y);

        } else {
            process_circle_event(tree, pq, etmp, etmp->sweep_event.y + 1e-8);
        }
    }


/*
    process_site(tree, pq, &test, -0.6);
    bst_print(tree, *boundary_print);
    pqueue_pop(pq, (void**) &etmp);
    printf("ETMP %f %f\n", etmp->sweep_event.x, etmp->sweep_event.y);
    process_circle_event(tree, pq, etmp, etmp->sweep_event.y + 1e-8);
    bst_print(tree, *boundary_print);
    return 0;
*/
}
