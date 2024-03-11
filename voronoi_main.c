/**
 * @file voronoi_main.c
 * @author Diram Tabaa (dtabaa@andrew.cmu.edu)
 * @brief 
 * @version 0.1
 * @date 2024-02-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <stdio.h>
#include "voronoi.h"



int parse_input(char* filename, pqueue_t* events) {
    FILE* input;
    int args;
    double x, y;
    event_t* event;

    input = fopen(filename, "r");
    if (fscanf(input, "%d\n", &args) < 1) return -1;
    for (int i = 0; i < args; i++) {
        if (fscanf(input, "%lf %lf\n", &x, &y) < 2) return -1;
        printf("[%f, %f], ", x, y);
        event = new_event(SITE_EVENT, x, y, NULL, NULL, NULL);
        pqueue_insert(events, event);
    }
    printf("\n");
    return 0;
}

int main(int argc, char** argv) {
    pqueue_t* pq = pqueue_new(*event_compare);
    parse_input(argv[1], pq);
    compute_voronoi(pq);
    return 0;
}