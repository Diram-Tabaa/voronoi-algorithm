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

void point_print(point_t* p, char* arg) {
    printf("%s: (%.2f, %.2f)\n", arg, p->x, p->y);
}
int point_equality(point_t* p1, point_t* p2) {
    return p1->x == p2->x && p1->y == p2->y;
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
    

    del_yl = left->y - sweep;
    del_yr = right->y - sweep;

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

    compute_bisector(p1, p2, &l1);
    compute_bisector(p2, p3, &l2);
    if ((retval = solve_linear(&l1, &l2, &center))) return retval;
    res->center.x = center.x;
    res->center.y = center.y;
    res->radius = computer_euclidean(p1, &center);
    return 0;
}

void compute_circle_tangent(point_t *p1, point_t *p2, point_t *p3, point_t* res) {
    circle_t circle;
    compute_circumcircle(p1, p2, p3, &circle);
    res->x = circle.center.x;
    res->y = circle.center.y - circle.radius;
    return;
}

int main_2(int argc, char** argv) {
    point_t p1 = {1.3, 5.1};
    point_t p2 = {-3.1, 7.1};
    point_t p3 = {-5, -5};
    circle_t result;
    compute_circumcircle(&p1, &p2, &p3, &result);

    point_t p4 = {-1, -0.1231111};
    point_t p5 = {5, 4};
    point_t p6;
    compute_arc_intersection(&p4, &p5, -1, &p6);
    printf("%f %f\n", p6.x, p6.y);
}
