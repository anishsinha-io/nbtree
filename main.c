#include <stdio.h>
#include "btree/btree.h"

void print_int(const void *key) {
    printf("%d\t", *(int *) key);
}

void print_double(const void *key) {
    printf("%f\t", *(double *) key);
}

int compare_double(const void *first, const void *second) {
    if (*(double *) first - *(double *) second < 0) return -1;
    if (*(double *) first - *(double *) second > 0) return 1;
    return 0;
}

int compare_int(const void *first, const void *second) {
    return *(int *) first - *(int *) second;
}

int main() {
    int a = 7;
    BTree *tree1 = make_btree(
            (int[]) {10, 9, 8, 7, 6, 5, 4, 3, 2}, 9, 2,
            sizeof(int), &compare_int);
    preorder(root(tree1), &print_int);
    printf("\n");
    delete(tree1, &a, &compare_int);
    preorder(root(tree1), &print_int);
    return 0;
}
