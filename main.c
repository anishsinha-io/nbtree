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
    int d = 6;
    int e = 17;
    int f = 24;
    int g = 10;
    int h = 9;
    int i = 20;
    int j = 28;
    int k = 4;
    int l = 14;
    int m = 26;
    int n = 22;
    int o = 21;
    int p = 11;
    int q = 15;
    int r = 16;
    int s = 19;
    int t = 23;
    int u = 2;
    int v = 7;
    int w = 5;
    int x = 3;
    int y = 29;
    int z = 25;
    int aa = 13;
    int bb = 1;
    int cc = 27;
    int dd = 18;
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
    delete(tree1, &d, &compare_int);
    diagnose(tree1);
    delete(tree1, &e, &compare_int);
    diagnose(tree1);
    delete(tree1, &f, &compare_int);
    diagnose(tree1);
    delete(tree1, &g, &compare_int);
    diagnose(tree1);
    delete(tree1, &h, &compare_int);
    diagnose(tree1);
    delete(tree1, &i, &compare_int);
    diagnose(tree1);
    delete(tree1, &j, &compare_int);
    diagnose(tree1);
    delete(tree1, &k, &compare_int);
    diagnose(tree1);
    delete(tree1, &l, &compare_int);
    diagnose(tree1);
    delete(tree1, &m, &compare_int);
    diagnose(tree1);
    delete(tree1, &n, &compare_int);
    diagnose(tree1);
    delete(tree1, &o, &compare_int);
    diagnose(tree1);
    delete(tree1, &p, &compare_int);
    diagnose(tree1);
    delete(tree1, &q, &compare_int);
    diagnose(tree1);
    delete(tree1, &r, &compare_int);
    diagnose(tree1);
    delete(tree1, &s, &compare_int);
    diagnose(tree1);
    delete(tree1, &t, &compare_int);
    diagnose(tree1);
    delete(tree1, &t, &compare_int);
    diagnose(tree1);
    delete(tree1, &u, &compare_int);
    diagnose(tree1);
    delete(tree1, &v, &compare_int);
    diagnose(tree1);
    delete(tree1, &w, &compare_int);
    diagnose(tree1);
    delete(tree1, &x, &compare_int);
    diagnose(tree1);
    delete(tree1, &y, &compare_int);
    diagnose(tree1);
    delete(tree1, &z, &compare_int);
    diagnose(tree1);
    delete(tree1, &z, &compare_int);
    diagnose(tree1);
    delete(tree1, &aa, &compare_int);
    diagnose(tree1);
    delete(tree1, &bb, &compare_int);
    diagnose(tree1);
    delete(tree1, &cc, &compare_int);
    diagnose(tree1);
    delete(tree1, &dd, &compare_int);
    diagnose(tree1);
    delete(tree1, &dd, &compare_int);
    diagnose(tree1);
    return 0;
}
