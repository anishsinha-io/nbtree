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
** This file is used to run, test, and debug B-Trees
*/

#include <stdio.h>
#include "btree/btree.h"

void print_int(const void *key) {
    printf("%d\t", *(int *) key);
}

int compare_int(const void *first, const void *second) {
    return *(int *) first - *(int *) second;
}

void debug_tree(BTree *tree, void(*printfunc)(const void *)) {
    printf("PREORDER: \n");
    preorder(root(tree), printfunc);
    printf("ROOT: \n");
    print(data(root(tree)), printfunc);
    printf("-------------------------------\n");
}

int main() {
    BTree *tree1 = make_btree(
            (int[]) {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
                     28, 29, 30}, 30, 2,
            sizeof(int), &compare_int);
    debug_tree(tree1, &print_int);
    return 0;
}
