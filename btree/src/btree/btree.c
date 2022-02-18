#include <cslice.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct Node {
    bool leaf;
    uint32_t order;
    Slice *data, *children;
} Node;

typedef struct BTree {
    Node *root;
    uint32_t order;
} BTree;

typedef struct NodePair {
    Node *first, *second;
    void *promoted_key;
} NodePair;

static Node *node(uint32_t min_order) {
    Node *n = malloc(sizeof(Node));
    n->children = slice(2 * min_order);
    n->data = slice(2 * min_order - 1);
    n->leaf = true;
    n->order = min_order;
    return n;
}

static Node *make_node(Slice *data, Slice *children, uint32_t min_order) {
    Node *n = malloc(sizeof(Node));
    n->children = children;
    n->data = data;
    n->order = min_order;
    n->leaf = true;
    return n;
}

static NodePair *node_pair(Node *first, Node *second, void *promoted_key) {
    NodePair *np = malloc(sizeof(NodePair));
    np->first = first;
    np->second = second;
    np->promoted_key = promoted_key;
    return np;
}

BTree *btree(uint32_t min_order) {
    BTree *t = malloc(sizeof(BTree));
    t->order = min_order;
    t->root = node(t->order);
    return t;
}

Node *root(BTree *tree) {
    return tree->root;
}

static bool full(Node *n) {
    return len(n->data) == 2 * n->order - 1;
}

void test_print(const void *key) {
    printf("%f\t", *(double *) key);
}

static NodePair *split(Node *n) {

    // nodes are only split if they are full, and the only possible full nodes are leaf nodes or the root. in the case
    // that the root is a leaf node, we need to split its children (evenly), since all of its children are valid node
    // pointers. on the other hand, if the node we are splitting is a leaf node, we don't need to worry about its
    // children because they are all null pointers. it is not possible for a node to have non-consecutive or uneven
    // children because that would violate the B-Tree requirement that all leaf nodes are on the same level. because
    // of this, we can safely assume that no leaf will have null siblings -- and therefore, all children slices will
    // be homogenous (made up of null pointers entirely or node pointers entirely).

    Slice *left_data = sslice(n->data, 0, len(n->data) / 2 + 1);
    Slice *right_data = sslice(n->data, len(n->data) / 2 + 1, len(n->data));

    // create nodes using the data slices we just created
    Node *left = make_node(left_data, slice(n->order), n->order);
    Node *right = make_node(right_data, slice(n->order), n->order);

    if (!n->leaf) {
        Slice *left_children = sslice(n->children, 0, len(n->children) / 2);
        Slice *right_children = sslice(n->children, len(n->children) / 2, len(n->children));
        left->children = left_children;
        right->children = right_children;
        left->leaf = false;
        right->leaf = false;
    }

    // pop the last key off the left slice in O(1) time
    void *promoted_key = pop(left_data);

    // create and return a new node pair
    NodePair *np = node_pair(left, right, promoted_key);
    return np;
}

static void insert_non_full(Node *n, void *key, int(*cmpfunc)(const void *, const void *)) {
    KeyIndex *kx = find_index(n->data, key, cmpfunc);
    if (n->leaf) {
        put_index(n->data, key, kx_index(kx));
    } else {
        Node *target = (Node *) get_index(n->children, kx_index(kx));
        if (full(target)) {
            NodePair *np = split(target);
            n->leaf = false;
            put_index(n->data, np->promoted_key, kx_index(kx));
            set_index(n->children, np->first, kx_index(kx));
            put_index(n->children, np->second, kx_index(kx) + 1);
            insert_non_full(n, key, cmpfunc);
            return;
        }
        insert_non_full(target, key, cmpfunc);
    }
}


void insert(BTree *tree, void *key, int(*cmpfunc)(const void *, const void *)) {
    Node *r = tree->root;
    if (full(r)) {
        Node *new_root = node(r->order);
        tree->root = new_root;
        new_root->leaf = false;
        NodePair *np = split(r);
        push(new_root->data, np->promoted_key);
        push(new_root->children, np->first);
        push(new_root->children, np->second);
        insert_non_full(new_root, key, cmpfunc);
        return;
    }
    insert_non_full(r, key, cmpfunc);
}

void *delete(BTree *tree, void *key, int(*cmpfunc)(const void *, const void *)) {

}

void preorder(Node *root, void(*printfunc)(const void *)) {
    if (root) {
        print(root->data, printfunc);
        for (int i = 0; i < len(root->children); i++) {
            preorder((Node *) get_index(root->children, i), printfunc);
        }
    }
}

void test(Node *root, void(*printfunc)(const void *)) {
    print(root->data, printfunc);
    print(((Node *) get_index(root->children, 0))->data, printfunc);
    print(((Node *) get_index(root->children, 1))->data, printfunc);
}