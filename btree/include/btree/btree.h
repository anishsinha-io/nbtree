

#ifndef NBTREE_BTREE_H
#define NBTREE_BTREE_H

#include <stdint.h>

typedef struct BTree BTree;
typedef struct Node Node;

BTree *btree(uint32_t);

Node *root(BTree *);

void insert(BTree *, void *, int(*)(const void *, const void *));
void preorder(Node *, void(*)(const void *));

void test(Node *, void(*)(const void *));

#endif //NBTREE_BTREE_H
