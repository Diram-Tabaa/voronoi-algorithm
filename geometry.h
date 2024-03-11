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

#define SEG_LINE 0
#define SEG_RAY 1
#define SEG_SEG 2

#define RAY_GRADIENT(sg) (sg->options.ray.gradient)
#define RAY_POINT(sg) (&sg->options.ray.p)
#define LINE_GRADIENT(sg) (sg->options.line.gradient)
#define LINE_INTERCEPT(sg) (sg->options.line.intercept)
#define SEG_POINT1(sg) (&sg->options.seg.p1)
#define SEG_POINT2(sg) (&sg->options.seg.p2)

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

struct segment {
    char label;
    union {
        struct line line;
        struct {
            struct point p;
            double gradient;
        } ray;
        struct {
            struct point p1;
            struct point p2;
        } seg;
    } options;
    struct {
        struct point p1;
        struct point p2;
    } dual;
};

typedef struct point point_t;
typedef struct circle circle_t;
typedef struct line line_t;
typedef struct segment segment_t;
void point_print(point_t* p, char* arg);

int point_equality(point_t* p1, point_t* p2);

void point_copy(point_t* src, point_t* dest);

segment_t* segment_new(line_t* source_line, point_t* dp1, point_t* dp2);

void segment_print(segment_t* seg);

void segment_transform(segment_t* seg, point_t* point);

void segment_line2ray(segment_t* seg, point_t* point);

void segment_ray2seg(segment_t* seg, point_t* point);

double compute_parabola_value(point_t* focus, double sweep, double x);

int compute_arc_intersection(point_t *left, point_t *right, double sweep,
                              point_t *res);

void compute_bisector(point_t *p1, point_t *p2, line_t *res);

void compute_midpoint(point_t *p1, point_t *p2, point_t *res);

int compute_circumcircle(point_t *p1, point_t *p2, point_t *p3, circle_t *res);

void compute_circle_tangent(point_t *p1, point_t *p2, point_t *p3, point_t* res);

#endif 