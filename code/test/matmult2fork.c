#include "syscall.h"
#define Dim   20  /* sum total of the arrays doesn't fit in 
       * physical memory 
       */

int A[Dim][Dim];
int B[Dim][Dim];
int C[Dim][Dim];
int D[Dim][Dim];
int E[Dim][Dim];
int F[Dim][Dim];
void matmult1(){
      int i, j, k;

    for (i = 0; i < Dim; i++)   /* first initialize the matrices */
  for (j = 0; j < Dim; j++) {
       A[i][j] = i;
       B[i][j] = j;
       C[i][j] = 0;
  }

    for (i = 0; i < Dim; i++)   /* then multiply them together */
  for (j = 0; j < Dim; j++)
            for (k = 0; k < Dim; k++)
     C[i][j] += A[i][k] * B[k][j];

    Exit(C[Dim-1][Dim-1]);    /* and then we're done */
}

void matmult2(){
    int i, j, k;

    for (i = 0; i < Dim; i++)   /* first initialize the matrices */
  for (j = 0; j < Dim; j++) {
       D[i][j] = i;
       E[i][j] = j;
       F[i][j] = 0;
  }

    for (i = 0; i < Dim; i++)   /* then multiply them together */
  for (j = 0; j < Dim; j++)
            for (k = 0; k < Dim; k++)
     F[i][j] += D[i][k] * E[k][j];

    Exit(F[Dim-1][Dim-1]);    /* and then we're done */
}

int main() {
  Fork(matmult1, "matmult1", sizeof("matmult1"));
  Fork(matmult2, "matmult2", sizeof("matmult2"));
}
