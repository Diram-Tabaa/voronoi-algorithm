/**
 * @file bst.c
 * @author Diram Tabaa (dtabaa@andrew.cmu.edu)
 * @brief 
 * @version 0.1
 * @date 2024-02-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "bst.h"
#include <stdlib.h>
#include <stdio.h>
#include "geometry.h"
#include "voronoi.h"

struct node {
    struct node* left;
    struct node* right;
    void* key;
    void* val;
};

struct bst {
    node_t* root;
    int (*compare_fn)(void*, void*, void*);
};  

/**
 * @brief allocates and initializes a new node structure
 * 
 * @param key key associated with the node
 * @param val value associated with  the node
 * @return node_t* 
 */
node_t *node_new(void* key, void* val) {
    node_t* node;
    if (!(node = malloc(sizeof(node_t)))) {
        return NULL;
    }
    node->key = key;
    node->val = val;
    node->right = NULL;
    node->left = NULL;
    return node;
}

/**
 * @brief allocates a new Binary Search Tree
 * 
 * @param compare_fn compare function used to compare between keys
 * @return bst_t* 
 */
bst_t *bst_new(int (*compare_fn)(void*, void*, void*)) {
    bst_t* tree;
    if (!(tree = malloc(sizeof(bst_t)))) {
        return NULL;
    }
    tree->root = NULL;
    tree->compare_fn = compare_fn;
    return tree;
}


int node_interval(bst_t* tree, node_t* root, void* key,
                  void** leftp, void** rightp, void* arg) {
    int comp;
    if (root == NULL) return -1;


    if ((comp = tree->compare_fn(key, root->key, arg)) == SMALLER) {
        *rightp = root->key;
        node_interval(tree, root->left, key, leftp, rightp, arg);

    } else if (comp == GREATER) {
        *leftp = root->key;
        node_interval(tree, root->right, key, leftp, rightp, arg);

    }

    return 0;
}

/**
 * @brief 
 * 
 * @param tree 
 * @param root 
 * @param elem 
 * @param resp 
 * @param parentp 
 * @param arg 
 * @return int 
 */
int node_find_parent(bst_t* tree, node_t* root, void* key, void** valp,
              node_t** parentp, void* arg) {
    int res;
    int comp;

    if (root == NULL) {
        return -1;
    }

    /* if element is equal at root, we return 1 */
    if ((comp = tree->compare_fn(key, root->key, arg)) == EQUAL) {
      //  printf("key? %p\n", root->key);
        return 1;
    } 
    
    /* if the element is smaller than root, traverse left */
    if (comp == SMALLER) {
        res = node_find_parent(tree, root->left, key, valp, parentp, arg);
    } else {
        res = node_find_parent(tree, root->right, key, valp, parentp, arg);
    }

    /* if res is 1, it means that the child node is the node we are looking 
       for, hence we are the parent */
    if (res == 1) {
        *valp = root->val;
        if (parentp) *parentp = root;
       // printf("parent ket %p\n", root->key);
    }

    return 0;
}

node_t* left_spine(node_t* root) {
    node_t* target = root;
    while (target->left != NULL) {
        target = target->left;
    }
    return target;
}

node_t* right_spine(node_t* root) {
    node_t* target = root;
    while(target->right != NULL) {
        target = target->right;
    }
    return target;
}

int bst_interval(bst_t* tree, void* key, void** leftp, void** rightp,
                 void* arg) {
    node_t* target = tree->root;
    int cmp;
    *leftp = NULL;
    *rightp = NULL;
    while (target && (cmp = tree->compare_fn(key, target->key, arg)) != EQUAL) {
        if (cmp == SMALLER) {
            *rightp = target->key;
            target = target->left;
        } else {
            *leftp = target->key;
            target = target->right;
        }
    }
    if (cmp == EQUAL) {
        if (target->left != NULL) *leftp = right_spine(target->left)->key;
        if (target->right != NULL) *rightp = left_spine(target->right)->key;
    } 

    return 0;
}

int node_insert(bst_t* tree, node_t* root, void* key, void* val, void* arg) {
    if (root == NULL) return -1;
    int comp;
    if ((comp = tree->compare_fn(key, root->key, arg)) == SMALLER) {
        if (node_insert(tree, root->left, key, val, arg)) {
            root->left = node_new(key, val);
        }
        
    } else if (comp == GREATER) {
        if (node_insert(tree, root->right, key, val, arg)) {
            root->right = node_new(key, val);
        } 
    }
    return 0;
}

int bst_insert(bst_t* tree, void* key, void* val, void* arg) {
    if (tree == NULL) return -1;
    if (tree->root == NULL) {
        tree->root = node_new(key, val);
        return 0;
    } 
    node_t* target = tree->root;
    int cmp;
    while((cmp = tree->compare_fn(key, target->key, arg)) != EQUAL) {
        if (cmp == SMALLER) {
            if (target->left == NULL) {
                target->left = node_new(key, val); 
                return 0;
            } 
            target = target->left;
        } else {
            if (target->right == NULL) {
                target->right = node_new(key, val);
                return 0;
            }
            target = target->right;
        }
    }
    return 0;


    return node_insert(tree, tree->root, key, val, arg);
}


node_t* node_union(node_t* l_tree, node_t* r_tree) {
    if (l_tree == NULL) return r_tree;
    if (r_tree == NULL) return l_tree;
    node_t* r_temp = r_tree->left;
    node_t* l_temp = l_tree->right;
    l_tree->right = node_union(l_temp, r_temp);
    r_tree->left = l_tree;
    return r_tree;
}


int bst_delete(bst_t* tree, void* key, void** valp, void* arg) {
    node_t *parent, *target;
    int cmp;
    if (tree->root == NULL) return -1;
    parent = tree->root;
    target = tree->root;

    while ((cmp = tree->compare_fn(target->key, key, arg)) != EQUAL) {
        parent = target;
        if (cmp == GREATER) {
            target = target->left;
        } else {
            target = target->right;
        }
    }

    if (parent == target) {
        if (valp) *valp = parent->val;
        tree->root = node_union(parent->left, parent->right);
        return 0;
    }

    if (tree->compare_fn(parent->key, key, arg) == SMALLER) {
        target = parent->right;
        if (valp) *valp = target->val;
        parent->right = node_union(target->left, target->right);
    } else {
        target = parent->left;
        if (valp) *valp = target->val;
        parent->left = node_union(target->left, target->right);
    }

}

void node_print(node_t* root, void (*print_fn)(void*)) {
    if (root == NULL) return;
    node_print(root->left, print_fn);
    segment_print(root->val);
    node_print(root->right, print_fn);
}

void bst_print(bst_t* tree,void (*print_fn)(void*)) {
    node_print(tree->root, print_fn);
}
