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
#include "uarray.h"


int global_tag = 0;
/***************/
/* BOUNDARY    */
/***************/

/**
 * @brief This function initializes the boundary struct
 * 
 * @param bound 
 * @param left_x 
 * @param left_y 
 * @param right_x 
 * @param right_y 
 * @param label 
 */
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
    //printf("WELL IT IS %p\n", bound);
    init_boundary(bound, left_x, left_y, right_x, right_y, label);
    return bound;
}

void boundary_free(void* bound) {
    free(bound);
}

void boundary_print(void* elem) {
    boundary_t* bound = (boundary_t*) elem;
    printf("(%.2f, %.2f) -- (%.2f, %.2f) %p\n", bound->left_point.x,
            bound->left_point.y, bound->right_point.x, 
            bound->right_point.y, elem);
}

/***********/
/*  EVENTS */
/***********/

void event_print(void* e) {
    event_t* event = (event_t*) e;
    printf("(%.8f, %.8f)\n ", event->sweep_event.x, event->sweep_event.y);
    if (event->label == CIRCLE_EVENT) {
        point_print(event->triplet.left, "L");
        point_print(event->triplet.mid, "M");
        point_print(event->triplet.right, "R");
    }
}

void init_event(event_t* e, char label, double x, double y, point_t* left, 
                point_t* mid, point_t* right) {
    e->label = label;
    e->sweep_event.x = x;
    e->sweep_event.y = y;
    e->triplet.left = NULL;
    e->triplet.mid = NULL;
    e->triplet.right = NULL;
    if (left && (e->triplet.left = malloc(sizeof(point_t)))) {
        point_copy(left, e->triplet.left);
    } 
    if (mid && (e->triplet.mid = malloc(sizeof(point_t)))) {
        point_copy(mid, e->triplet.mid);
    } 
    if (right && (e->triplet.right = malloc(sizeof(point_t)))) {
        point_copy(right, e->triplet.right);
    } 
    e->tag = global_tag++;
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
    if (event1->sweep_event.y == event2->sweep_event.y) {
        if (event1->sweep_event.x > event2->sweep_event.x) return 1;
        if (event1->sweep_event.x == event2->sweep_event.x)  {
            if (event1->tag < event2->tag) return 1;
            return -1;
        }
    }
    return -1;
}

void event_free(event_t* event) {
    if (event->triplet.left) free(event->triplet.left);
    if (event->triplet.mid) free(event->triplet.mid);
    if (event->triplet.right) free(event->triplet.right);
    free(event);
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

    if (compute_circle_tangent(&neighbour->left_point, &neighbour->right_point,
                           site, &point)) return;

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

    if (intersect2.x == intersect1.x) return 0;
    return SYMMETRIC_LEQ(intersect1.x, intersect2.x);
}

double beachline_diff(void* elem1, void* elem2, void* args) {
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

    if (intersect2.x > intersect1.x) return intersect2.x - intersect1.x;
    return intersect1.x - intersect2.x;
}

