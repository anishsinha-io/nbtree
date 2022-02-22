#include <cslice.h>
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
    int a = 190;
    BTree *tree1 = make_btree(
            (int[]) {10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200}, 20, 2,
            sizeof(int), &compare_int);
    delete(tree1, &a, &compare_int);
    preorder(root(tree1), &print_int);
    return 0;
}
