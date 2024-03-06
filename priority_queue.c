/**
 * @file priority_queue.c
 * @author Diram Tabaa (dtabaa@andrew.cmu.edu)
 * @brief 
 * @version 0.1
 * @date 2024-02-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "priority_queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct pqueue {
    int size;
    int heap_size;
    void** heap;
    int (*compare_fn)(void*, void*);
};

void traverse_up(void** heap, int size, int (*compare_fn)(void*, void*)) {
    int idx, parent_idx;
    void* temp;
    idx = size - 1;
    parent_idx = (idx - 1) / 2;

    while (idx > 0 && compare_fn(heap[idx], heap[parent_idx]) == 1) {

        /* swap the elements around */
        temp = heap[idx];
        heap[idx] = heap[parent_idx];
        heap[parent_idx] = temp;

        /* update the indices */
        idx = parent_idx;
        parent_idx = (idx - 1) / 2;
    }
}

/**
 * @brief allocate and initialize a generic priority queue
 * 
 * @param compare_fn function used to compare the priority of two items
 * @return pqueue_t* pointer to the pqueue
 */
pqueue_t *pqueue_new(int (*compare_fn)(void*, void*)) {
    pqueue_t *que;
    if (compare_fn == NULL) return NULL;
    if (!(que = malloc(sizeof(pqueue_t)))) return NULL;
    if (!(que->heap = malloc(sizeof(void*)))) {
        free(que);
        return NULL;
    } 
    que->size = 0;
    que->heap_size = 1;
    que->compare_fn = compare_fn;
    return que;
}


void pqueue_print(pqueue_t* que) {
    assert(que != NULL);
    for (int i = 0; i < que->size; i++) {
        printf("%p ", que->heap[i]);
    }
    printf("\n");
}
/**
 * @brief return the size of the pqueue
 * 
 * @param que 
 * @return int 
 */
int pqueue_size(pqueue_t* que) {
    assert(que != NULL);
    return que->size;
}

/**
 * @brief insert an item into the priority queue, shifting up elements 
 *        if needed
 * 
 * @param que priority queue to insert into 
 * @param item item
 * @return int 0 if insertion is successful, -1 otherwise
 */
int pqueue_insert(pqueue_t *que, void* item) {
    assert(que != NULL);
    void** heap;
    /* if the queue is at capacity, we need to expand the array */
    if (que->heap_size == que->size) {
        if (!(heap = realloc(que->heap, 2*sizeof(void*)*que->heap_size))) {
            return -1;
        }
        que->heap = heap;
        que->heap_size *= 2;

    }
    que->heap[que->size++] = item;
    traverse_up(que->heap, que->size, que->compare_fn);
    return 0;
}

/**
 * @brief get the top priority element from the priority queue without 
 *        removing it from the queue
 * 
 * @param que priority queue
 * @param itemp pointer to memory address to which item is to be written
 * @return int 0 if successful, -1 otherwise
 */
int pqueue_peek(pqueue_t *que, void** itemp) {
    assert(que != NULL && itemp != NULL);
    if (que->size == 0) return -1;
    *itemp = que->heap[0];
    return 0;
}

/**
 * @brief pop the top priority element from the pqueue
 * 
 * @param que priority queue 
 * @param itemp pointer to memory address to which item is to be written
 * @return int 0 if successful, -1 otherwise
 */
int pqueue_pop(pqueue_t *que, void** itemp) {
    assert(que != NULL && itemp != NULL);
    if (que->size == 0) {
        return -1;
    }
    int idx = 0;
    int first_child = 2*idx + 1, second_child = 2*idx + 2;
    *itemp = que->heap[0];

    while (first_child < que->size) {

        /* first child has higher priority */
        if (second_child == que->size || 
            que->compare_fn(que->heap[first_child],
                            que->heap[second_child]) == 1) {
            que->heap[idx] = que->heap[first_child];
            idx = first_child;
        } else {
            que->heap[idx] = que->heap[second_child];
            idx = second_child;
        }

        first_child = 2*idx + 1;
        second_child = 2*idx + 2;
     }
    
    memmove(&que->heap[idx], &que->heap[idx+1],
             sizeof(void*)*(que->size - idx - 1));
    que->size--;
    return 0;
}
