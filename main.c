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

void diagnose(BTree *tree) {
    printf("PREORDER: \n");
    preorder(root(tree), &print_int);
    printf("ROOT: \n");
    print(data(root(tree)), &print_int);
    printf("-------------------------------\n");
}

int main() {
    int a = 12;
    int b = 8;
    int c = 30;
    BTree *tree1 = make_btree(
            (int[]) {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
                     28, 29, 30}, 30, 2,
            sizeof(int), &compare_int);
    diagnose(tree1);
    delete(tree1, &a, &compare_int);
    diagnose(tree1);
    delete(tree1, &b, &compare_int);
    diagnose(tree1);
    delete(tree1, &c, &compare_int);
    diagnose(tree1);
    return 0;
}
