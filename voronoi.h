/**
 * @file voronoi.h
 * @author Diram Tabaa (dtabaa@andrew.cmu.edu)
 * @brief 
 * @version 0.1
 * @date 2024-02-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _VORONOI_H_
#define _VORONOI_H_
#include <stdio.h>
#include <stdlib.h>
#include "geometry.h"
#include "priority_queue.h"
#include "bst.h"

#define INTERSECT 0
#define SINGLETON 1
#define SITE_EVENT 0
#define CIRCLE_EVENT 1
#define LEFT_SIDE 0
#define RIGHT_SIDE 1
#define EPSILON 1e-9

#define SYMMETRIC_LEQ(a, b) (((a) > (b))*(-2) + 1) // -1 if a > b, 1 if a <= b
#define DOUBLE2VOID(doublevar) (*((void**) &doublevar)); //can only be used with named variables


struct boundary {
    char label;
    point_t left_point;
    point_t right_point;
};

struct event {
    char label;
    int tag;
    point_t sweep_event;
    struct {
        point_t* left;
        point_t* mid;
        point_t* right;
    } triplet;
};

typedef struct boundary boundary_t;
typedef struct event event_t;

void event_print(void* e);

void boundary_print(void* elem);

event_t* new_event(char label, double x, double y, point_t* left, 
                point_t* mid, point_t* right);

int event_compare(void* e1, void* e2);

bst_t* compute_voronoi(pqueue_t* points);

#endif