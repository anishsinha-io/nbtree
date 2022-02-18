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

int main() {
    BTree *tree1 = btree(3);
    double a = 100;
    double b = 200;
    double c = 300;
    double d = 400;
    double e = 500;
    double f = 600;
    double g = 700;
    double h = 800;
    double i = 900;
    double j = 1000;
    double k = 1100;
    double l = 1200;
    double m = 50;
    double n = 75;
    double o = 80;
    double p = 90;
    double q = 110;
    double r = 120;
    double s = 130;
    double t = 450;
    insert(tree1, &a, &compare_double);
    insert(tree1, &b, &compare_double);
    insert(tree1, &c, &compare_double);
    insert(tree1, &d, &compare_double);
    insert(tree1, &e, &compare_double);
    insert(tree1, &f, &compare_double);
    insert(tree1, &g, &compare_double);
    insert(tree1, &h, &compare_double);
    insert(tree1, &i, &compare_double);
    insert(tree1, &j, &compare_double);
    insert(tree1, &k, &compare_double);
    insert(tree1, &l, &compare_double);
    insert(tree1, &m, &compare_double);
    insert(tree1, &n, &compare_double);
    insert(tree1, &o, &compare_double);
    insert(tree1, &p, &compare_double);
    insert(tree1, &q, &compare_double);
    insert(tree1, &r, &compare_double);
    insert(tree1, &s, &compare_double);
    insert(tree1, &t, &compare_double);
    preorder(root(tree1), &print_double);

    return 0;
}
