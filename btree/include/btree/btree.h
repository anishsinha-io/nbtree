#ifndef NBTREE_BTREE_H
#define NBTREE_BTREE_H

#include <stdint.h>
#include <cslice.h>

typedef struct Node Node;
typedef struct BTree BTree;
typedef struct Loc Loc;

uint32_t loc_index(Loc *);

BTree *btree(uint32_t);
BTree *make_btree(void *, uint32_t, uint32_t, size_t, int(*)(const void *, const void *));

Slice *data(Node *);

Node *root(BTree *);
Node *loc_node(Loc *);

Loc *search(Node *, void *, int(*)(const void *, const void *));
Loc *inorder_predecessor(Node *, void *, int(*)(const void *, const void *));
Loc *inorder_successor(Node *, void *, int(*)(const void *, const void *));

void insert(BTree *, void *, int(*)(const void *, const void *));
void preorder(Node *, void(*)(const void *));
void transfer_key_right(Node *, uint32_t);
void transfer_key_left(Node *, uint32_t);

void *loc_key(Loc *);
void *delete(BTree *, void *, int(*)(const void *, const void *));

void test(Node *, void *, int(*)(const void *, const void *));

#endif //NBTREE_BTREE_H