int process_intersection_site(bst_t* beachline, pqueue_t* events, bst_t* voronoi,
                              point_t* site, double sweep) {
    boundary_t temp, temp_left, temp_right;
    boundary_t *left, *right, *new_left, *new_right, *new_bound, *dummy;
    line_t source_line;
    circle_t voronoi_vertex;
    segment_t *seg, *new_edge;
    void* arg = DOUBLE2VOID(sweep);
    double lower_sweep = sweep - EPSILON;
    void* lower_arg = DOUBLE2VOID(lower_sweep);

    /* we directly delete the intersection between the two arcs */
    init_boundary(&temp, site->x - EPSILON, site->y,
                            site->x - EPSILON, site->y, SINGLETON);
    bst_interval(beachline, &temp, (void**) &left, (void**) &right, arg);
    bst_delete(beachline, right, (void**) &seg, arg);

    init_boundary(&temp_left,  site->x - EPSILON, sweep,
                        site->x  - EPSILON, sweep, SINGLETON);
    init_boundary(&temp_right, site->x + EPSILON, sweep,
                        site->x  + EPSILON, sweep, SINGLETON);

    /* we only care about the left boundary */
    bst_interval(beachline, &temp_left, (void**) &new_left,
                (void**) &dummy, arg);
    /* we only care about the right boundary */
    bst_interval(beachline, &temp_right, (void**) &dummy,
                (void**) &new_right, arg);

    if (new_left) {
        new_circle_event(events, new_left, site, LEFT_SIDE,
                            CIRCLE_EVENT, sweep);
    }
    if (new_right) {
        new_circle_event(events, new_right, site, RIGHT_SIDE,
                        CIRCLE_EVENT, sweep);
    }

    /* we compute the voronoi vertex that results from the new site and the
       two sites at the intersection */
    compute_circumcircle(&right->left_point, site,
                         &right->right_point, &voronoi_vertex);
    segment_transform(seg, &voronoi_vertex.center);
    bst_insert(voronoi, seg, NULL, NULL);

    /* we compute the new boundary for the site and the left point, and 
       add a new dangling edge for the left side */
    compute_bisector(&right->left_point, site, &source_line);
    new_edge = segment_new(&source_line, &right->left_point, site);
    segment_transform(new_edge, &voronoi_vertex.center);
    new_bound = new_boundary(right->left_point.x, right->left_point.y,
                             site->x, site->y, INTERSECT);
    bst_insert(beachline, new_bound, new_edge, lower_arg);

    /* we compute the new boundary for the site and the right point, and 
       add a new dangling edge for the right side */
    compute_bisector(&right->right_point, site, &source_line);
    new_edge = segment_new(&source_line, site, &right->right_point);
    segment_transform(new_edge, &voronoi_vertex.center);
    new_bound = new_boundary(site->x, site->y, right->right_point.x,
                             right->right_point.y,  INTERSECT);
    bst_insert(beachline, new_bound, new_edge, lower_arg);
    
    return 0;
}

int process_site(bst_t* beachline, pqueue_t* events,  bst_t* voronoi,
                     point_t* site, double sweep) {
    boundary_t temp;
    boundary_t *left, *right,  *new_bound;
    line_t source_line;
    point_t* arc_point;
    segment_t* new_edge;
    double lower_sweep = sweep - EPSILON;
    double right_diff, left_diff;
    void* arg = DOUBLE2VOID(sweep);
    void* lower_arg = DOUBLE2VOID(lower_sweep);

    init_boundary(&temp, site->x, site->y, site->x, site->y, SINGLETON);

    /* we first check if the site lies directly underneath an intersection of 
       two arcs, if so we process this in a similar manner to a circle event */
    if (!bst_find(beachline, (void*) &temp, NULL, arg)) {
        return process_intersection_site(beachline, events, voronoi, site, sweep);
    }

    /* if we are unable to find boundaries to our left or our right, it means
       that the beachline is empty  */
    if (bst_interval(beachline, &temp, (void**) &left, (void**) &right, arg)) {
        return -1;
    }


    /* INVARIANT: if left and right not NULL, left->right == right->left */
    /* the invariant maintains that the left intersection's right will always
       be the right intersection's left, and at least one of them will be 
       non-NULL */

    /* this conditional handles the case where the new site lies between 
       the intersection of the same two arcs, just in different orientation, 
       since the two arcs have the same two points, they can only yield one 
       new circle event, and the orientation of the points is decided based 
       on the proximity to one of the insersections */
    if (left && right && point_equality(&left->right_point, &right->left_point)
        && point_equality(&left->left_point, &right->right_point)) {
        arc_point = &left->right_point; /* or &right->left_point*/
        right_diff = beachline_diff(right, &temp, arg);
        left_diff = beachline_diff(left, &temp, arg);
    
        /* TODO: possibly replace with atan2 check of two bisectors? */
        if (left_diff < right_diff) {
            new_circle_event(events, left, site, LEFT_SIDE, SITE_EVENT, sweep);
        } else {
            new_circle_event(events, right, site, RIGHT_SIDE, SITE_EVENT, sweep);
        }
    } else {
        
        /* this part handles the case where the two intersections (if present) 
           are of different points, and hence we get possibly 
           two circle events */
        if (left != NULL) {
            arc_point = &left->right_point;
            new_circle_event(events, left, site, LEFT_SIDE, SITE_EVENT, sweep);
        } 
        
        if (right != NULL) {
            arc_point = &right->left_point; /* even if both not NULL, by invariant still equal */
            new_circle_event(events, right, site, RIGHT_SIDE, SITE_EVENT, sweep);
        }
    }

    /* since the new site is going to lie under a parabola, we need to figure 
       out the line that goes through the insercetion of the parent parabola 
       and the new site paraobla, this will become a voronoi edge */
    compute_bisector(arc_point, site, &source_line);
    new_edge = segment_new(&source_line, arc_point, site);

    /* if the parent parabola is on the same y level, then there will only 
       be one intersection */
    if (arc_point->y == site->y) {
        double min_x = arc_point->x < site->x ? arc_point->x : site->x;
        double max_x = arc_point->x < site->x ? site->x : arc_point->x;
        new_bound = new_boundary(min_x, arc_point->y, max_x, site->y, INTERSECT);
        bst_insert(beachline, new_bound, new_edge, arg);
    } else {
        /*otherwise, we would have two intersections as the sweepline goes 
          down, and hence we need to add both boundaries, note how we offset
          the sweepline slightly down so that the two intersections become 
          distinguishable */
        new_bound = new_boundary(arc_point->x, arc_point->y,
                                 site->x, site->y, INTERSECT);
        bst_insert(beachline, new_bound, new_edge, lower_arg);
        new_bound = new_boundary(site->x, site->y, 
                                arc_point->x, arc_point->y, INTERSECT);
        bst_insert(beachline, new_bound, new_edge, lower_arg);
    }

    return 0;
}

