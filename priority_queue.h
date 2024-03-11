/**
 * @file priority_queue.h
 * @author Diram Tabaa (dtabaa@andrew.cmu.edu)
 * @brief 
 * @version 0.1
 * @date 2024-02-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _PQUEUE_H_
#define _PQUEUE_H_
#include <assert.h>

struct pqueue;

typedef struct pqueue pqueue_t;

pqueue_t *pqueue_new(int (*compare_fn)(void*, void*));

int pqueue_size(pqueue_t* que);

int pqueue_insert(pqueue_t *que, void* item);

int pqueue_peek(pqueue_t *que, void** itemp);

int pqueue_pop(pqueue_t *que, void** itemp);

void pqueue_print(pqueue_t* que, void (* print_fn)(void*));

#endif