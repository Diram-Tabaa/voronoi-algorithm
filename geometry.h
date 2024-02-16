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

int compute_arc_intersection(point_t *left, point_t *right, double sweep,
                              point_t *res);

int compute_circumcircle(point_t *p1, point_t *p2, point_t *p3, circle_t *res);

#endif 