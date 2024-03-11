/**
 * @file uarray.c
 * @author Diram Tabaa (dtabaa@andrew.cmu.edu)
 * @brief 
 * @version 0.1
 * @date 2024-03-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */

 #include "uarray.h"
 #include <stdlib.h>


uarray_t* uarray_new() {
    uarray_t* arr;
    if (!(arr = malloc(sizeof(uarray_t)))) return NULL;
    if (!(arr->array = malloc(sizeof(void*)))) return NULL;
    arr->len = 0;
    arr->capacity = 1;
    return arr;
}

int uarray_append(uarray_t* arr, void* elem) {
    void** temp;
    arr->array[arr->len++] = elem;
    if (arr->len == arr->capacity) {
        if (!(temp = realloc(arr->array, 2*sizeof(void*)*arr->capacity))) {
            return -1;
        }
        arr->array = temp;
        arr->capacity *= 2;
    }
    return 0;
}

unsigned int uarray_len(uarray_t* arr) {
    return arr->len;
}

void* uarray_get(uarray_t* arr, int idx) {
    return arr->array[idx];
}