int process_circle_event(bst_t* beachline, pqueue_t* events, bst_t* voronoi, event_t* e, 
                         double sweep) {
    circle_t voronoi_vertex;
    boundary_t left, right, *dummy, *new_left, *new_right, *new_bound;
    point_t *leftp, *midp, *rightp;
    segment_t *leftseg, *rightseg, *edge;
    line_t source_line;
    void* arg = DOUBLE2VOID(sweep);

    leftp = e->triplet.left;
    rightp = e->triplet.right;
    midp = e->triplet.mid;

    /* if the circle event is stale, i.e. another site came before in between,
        then just ignore */
    if (is_circle_event_stale(e, beachline, sweep)) return -1;

    /* computes the vertex that is to be added to the voronoi diagram, if 
       this circle event happens to be impossible (the bisectors diverge or 
       are parellel) we do not proceed */
    if (compute_circumcircle(leftp, midp, rightp, &voronoi_vertex)) return -1;

    /* we need to remove the two pairs containing the middle point since
      the arc between has now dissolved */
    init_boundary(&left, leftp->x, leftp->y, midp->x, midp->y, INTERSECT);
    init_boundary(&right, midp->x, midp->y, rightp->x, rightp->y, INTERSECT);

    /* if the two pairs are not on the beachline, it means that some other 
       circle event beat us to the dissolution, meaning this event is stale*/
    if (bst_find(beachline, (void*) &left, (void**) &leftseg, arg) ||
        bst_find(beachline, (void*) &right, (void**) &rightseg, arg)) return -1;

    /* this part should not exist in theory, however it handles a bug 
       that sometimes appear with the second delete */
    /* TODO: fix second delete bug */
    if (bst_delete(beachline, (void*) &left, (void**) &leftseg, arg)) return -1;
    if (bst_delete(beachline, (void*) &right, (void**) &rightseg, arg)) return -1;

    /* transforms what previously was a line into a ray, or what was prevously 
       was a ray into a segment, since now we hit a new voronoi vertex */
    segment_transform(leftseg, &voronoi_vertex.center);
    segment_transform(rightseg, &voronoi_vertex.center);

    /* if the segment is a line segment and is not already in the voronoi set,
       add it */
    if (leftseg->label == SEG_SEG && bst_find(voronoi, leftseg, NULL, NULL)) 
            bst_insert(voronoi, leftseg, NULL, NULL);
    if (rightseg->label == SEG_SEG && bst_find(voronoi, rightseg, NULL, NULL)) 
            bst_insert(voronoi, rightseg, NULL, NULL);
  


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

    /* inserting the new pair (arc intersection) after the middle point is 
      removed, there is only one such pair, we also add a new dangling edge 
      for this new boundary formed from the left and the right point of the 
      circle event */
    new_bound = new_boundary(leftp->x, leftp->y, rightp->x, rightp->y,
                             INTERSECT);
    compute_bisector(leftp, rightp, &source_line);
    edge = segment_new(&source_line, leftp, rightp);
    segment_transform(edge, &voronoi_vertex.center);
    bst_insert(beachline, new_bound, edge, arg);


    /*this conditional handles a very specific edge case where the two 
      boundaries that neighbour the boundaries of the circle event happen
      to be antisymmetric and involve the original points, in which case this
      would not form a new circle event*/
    if (new_left && new_right && (point_equality(&new_left->left_point,
         &new_right->right_point)) && (point_equality(&new_right->left_point,
        rightp)) && (point_equality(&new_left->right_point, leftp))) return 0;
    

    /* if the neighbouring left actually exists and that is not the 
       midpoint itself, then add a new circle event */
    if (new_left && !point_equality(&new_left->left_point, midp)
        && !point_equality(&new_left->right_point, rightp)) {
        new_circle_event(events, new_left, rightp, LEFT_SIDE,
                         CIRCLE_EVENT, sweep);
    }

     /* if the neighbouring right actually exists and that is not the 
       midpoint itself, then add a new circle event */
    if (new_right && !point_equality(&new_right->right_point, midp)
        && !point_equality(&new_right->left_point, leftp)) {
        new_circle_event(events, new_right, leftp, RIGHT_SIDE,
                         CIRCLE_EVENT, sweep);
    }

    return 0;
}

