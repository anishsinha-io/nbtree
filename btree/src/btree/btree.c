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

void transfer_key_right(Node *n, uint32_t i) {
    Node *right_child = (Node *) get_index(n->children, i + 1);
    Node *left_child = (Node *) get_index(n->children, i);
    void *key_to_promote = pop(left_child->data);
    if (len(left_child->children) > 0) {
        Node *child_to_transfer = pop(left_child->children);
        unshift(right_child->children, child_to_transfer);
    }
    void *key_to_demote = (void *) get_index(n->data, i);
    unshift(right_child->data, key_to_demote);
    set_index(n->data, key_to_promote, i);
}

void transfer_key_left(Node *n, uint32_t i) {
    Node *left_child = (Node *) get_index(n->children, i);
    Node *right_child = (Node *) get_index(n->children, i + 1);
    void *key_to_promote = shift(right_child->data);
    if (len(right_child->children) > 0) {
        Node *child_to_transfer = shift(right_child->children);
        push(left_child->children, child_to_transfer);
    }
    void *key_to_demote = (void *) get_index(n->data, i);
    push(left_child->data, key_to_demote);
    set_index(n->data, key_to_promote, i);
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

static bool has_left_sibling(Loc *l) {
    return loc_index(l) != 0;
}

static bool has_right_sibling(Loc *l) {
    return loc_index(l) != len(l->node->children) - 1;
}

static Loc *search_node(Node *root, void *key, int(*cmpfunc)(const void *, const void *)) {
    KeyIndex *kx = find_index(root->data, key, cmpfunc);
    return loc(root, kx);
}

static Node *get_child(Node *n, uint32_t index) {
    return (Node *) get_index(n->children, index);
}

static bool required_keys(Node *n) {
    return len(n->data) >= n->order;
}

static void shift_key_left(Node *root, uint32_t index) {
    const void *root_key = get_index(root->data, index);
    Node *left_child = (Node *) get_index(root->children, index);
    Node *right_child = (Node *) get_index(root->children, index + 1);
    set_index(root->data, shift(right_child->data), index);
    push(left_child->data, (void *) root_key);
    if (len(right_child->children) > 0) push(left_child->children, shift(right_child->children));
}

static void shift_key_right(Node *root, uint32_t index) {
    const void *root_key = get_index(root->data, index - 1);
    Node *left_child = (Node *) get_index(root->children, index - 1);
    Node *right_child = (Node *) get_index(root->children, index);
    set_index(root->data, pop(left_child->data), index - 1);
    unshift(right_child->data, (void *) root_key);
    if (len(left_child->children) > 0) unshift(right_child->children, pop(left_child->children));
}

static void merge_nodes_at_median(Node *root, uint32_t index) {
    void *root_key = remove_index(root->data, index);
    Node *left_child = (Node *) get_index(root->children, index);
    Node *right_child = (Node *) get_index(root->children, index + 1);
    push(left_child->data, root_key);
    join(left_child->data, right_child->data);
    remove_index(root->children, index + 1);
    if (len(left_child->children) > 0) join(left_child->children, right_child->children);
}

static void *single_pass_delete(Node *root, void *key, int(*cmpfunc)(const void *, const void *)) {
    KeyIndex *kx = find_index(root->data, key, cmpfunc);
    if (root->leaf && !kx_key(kx)) return NULL;
    if (root->leaf && kx_key(kx)) return remove_index(root->data, kx_index(kx));
    Node *root_ci = get_child(root, kx_index(kx));
    Node *root_ci_left = (Node *) get_index(root->children, kx_index(kx) - 1);
    Node *root_ci_right = (Node *) get_index(root->children, kx_index(kx) + 1);
    Loc *inorder_loc;
    if (!kx_key(kx)) {
        if (!required_keys(root_ci)) {
            if (root_ci_left && required_keys(root_ci_left)) {
                shift_key_right(root, kx_index(kx));
            } else if (root_ci_right && required_keys(root_ci_right)) {
                shift_key_left(root, kx_index(kx));
            } else {
                printf("here\n");
                printf("%d\n", kx_index(kx));
                for (int i = 0; i < len(root_ci_left->data); i++)
                    unshift(root->data, (void *) get_index(root_ci_left->data, i));
                root_ci_right ? join(root->data, root_ci_right->data) : root->data;
                root_ci_right && len(root_ci_right->children) > 0 ? join(root->children, root_ci_right->children)
                                                                  : root->children;
                print(root->data, &test_print);
            }
        }
    } else {
        if (root_ci_left && required_keys(root_ci_left)) {
            inorder_loc = inorder_predecessor(root_ci, key, cmpfunc);
            set_index(root_ci->data, loc_key(inorder_loc), kx_index(kx));
            return single_pass_delete(inorder_loc->node, loc_key(inorder_loc), cmpfunc);
        } else if (root_ci_right && required_keys(root_ci_right)) {
            inorder_loc = inorder_successor(root_ci, key, cmpfunc);
            set_index(root_ci->data, loc_key(inorder_loc), kx_index(kx));
            return single_pass_delete(inorder_loc->node, loc_key(inorder_loc), cmpfunc);
        } else {
            merge_nodes_at_median(root, kx_index(kx));
        }
    }
    return single_pass_delete(root_ci, key, cmpfunc);
}

void *delete(BTree *tree, void *key, int(*cmpfunc)(const void *, const void *)) {
    return single_pass_delete(tree->root, key, cmpfunc);
}

void preorder(Node *root, void(*printfunc)(const void *)) {
    if (root) {
        print(root->data, printfunc);
        for (int i = 0; i < len(root->children); i++) {
            preorder((Node *) get_index(root->children, i), printfunc);
        }
    }
}

void test(Node *root, void *key, int(*cmpfunc)(const void *, const void *)) {
    KeyIndex *kx = find_index(root->data, key, cmpfunc);
    printf("%d\n", kx_index(kx));
}