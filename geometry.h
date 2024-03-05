/**
 * @file geometry.h
 * @author Diram Tabaa (dtabaa@andrew.cmu.edu)
 * @brief 
 * @version 0.1
 * @date 2024-02-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

struct point {
    double x;
    double y;
};

struct circle {
    struct point center;
    double radius;
};

struct line {
    double gradient;
    double intercept;
};

typedef struct point point_t;
typedef struct circle circle_t;
typedef struct line line_t;

void point_print(point_t* p, char* arg);

int point_equality(point_t* p1, point_t* p2);

double compute_parabola_value(point_t* focus, double sweep, double x);

int compute_arc_intersection(point_t *left, point_t *right, double sweep,
                              point_t *res);

void compute_midpoint(point_t *p1, point_t *p2, point_t *res);

int compute_circumcircle(point_t *p1, point_t *p2, point_t *p3, circle_t *res);

void compute_circle_tangent(point_t *p1, point_t *p2, point_t *p3, point_t* res);

#endif 