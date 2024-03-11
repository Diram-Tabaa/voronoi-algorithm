/**
 * @file uarray.h
 * @author Diram Tabaa (dtabaa@andrew.cmu.edu)
 * @brief 
 * @version 0.1
 * @date 2024-03-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */

 #ifndef _UARRAY_H_
 #define _UARRAY_H_

struct uarray {
    unsigned int len;
    unsigned int capacity;
    void** array;
};

typedef struct uarray uarray_t;

uarray_t* uarray_new();

int uarray_append(uarray_t* arr, void* elem);

unsigned int uarray_len(uarray_t* arr);

void* uarray_get(uarray_t* arr, int idx);

#endif