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
    void* val;
};

struct bst {
    node_t* root;
    int (*compare_fn)(void*, void*, void*);
};  

node_t *node_new(void* elem) {
    node_t* node;
    if (!(node = malloc(sizeof(node_t)))) {
        return NULL;
    }
    node->val = elem;
    node->right = NULL;
    node->left = NULL;
    return node;
}

bst_t *bst_new(int (*compare_fn)(void*, void*, void*)) {
    bst_t* tree;
    if (!(tree = malloc(sizeof(bst_t)))) {
        return NULL;
    }
    tree->root = NULL;
    tree->compare_fn = compare_fn;
    return tree;
}

int node_find(bst_t* tree, node_t* root, void* elem, void** resp,
              node_t** nodep, void* arg) {
    int res;
    int comp;

    if (root == NULL) {
        return -1;
    }

    /* if the element is smaller than root, traverse left */
    if ((comp = tree->compare_fn(elem, root->val, arg)) == SMALLER) {
        res = node_find(tree, root->left, elem, resp, nodep, arg);
    } else if (comp == GREATER) {
        res = node_find(tree, root->right, elem, resp, nodep, arg);
    } else {
        *resp = root->val;
        if (nodep) *nodep = root;
        return 1;
    }

    if (res == NULL_ROOT) {
        *resp = root->val;
        if (nodep) *nodep = root;
    }

    return 0;
}

int node_find_parent(bst_t* tree, node_t* root, void* elem, void** resp,
              node_t** parentp, void* arg) {
    int res;
    int comp;

    if (root == NULL) {
        return -1;
    }

    /* if the element is smaller than root, traverse left */
    //printf("so %p %p\n", elem, root->val);
    if ((comp = tree->compare_fn(elem, root->val, arg)) == EQUAL) {
        return 1;
    } else if (comp == SMALLER) {
        res = node_find_parent(tree, root->left, elem, resp, parentp, arg);
    } else {
        res = node_find_parent(tree, root->right, elem, resp, parentp, arg);
    }

    if (res == 1) {
        *resp = root->val;
        if (parentp) *parentp = root;
    }

    return 0;
}

int bst_find(bst_t* tree, void* elem, void** resp, node_t** nodep, void* arg) {
    return node_find(tree, tree->root,elem, resp, nodep, arg);
}

int node_interval(bst_t* tree, node_t* root, void* elem,
                  void** leftp, void** rightp, void* arg) {
    int comp;
    if (root == NULL) return -1;

    if ((comp = tree->compare_fn(elem, root->val, arg)) == SMALLER) {
        *rightp = root->val;
        node_interval(tree, root->left, elem, leftp, rightp, arg);

    } else if (comp == GREATER) {
        *leftp = root->val;
        node_interval(tree, root->right, elem, leftp, rightp, arg);

    }
    return 0;
}

int bst_interval(bst_t* tree, void* elem, void** leftp, void** rightp,
                 void* arg) {
    *leftp = NULL;
    *rightp = NULL;
    return node_interval(tree, tree->root, elem, leftp, rightp, arg);
}

int node_insert(bst_t* tree, node_t* root, void* elem, void* arg) {
    if (root == NULL) return -1;
    int comp, ins;
    if ((comp = tree->compare_fn(elem, root->val, arg)) == SMALLER) {
        if ((ins = node_insert(tree, root->left, elem, arg))) {
            root->left = node_new(elem);
        }
        
    } else if (comp == GREATER) {
        if ((ins = node_insert(tree, root->right, elem, arg))) {
            root->right = node_new(elem);
        } 
    }
    return 0;
}

int bst_insert(bst_t* tree, void* elem, void* arg) {
    if (tree == NULL) return -1;
    if (tree->root == NULL) {
        tree->root = node_new(elem);
        return 0;
    }
    return node_insert(tree, tree->root, elem, arg);
}

node_t* node_delete(node_t* root) {
    if (root == NULL) return NULL;
    if (root->left == NULL) return root->right;
   // printf("NODE %p\n", root->val);
    //printf("NODE LE%p\n", root->left->val);
    root->val = root->left->val;
    root->left = node_delete(root->left);
    return root;
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

int bst_delete(bst_t* tree, void* elem, void* arg) {
    void* res; 
    node_t* parent;
    int val;

    /* if the root itself is the element, merge directly */
    if (tree->compare_fn(tree->root->val, elem, arg) == EQUAL) {
        tree->root = node_union(tree->root->left, tree->root->right);
        return 0;
    }

    if (node_find_parent(tree, tree->root, elem, &res, &parent, arg) == -1) {
        return -1;
    } 
    //printf("PARENT %p\n", parent->val);
    if (tree->compare_fn(parent->val, elem, arg) == SMALLER) {
        parent->right = node_union(parent->right->left, parent->right->right);
    } else parent->left = node_union(parent->left->left, parent->left->right);
    return 0;
}


void node_print(node_t* root, void (*print_fn)(void*)) {
    if (root == NULL) return;
    node_print(root->left, print_fn);
    print_fn(root->val);
    node_print(root->right, print_fn);
}

void bst_print(bst_t* tree,void (*print_fn)(void*)) {
    node_print(tree->root, print_fn);
}
