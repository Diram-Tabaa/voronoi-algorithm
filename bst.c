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

/**
 * @brief find a node with a given key in a binary search tree
 * 
 * @param tree tree in which search is performed
 * @param root current call root element (needed for recursion)
 * @param key key to be matched
 * @param valp pointer to memory address to which value is written
 * @param nodep optional, addresss to which node pointer is written
 * @param arg argument to be passed to compare function
 * @return int 0 if node found, -1 otherwise
 */
int node_find(bst_t* tree, node_t* root, void* key, void** valp,
              node_t** nodep, void* arg) {
    int res;
    int comp;

    if (root == NULL) {
        return -1;
    }
    /* if the element is smaller than root, traverse left */
    if ((comp = tree->compare_fn(key, root->key, arg)) == SMALLER) {
        res = node_find(tree, root->left, key, valp, nodep, arg);
    } else if (comp == GREATER) {
        res = node_find(tree, root->right, key, valp, nodep, arg);
    } else {
        *valp = root->val;
        if (nodep) *nodep = root;
        return 0;
    }
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
    }

    return 0;
}

int bst_find(bst_t* tree, void* key, void** valp, node_t** nodep, void* arg) {
    return node_find(tree, tree->root, key, valp, nodep, arg);
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

int bst_interval(bst_t* tree, void* key, void** leftp, void** rightp,
                 void* arg) {
    *leftp = NULL;
    *rightp = NULL;
    return node_interval(tree, tree->root, key, leftp, rightp, arg);
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
    void* res; 
    node_t *parent, *target;
    int val;

    /* if the root itself is the element, merge directly */
    if (tree->compare_fn(tree->root->key, key, arg) == EQUAL) {
        target = tree->root;
        if (valp) *valp = target->key;
        tree->root = node_union(target->left, target->right);
        return 0;
    }

    if (node_find_parent(tree, tree->root, key, &res, &parent, arg) == -1) {
        return -1;
    } 

    if (tree->compare_fn(parent->key, key, arg) == SMALLER) {
        target = parent->right;
        parent->right = node_union(target->left, target->right);
    } else {
        target = parent->left;
        parent->left = node_union(target->left, target->right);
    }

    if (valp) *valp = target->val;

    return 0;
}


void node_print(node_t* root, void (*print_fn)(void*)) {
    if (root == NULL) return;
    node_print(root->left, print_fn);
    print_fn(root->key);
    node_print(root->right, print_fn);
}

void bst_print(bst_t* tree,void (*print_fn)(void*)) {
    node_print(tree->root, print_fn);
}
