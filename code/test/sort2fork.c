#include "syscall.h"

int A[1024];  /* size of physical memory; with code, we'll run out of space!*/
int B[1024];  /* size of physical memory; with code, we'll run out of space!*/

void sort1() {
 int i, j, tmp;

    /* first initialize the array, in reverse sorted order */
    for (i = 0; i < 1024; i++)    
        A[i] = 1024 - i;

    /* then sort! */
    for (i = 0; i < 1023; i++)
        for (j = i; j < (1023 - i); j++)
     if (A[j] > A[j + 1]) { /* out of order -> need to swap ! */
        tmp = A[j];
        A[j] = A[j + 1];
        A[j + 1] = tmp;
         }
    Exit(A[0]);   /* and then we're done -- should be 0! */
}

void sort2() {
 int i, j, tmp;

    /* first initialize the array, in reverse sorted order */
    for (i = 0; i < 1024; i++)    
        B[i] = 1024 - i;

    /* then sort! */
    for (i = 0; i < 1023; i++)
        for (j = i; j < (1023 - i); j++)
     if (B[j] > B[j + 1]) { /* out of order -> need to swap ! */
        tmp = B[j];
        B[j] = B[j + 1];
        B[j + 1] = tmp;
         }
    Exit(B[0]);   /* and then we're done -- should be 0! */
}

int main() {
  Fork(sort1, "sort1", sizeof("sort1"));
  Fork(sort2, "sort2", sizeof("sort2"));
}