void preprocess_beachline(pqueue_t* points, bst_t* beachline) {
    double x1, y1, x2, y2, sweep;
    line_t source_line;
    segment_t *edge;
    event_t* event;
    void* arg;

    pqueue_pop(points, (void**) &event);
    x1 = event->sweep_event.x;
    y1 = event->sweep_event.y;
    event_free(event);

    pqueue_pop(points, (void**) &event);
    x2 = event->sweep_event.x;
    y2 = event->sweep_event.y; 
    event_free(event);

    sweep = y2 - EPSILON;   
    arg = DOUBLE2VOID(sweep);

    point_t p1 = {x1, y1};
    point_t p2 = {x2, y2};

    compute_bisector(&p1, &p2, &source_line);
    edge = segment_new(&source_line, &p1, &p2);
     if (y1 == y2) {
        bst_insert(beachline, new_boundary(x2, y2, x1, y1, INTERSECT),
                   edge, arg);
     } else {
        bst_insert(beachline, new_boundary(x1, y1, x2, y2, INTERSECT), 
                   edge, arg);
        bst_insert(beachline, new_boundary(x2, y2, x1, y1, INTERSECT),
                   edge, arg);
    }
}

void postprocess_beachline(bst_t* beachline, bst_t* voronoi, double sweep) {
    boundary_t* bound;
    segment_t* segment;
    void* arg = DOUBLE2VOID(sweep);

    while(!bst_rootkey(beachline, (void**) &bound)) {
        bst_delete(beachline, bound, (void**) &segment, arg);
        bst_insert(voronoi, segment, NULL, NULL);
    }

}

bst_t* compute_voronoi(pqueue_t* points) {
    event_t* event;
    bst_t *beachline = bst_new(*beachline_compare, *boundary_free);
    bst_t *voronoi = bst_new(*segment_compare, *segment_free);
    double sweep;

    preprocess_beachline(points, beachline);

     while (pqueue_size(points) > 0) {

        pqueue_pop(points, (void**) &event);

        sweep = event->sweep_event.y;

        if (event->label == SITE_EVENT) {
            process_site(beachline, points, voronoi, &event->sweep_event, sweep);
        } else {
            process_circle_event(beachline, points, voronoi, event, sweep + EPSILON);
        }
        event_free(event);
    }

    postprocess_beachline(beachline, voronoi, sweep);
    free(beachline);

    return voronoi; 
}

