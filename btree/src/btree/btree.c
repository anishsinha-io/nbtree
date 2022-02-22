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

typedef struct Loc {
    Node *node;
    KeyIndex *kx;
} Loc;

uint32_t loc_index(Loc *l) {
    return kx_index(l->kx);
}

void *loc_key(Loc *l) {
    return kx_key(l->kx);
}

Node *loc_node(Loc *l) {
    return l->node;
}

typedef struct NodePair {
    Node *first, *second;
    void *promoted_key;
} NodePair;

static Loc *loc(Node *n, KeyIndex *kx) {
    Loc *l = malloc(sizeof(Loc));
    l->node = n;
    l->kx = kx;
    return l;
}

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
    printf("%d\t", *(int *) key);
}

Loc *search(Node *root, void *key, int(*cmpfunc)(const void *, const void *)) {
    KeyIndex *kx = find_index(root->data, key, cmpfunc);
    if (kx_key(kx)) return loc(root, kx);
    if (!kx_key(kx) && root->leaf) {
        printf("key not found!\n");
        return loc(root, key_index(NULL, kx_index(kx)));
    }
    if (!kx_key(kx)) return search((Node *) get_index(root->children, kx_index(kx)), key, cmpfunc);
    return loc(root, key_index(NULL, kx_index(kx)));
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

BTree *make_btree(void *keys, uint32_t num_keys, uint32_t min_order, size_t key_size,
                  int(*cmpfunc)(const void *, const void *)) {
    BTree *tree = btree(min_order);
    for (int i = 0; i < num_keys; i++) insert(tree, keys + i * key_size, cmpfunc);
    return tree;
}

Loc *predecessor_helper(Loc *l) {
    Node *adjacent_left_child = (Node *) get_index(l->node->children, loc_index(l));
    while (!adjacent_left_child->leaf) {
        adjacent_left_child = (Node *) last(adjacent_left_child->children);
    }
    return loc(adjacent_left_child,
               key_index((void *) last(adjacent_left_child->data), len(adjacent_left_child->data) - 1));
}

Loc *inorder_predecessor(Node *root, void *key, int(*cmpfunc)(const void *, const void *)) {
    Loc *l = search(root, key, cmpfunc);
    if (!l->kx) {
        printf("cannot find inorder predecessor of nonexistent node!\n");
        return loc(root, NULL);
    }
    if (l->node->leaf) return loc(root, l->kx);
    return predecessor_helper(loc(root, l->kx));
}

Loc *successor_helper(Loc *l) {
    Node *adjacent_right_child = (Node *) get_index(l->node->children, loc_index(l) + 1);
    while (!adjacent_right_child->leaf) {
        adjacent_right_child = (Node *) first(adjacent_right_child->children);

    }
    return loc(adjacent_right_child, key_index((void *) first(adjacent_right_child->data), 0));
}

Loc *inorder_successor(Node *root, void *key, int(*cmpfunc)(const void *, const void *)) {
    Loc *l = search(root, key, cmpfunc);
    if (!l->kx) {
        printf("cannot find inorder successor of nonexistent node!\n");
        return loc(root, NULL);
    }
    if (l->node->leaf) return loc(root, l->kx);
    return successor_helper(loc(root, l->kx));
}

void preorder(Node *root, void(*printfunc)(const void *)) {
    if (root) {
        print(root->data, printfunc);
        for (int i = 0; i < len(root->children); i++) {
            preorder((Node *) get_index(root->children, i), printfunc);
        }
    }
}

static void transfer_key_left(Node *root, uint32_t index) {
    Node *sender = (Node *) get_index(root->children, index + 1);
    Node *receiver = (Node *) get_index(root->children, index);
    void *key_to_promote = shift(sender->data);
    void *key_to_demote = (void *) get_index(root->data, index);
    push(receiver->data, key_to_demote);
    set_index(root->data, key_to_promote, index);
}

static void transfer_key_right(Node *root, uint32_t index) {
    Node *sender = (Node *) get_index(root->children, index - 1);
    Node *receiver = (Node *) get_index(root->children, index);
    void *key_to_promote = pop(sender->data);
    void *key_to_demote = (void *) get_index(root->data, index - 1);
    unshift(receiver->data, key_to_demote);
    set_index(root->data, key_to_promote, index - 1);
}

static void *single_pass_delete(Node *root, void *key, int(*cmpfunc)(const void *, const void *)) {
    printf("here\n");
    print(root->data, &test_print);
    KeyIndex *kx = find_index(root->data, key, cmpfunc);
    if (root->leaf && !kx_key(kx)) return NULL;
    if (root->leaf && kx_key(kx)) return remove_index(root->data, kx_index(kx));
    Node *root_ci = (Node *) get_index(root->children, kx_index(kx));
    Node *root_ci_left = (Node *) get_index(root->children, kx_index(kx) - 1);
    Node *root_ci_right = (Node *) get_index(root->children, kx_index(kx) + 1);
    // if the child (root sub c(i)) has fewer than the required number of children
    if (kx_key(kx)) {
        // any key which is in a non-leaf node will have an inorder predecessor and successor.
        Loc *inorder_p = inorder_predecessor(root, key, cmpfunc);
        Loc *inorder_s = inorder_successor(root, key, cmpfunc);
        if (len(inorder_p->node->data) >= inorder_p->node->order) {
            const void *pred = last(inorder_p->node->data);
            set_index(root->data, (void *) pred, kx_index(kx));
            return single_pass_delete(inorder_p->node, key, cmpfunc);
        } else if (len(inorder_s->node->data) >= inorder_p->node->order) {
            const void *succ = last(inorder_p->node->data);
            set_index(root->data, (void *) succ, kx_index(kx));
            return single_pass_delete(inorder_s->node, key, cmpfunc);
        } else {
            // we have to merge the nodes that contain the inorder predecessor and inorder successor around the key at
            // the index we found, then recursively delete from there.
            void *key_to_demote = remove_index(root->data, kx_index(kx));
            push(inorder_p->node->data, key_to_demote);
            join(inorder_p->node->data, inorder_s->node->data);
            if (len(inorder_p->node->children) > 0) {
                join(inorder_p->node->children, inorder_s->node->children);
            }
            remove_index(root->children, kx_index(kx) + 1);
        }
        return single_pass_delete(inorder_p->node, key, cmpfunc);
    }
    if (len(root_ci->data) < root_ci->order) {
        if (kx_index(kx) != 0 && kx_index(kx) < len(root->data)) {
            // here if the child (root sub c(i)) has both children
            if (len(root_ci_left->data) > root_ci_left->order) {
                transfer_key_right(root, kx_index(kx));
            } else if (len(root_ci_right->data) > root_ci_right->order) {
                // if (root sub c(i))'s right sibling has a spare key to donate
                transfer_key_left(root, kx_index(kx));

            } else {
                // if neither of (root sub c(i))'s immediate siblings have spare keys, we need to merge it with either
                // its left or right sibling. in this case we will always merge right, to keep things simple.
                push(root_ci->data, remove_index(root->data, kx_index(kx)));
                join(root_ci->data, root_ci_right->data);
                if (len(root_ci_right->children) > 0) join(root_ci->children, root_ci_right->children);
            }
        } else {
            // here if the child (root sub c(i)) has only one child
            if (root_ci_left) {
                // if the only sibling is the left one
                if (len(root_ci_left->data) > root_ci_left->order) {
                    // here if the only left sibling has the required number of children
                    transfer_key_right(root, kx_index(kx));
                } else {
                    // here if the only left sibling has fewer than the required number of children. if this is the
                    // case, then we need to merge root_ci with root and root_ci_left.
                    unshift(root_ci->data, remove_index(root->data, kx_index(kx) - 1));
                    root->data = root_ci->data;
                    join(root_ci_left->data, root->data);
                    root->data = root_ci_left->data;
                    if (len(root_ci_left->children) > 0) {
                        Slice *new_children = join(root_ci_left->children, root_ci->children);
                        root->children = new_children;
                    }
                }
            }
            if (root_ci_right) {
                // if the only sibling is the right one
                if (len(root_ci_right->data) > root_ci_right->order) {
                    // here if the only right sibling has the required number of children
                    transfer_key_left(root, kx_index(kx));
                } else {
                    // here if the only right sibling has fewer than the required number of children. if this is the
                    // case, then we need to merge root_ci with root and root_ci_right.
                    push(root_ci->data, remove_index(root->data, kx_index(kx)));
                    root->data = root_ci->data;
                    join(root->data, root_ci_right->data);
                    if (len(root_ci_right->children) > 0) {
                        Slice *new_children = join(root_ci->children, root_ci_right->children);
                        root->children = new_children;
                    }
                }
            }
        }
        return single_pass_delete(root_ci, key, cmpfunc);
    }
}

void *delete(BTree *tree, void *key, int(*cmpfunc)(const void *, const void *)) {
    return single_pass_delete(tree->root, key, cmpfunc);
}

// void preorder(Node *root, void(*printfunc)(const void *)) {
//     if (root) {
//         print(root->data, printfunc);
//         for (int i = 0; i < len(root->children); i++) {
//             preorder((Node *) get_index(root->children, i), printfunc);
//         }
//     }
// }

void test(Node *root, void *key, int(*cmpfunc)(const void *, const void *)) {
    KeyIndex *kx = find_index(root->data, key, cmpfunc);
    printf("%d\n", kx_index(kx));
}