/**
 * @file bst.h
 * @author Diram Tabaa (dtabaa@andrew.cmu.edu)
 * @brief 
 * @version 0.1
 * @date 2024-02-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _BST_H_
#define _BST_H_

#define GREATER -1
#define SMALLER 1
#define EQUAL 0
#define NULL_ROOT -1
#define KEY_FOUND 1
#define KEY_NOT_FOUND 0

struct node;
struct bst;


typedef struct bst bst_t;
typedef struct node node_t;

bst_t *bst_new(int (*compare_fn)(void*, void*, void*));

int bst_find(bst_t* tree, void* key, void** valp, node_t** nodep, void* arg);

int bst_interval(bst_t* tree, void* key, void** leftp, void** rightp, void* arg);

int bst_insert(bst_t* tree, void* key, void* val, void* arg);

int bst_delete(bst_t* tree, void* key, void** valp, void* arg);

void bst_print(bst_t* tree,void (*print_fn)(void*));

#endif 