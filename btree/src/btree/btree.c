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

Slice *data(Node *n) {
    return n->data;
}

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
    if (len(root->data) > 0) {
        print(root->data, printfunc);
        if (len(root->children) < 1) return;
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
    if (len(sender->children) > 0) push(receiver->children, shift(sender->children));
    set_index(root->data, key_to_promote, index);
}

static void transfer_key_right(Node **root, uint32_t index) {
    Node *sender = (Node *) get_index((*root)->children, index - 1);
    Node *receiver = (Node *) get_index((*root)->children, index);
    void *key_to_promote = pop(sender->data);
    void *key_to_demote = (void *) get_index((*root)->data, index - 1);
    unshift(receiver->data, key_to_demote);
    if (len(sender->children) > 0) unshift(receiver->children, pop(sender->children));
    set_index((*root)->data, key_to_promote, index - 1);
}

static void *single_pass_delete(Node **root, void *key, int(*cmpfunc)(const void *, const void *)) {
    // search for the key in a node called root.
    KeyIndex *kx = find_index((*root)->data, key, cmpfunc);
    // we will use index a lot, so i'm saving it into a variable.
    uint32_t index = kx_index(kx);
    // CASE 1
    // if we found the key in a leaf node then we can just remove and return it. leaves don't have children so there's
    // nothing to worry about there. If we're at a leaf node, and we haven't found the key, we can just return a null
    // pointer.
    if (kx_key(kx) && (*root)->leaf) {
        return remove_index((*root)->data, index);
    }
    if (!kx_key(kx) && (*root)->leaf) {
        printf("key not found!\n");
        return NULL;
    }
    // CASE 2
    // if we found the key in an internal node (root), we need to take the following steps:
    //      1. if the child that precedes the key (root->children[index]) has a length greater than or equal to the
    //         minimum order of the tree, then we need to find the inorder predecessor inorder_p in the subtree rooted
    //         at root->children[index]. Otherwise,
    //      2. if the child that succeeds the key (root->children[index+1]) has a length greater than or equal to the
    //         minimum order of the tree, then we need to find the inorder successor inorder_s in the subtree rooted
    //         at root->children[index+1]. Otherwise,
    //      3. if neither the child preceding the key nor the child succeeding the key have sufficient keys to proceed
    //         with steps 1 or 2, then we must merge the data in the root with the left child, then the data in the
    //         right child with the left child. then we remove the right child from root's array of child pointers, and
    //         if the root no longer has any keys, we set the left child as the new root. if the left child has more
    //         than zero children, then the right child must also have more than zero children, so we must merge the
    //         left child's children with the right child's children.
    if (kx_key(kx)) {
        Node *root_ci = (Node *) get_index((*root)->children, index);
        Node *root_ci_right = (Node *) get_index((*root)->children, index + 1);
        if (len(root_ci->data) >= root_ci->order) {
            // CASE 2.1
            // find the inorder predecessor
            Loc *inorder_p = inorder_predecessor(*root, key, cmpfunc);
            // then recursively delete the key again.
            single_pass_delete(root, loc_key(inorder_p), cmpfunc);
            set_index((*root)->data, loc_key(inorder_p), index);
            return kx_key(kx);
        } else if (len(root_ci_right->data) >= root_ci_right->order) {
            // CASE 2.2
            // find the inorder successor
            Loc *inorder_s = inorder_successor((*root), key, cmpfunc);
            // then recursively delete the key again
            single_pass_delete(root, loc_key(inorder_s), cmpfunc);
            set_index((*root)->data, loc_key(inorder_s), index);
            return kx_key(kx);
        } else {
            // CASE 2.3
            // neither the immediate left nor right child to the key found in root have sufficient keys to perform the
            // above steps. therefore, we need to merge root_ci->data, root->data, and root_ci_right->data around the
            // key to be deleted and then recursively delete the key from there. if the root's length becomes zero,
            // then we reassign the root to be root_ci.
            // create the data
            Slice *data_builder = root_ci->data;
            push(data_builder, remove_index((*root)->data, index));
            join(data_builder, root_ci_right->data);
            if (len(root_ci->children) > 0) join(root_ci->children, root_ci_right->children);
            if (len((*root)->data) == 0) *root = root_ci;
            else remove_index((*root)->children, index + 1);
            return single_pass_delete(root, key, cmpfunc);
        }
    } else {
        // CASE 3
        // if we don't find the key in root we proceed with the following. we first determine the root (root sub c_i)
        // that is the root of the subtree that must contain the key. This is root_ci in our case. If root_ci only has
        // root_ci->order-1 keys (the minimum), we need to proceed with steps 3.1 or 3.2 to guarantee that we descend
        // to a node that contains at least root_ci->order keys.

        Node *root_ci = (Node *) get_index((*root)->children, index);

        // immediate left sibling
        Node *root_ci_left = (Node *) get_index((*root)->children, index - 1);

        // immediate right sibling
        Node *root_ci_right = (Node *) get_index((*root)->children, index + 1);
        // we may have cases where root_ci is the first or last child, in which case there will only be one sibling.
        // however, there will never be a case in which there is no sibling, because that would violate one of the
        // constraints of a B-Tree, that is that all leaf nodes must be on the same level.

        // the first if statement handles the case where both siblings of root_ci are defined. the second handles the
        // case where only the left sibling is defined and the last handles the case where only the right sibling is
        // defined

        if (len(root_ci->data) < root_ci->order) {
            if (root_ci_left && root_ci_right) {
                if (len(root_ci_left->data) >= root_ci_left->order) {
                    transfer_key_right(root, index);
                } else if (len(root_ci_right->data) >= root_ci_right->order) {
                    transfer_key_left(*root, index);
                } else {
                    push(root_ci_left->data, remove_index((*root)->data, index - 1));
                    join(root_ci_left->data, root_ci->data);
                    if (len(root_ci_left->children) > 0) join(root_ci_left->children, root_ci->children);
                    if (len((*root)->data) == 0) *root = root_ci_left;
                    else remove_index((*root)->children, index);
                    return single_pass_delete(root, key, cmpfunc);
                }
            } else if (root_ci_left) {
                if (len(root_ci_left->data) >= root_ci_left->order) {
                    // if we are here, we need to donate a key from the immediate left sibling to the node being
                    // descended to
                    transfer_key_right(root, index);
                } else {
                    push(root_ci_left->data, remove_index((*root)->data, index - 1));
                    join(root_ci_left->data, root_ci->data);
                    if (len(root_ci_left->children) > 0) join(root_ci_left->children, root_ci->children);
                    if (len((*root)->data) == 0) *root = root_ci_left;
                    else remove_index((*root)->children, index);
                    return single_pass_delete(root, key, cmpfunc);
                }
            } else {
                if (len(root_ci_right->data) >= root_ci_right->order) {
                    // if we are here, we need to donate a key from the immediate right sibling to the node being
                    // descended to
                    transfer_key_left(*root, index);
                } else {
                    push(root_ci->data, remove_index((*root)->data, index));
                    join(root_ci->data, root_ci_right->data);

                    if (len(root_ci->children) > 0) join(root_ci->children, root_ci_right->children);
                    if (len((*root)->data) == 0) *root = root_ci;
                    else remove_index((*root)->children, index + 1);
                    return single_pass_delete(root, key, cmpfunc);
                }
            }
        }
        return single_pass_delete(&root_ci, key, cmpfunc);
    }
}

void *delete(BTree *tree, void *key, int(*cmpfunc)(const void *, const void *)) {
    return single_pass_delete(&(tree->root), key, cmpfunc);
}
