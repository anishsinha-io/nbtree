/*
** February 23, 2022
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file implements an interface for polymorphic B-Trees of
** arbitrary order.
*/

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

void *loc_key(Loc *);
void *delete(BTree *, void *, int(*)(const void *, const void *));

#endif //NBTREE_BTREE_H
