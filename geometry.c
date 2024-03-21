/**
 * @file geometry.c
 * @author Diram Tabaa (dtabaa@andrew.cmu.edu)
 * @brief 
 * @version 0.1
 * @date 2024-02-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "geometry.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void point_print(point_t* p, char* arg) {
    printf("%s: (%.5f, %.5f)\n", arg, p->x, p->y);
}

int point_equality(point_t* p1, point_t* p2) {
    return p1->x == p2->x && p1->y == p2->y;
}

int point_compare(point_t* p1, point_t* p2) {
    int comp;
    if ((comp = ASYM_COMPARE(p1->y, p2->y))) return comp;
    return ASYM_COMPARE(p1->x, p2->x);
}

void point_copy(point_t* src, point_t* dest) {
    dest->x = src->x;
    dest->y = src->y;
}

segment_t* segment_new(line_t* source_line, point_t* dp1, point_t* dp2) {
    segment_t* seg;
    if (!(seg = malloc(sizeof(segment_t)))) return NULL;
    seg->label = SEG_LINE;
    seg->options.line.intercept = source_line->intercept;
    seg->options.line.gradient = source_line->gradient;
    seg->dual.p1.x = dp1->x;
    seg->dual.p1.y = dp1->y;
    seg->dual.p2.x = dp2->x;
    seg->dual.p2.y = dp2->y;
    return seg;
}

void segment_transform(segment_t* seg, point_t* point) {
    if (seg->label == SEG_LINE) {
        segment_line2ray(seg, point);
    } else {
        segment_ray2seg(seg, point);
    }
}
void segment_line2ray(segment_t* seg, point_t* point) {
    assert(seg->label == SEG_LINE);
    seg->label= SEG_RAY;
    seg->options.ray.gradient = LINE_GRADIENT(seg);
    seg->options.ray.p.x = point->x;
    seg->options.ray.p.y = point->y;
}

void segment_ray2seg(segment_t* seg, point_t* point) {
    assert(seg->label == SEG_RAY);
    seg->label = SEG_SEG;
    seg->options.seg.p1.x = RAY_POINT(seg)->x;
    seg->options.seg.p1.y = RAY_POINT(seg)->y;
    seg->options.seg.p2.x = point->x;
    seg->options.seg.p2.y = point->y;
}

void segment_print(segment_t* seg) {
    switch (seg->label) {
        case SEG_LINE:
            printf("LINE ");
            printf("grad %f, intercept %f\n", seg->options.line.gradient,
                    seg->options.line.intercept);
            break;
        case SEG_RAY:
            printf("[[%f, %f], [%f, %f]], ", seg->dual.p1.x, 
                  seg->dual.p1.y, seg->dual.p2.x,
                   seg->dual.p2.y);
        case SEG_SEG:
            printf("[[%f, %f], [%f, %f]], ", seg->dual.p1.x, 
                  seg->dual.p1.y, seg->dual.p2.x,
              seg->dual.p2.y);
          break;
    }
}

void segment_free(segment_t* seg) {
    free(seg);
}


/**
 * @brief An arbitrary compare function to facilitate insertion into BST
 * 
 * @param s1 
 * @param s2 
 * @return int 
 */
int segment_compare(void* s1, void* s2, void* arg) {
    segment_t* seg1 = (segment_t*) s1;
    segment_t* seg2 = (segment_t*) s2;
    int comp;
  //  printf("COMP\n");
  //  segment_print(s1);
  //  segment_print(s2);
    if ((comp = ASYM_COMPARE(seg1->label, seg2->label))) {
       // printf(" %d\n", comp);
        return comp;}
    if (seg1->label == SEG_LINE) {
        if ((comp = ASYM_COMPARE(seg1->options.line.intercept,
                                 seg2->options.line.intercept))) return comp;
        return ASYM_COMPARE(seg1->options.line.gradient,
                            seg2->options.line.gradient);                         
    } else if (seg1->label == SEG_RAY) {
        if ((comp = point_compare(&seg1->options.ray.p,
                                  &seg2->options.ray.p))) return comp;
        return ASYM_COMPARE(seg1->options.ray.gradient,
                            seg2->options.ray.gradient);  
    } else {
        if ((comp = point_compare(&seg1->options.seg.p1,
                                  &seg2->options.seg.p1))) return comp;
        return point_compare(&seg1->options.seg.p2,
                            &seg2->options.seg.p2);
    }

}

double compute_parabola_value(point_t* focus, double sweep, double x) {
    double x_prime = x - focus->x;
    x_prime *= x_prime;
    x_prime += focus->y*focus->y;
    x_prime -= sweep*sweep;
    return x_prime / (2 * (focus->y - sweep));
}

