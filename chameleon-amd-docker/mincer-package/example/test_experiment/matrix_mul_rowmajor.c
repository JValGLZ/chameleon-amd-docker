#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 256

void matmul_row_major(double A[N][N], double B[N][N], double C[N][N]) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            C[i][j] = 0.0;
            for (int k = 0; k < N; k++)
                C[i][j] += A[i][k] * B[k][j];
        }
}

int main() {
    static double A[N][N], B[N][N], C[N][N];

    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            A[i][j] = i + j;
            B[i][j] = i - j;
        }

    clock_t start = clock();
    matmul_row_major(A, B, C);
    clock_t end = clock();

    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Row-major matrix multiplication completed in %.6f seconds.\n", elapsed);
    return 0;
}