int compute_arc_intersection(point_t *left, point_t *right, double sweep,
                              point_t *res) {
    double del_yl, del_yr;
    double al, bl, cl;
    double a, b, c;
    double determinant;
    circle_t circle;
    point_t mid;

    if (left->y == right->y && left->y == sweep) {
        res->x = (left->x + right->x) / 2;
        res->y = 1e+8;
        return 0;
    } else if (left->y == sweep) {
        res->x = left->x;
        res->y = compute_parabola_value(right, sweep, left->x);
        return 0;
    } else if (right->y == sweep) {
        res->x = right->x;
        res->y = compute_parabola_value(right, sweep, right->x);
        return 0;
    }

    del_yl = left->y - sweep;
    del_yr = right->y - sweep;
    if (del_yl <= 0)  del_yl += 1e-6;
    if (del_yr <= 0) del_yr += 1e-6;

    if (del_yl == del_yr) {
        mid.y = sweep;
        mid.x = (right->x + left->x) / 2;
        compute_circumcircle(left, right, &mid, &circle);
        res->x = circle.center.x;
        res->y = circle.center.y;    
        return 0;
    }

    /* computing the coefficients of the quadratic equation */
    al = (0.5 / del_yl);
    a = al - (0.5 / del_yr);
    bl = - (left->x / del_yl);
    b = (right->x / del_yr) + bl;
    cl = (left->x*left->x + left->y*left->y - sweep*sweep) / (2*del_yl);
    c =  cl - (right->x*right->x + right->y*right->y - sweep*sweep) / (2*del_yr);
    if ((determinant = b*b - 4*a*c) < 0) return -1;
    res->x = (-b + sqrt(determinant)) / (2*a);
    res->y = al*(res->x*res->x) + bl*res->x + cl;
    return 0;
}

double compute_gradient(point_t *p1, point_t *p2) {
    double delta_x = p2->x - p1->x;
    double delta_y = p2->y - p1->y;
    if (delta_x == 0) return INFINITY;
    return (delta_y / delta_x);
}

double computer_euclidean(point_t *p1, point_t *p2) {
    double delta_x = (p1->x - p2->x);
    double delta_y = (p1->y - p2->y);
    return sqrt(delta_x*delta_x + delta_y*delta_y);
}
void compute_midpoint(point_t *p1, point_t *p2, point_t *res) {
    res->x = (p1->x + p2->x) / 2;
    res->y = (p1->y + p2->y) / 2;
}

void compute_bisector(point_t *p1, point_t *p2, line_t *res) {
    point_t midpoint;
    double grad = compute_gradient(p1, p2);
    compute_midpoint(p1, p2, &midpoint);
    if (grad == 0) {
        res->gradient = INFINITY;
        res->intercept = midpoint.x;
    } else if (grad == INFINITY) {
        res->gradient = 0;
        res->intercept = midpoint.y;
    } else {
        res->gradient = - 1 / grad;
        res->intercept = midpoint.y + (midpoint.x / grad);
    }
}

int solve_linear(line_t *l1, line_t *l2, point_t *res) {
    if (l1->gradient == l2->gradient) {
        if (l1->intercept == l2->intercept) return -1; /* infinite solutions */
        return -2; /* no solution */
    }
    if (l1->gradient == INFINITY) {
        res->x = l1->intercept;
        res->y = l2->gradient*res->x + l2->intercept;
    } else if (l2->gradient == INFINITY) {
        res->x = l2->intercept;
        res->y = l1->gradient*res->x + l1->intercept;
    } else {
        res->x = (l2->intercept - l1->intercept) / (l1->gradient - l2->gradient);
        res->y = l1->gradient*res->x + l1->intercept; /* either is fine */
    }
    return 0; /* one unique solution */
}

int compute_circumcircle(point_t *p1, point_t *p2, point_t *p3, circle_t *res) {
    line_t l1, l2;
    point_t center;
    int retval;


    point_t v1, v2;
    v1.x = p1->x - p2->x;
    v1.y = p1->y - p2->y;
    v2.x = p3->x - p2->x;
    v2.y = p3->y - p2->y;
    double ang = atan2(v2.y*v1.x - v2.x*v1.y, v1.x*v2.x + v1.y*v2.y);
    if (ang <= 0) return -1;

    compute_bisector(p1, p2, &l1);
    compute_bisector(p2, p3, &l2);
    if ((retval = solve_linear(&l1, &l2, &center))) return retval;
    res->center.x = center.x;
    res->center.y = center.y;
    res->radius = computer_euclidean(p1, &center);
    return 0;
}

int compute_circle_tangent(point_t *p1, point_t *p2, point_t *p3, point_t* res) {
    circle_t circle;
    int retval; 
    if ((retval = compute_circumcircle(p1, p2, p3, &circle))) return retval;
    res->x = circle.center.x;
    res->y = circle.center.y - circle.radius;
    return 0;
}

