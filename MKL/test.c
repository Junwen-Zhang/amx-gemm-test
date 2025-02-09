#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include "mkl.h"
#include <omp.h>
// #include <cblas.h>
#include <math.h>
#include <time.h>
#include <cstdint>

#define NUM 1
#define INT8_GFLOPS 4096 // AMX 2MHz

static double gtod_ref_time_sec = 0.0;

double dclock()
{
    double the_time, norm_sec;
    struct timeval tv;

    gettimeofday(&tv, NULL);

    if (gtod_ref_time_sec == 0.0)
        gtod_ref_time_sec = (double)tv.tv_sec;

    norm_sec = (double)tv.tv_sec - gtod_ref_time_sec;

    the_time = norm_sec + tv.tv_usec * 1.0e-6;

    return the_time;
}

void random_matrix_int8(int m, int n, MKL_INT8 *a)
{
    // 设置随机数种子
    // srand(time(NULL));
    int i, j;

    for (i = 0; i < m; i++)
        for (j = 0; j < n; j++)
            a[i * n + j] = (MKL_INT8)(rand() % 256 - 128);
}

void random_matrix_uint8(int m, int n, MKL_UINT8 *a)
{
    // 设置随机数种子
    // srand(time(NULL));
    int i, j;

    for (i = 0; i < m; i++)
        for (j = 0; j < n; j++)
            a[i * n + j] = (MKL_UINT8)(rand() % 256);
}

void regular_matrix_int8(long m, long n, MKL_INT8 *a)
{
    long i, j;

    for (i = 0; i < m; i++)
        for (j = 0; j < n; j++)
            a[i * n + j] = (int8_t)((i + j) % 128);
}

void regular_matrix_uint8(long m, long n, MKL_UINT8 *a)
{
    long i, j;

    for (i = 0; i < m; i++)
        for (j = 0; j < n; j++)
            a[i * n + j] = (uint8_t)((i + j) % 128);
}

void show_matrix(long m, long n, int *a)
{
    long i, j;

    for (i = 0; i < m; i++)
    {
        for (j = 0; j < n; j++)
            printf("%d ", a[i * n + j]);
        printf("\n");
    }
}

void show_matrix_int8(long m, long n, MKL_INT8 *a)
{
    long i, j;

    for (i = 0; i < m; i++)
    {
        for (j = 0; j < n; j++)
            printf("%d ", a[i * n + j]);
        printf("\n");
    }
}

void show_matrix_uint8(long m, long n, MKL_UINT8 *a)
{
    long i, j;

    for (i = 0; i < m; i++)
    {
        for (j = 0; j < n; j++)
            printf("%d ", a[i * n + j]);
        printf("\n");
    }
}

int main()
{
    mkl_set_num_threads(NUM);
    omp_set_num_threads(NUM);

    int i, j, k, jj, pc;
    long M, N, K;
    double start, cost;
    double gflops;
    int loop = 10;
    int lda, ldb, ldc;
    int flag = 0;
    float alpha = 1.0, beta = 0.0;
    const MKL_INT8 ao = 0;
    const MKL_INT8 bo = 0;
    MKL_INT32 co = 0;
    CBLAS_OFFSET offsetc = CblasFixOffset;

    FILE *fp;
    if ((fp = fopen("./result.txt", "w")) == NULL)
    {
        puts("Fail to open file!");
        exit(0);
    }

    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            for (int k = 0; k < 4; k++)
            {
                long M = 2 << (5 + i);
                long N = 2 << (5 + j);
                long K = 1024 << k;

                double ops = (double)M * N * K * 1.0e-09 * 2;
                MKL_INT8 *A = (MKL_INT8 *)malloc(M * K * sizeof(MKL_INT8));
                MKL_UINT8 *B = (MKL_UINT8 *)malloc(K * N * sizeof(MKL_UINT8));
                MKL_INT32 *C = (MKL_INT32 *)malloc(M * N * sizeof(MKL_INT32));

                // init matrix
                random_matrix_int8(M, K, A);
                random_matrix_uint8(K, N, B);

                // for (int t = 0; t < 5; t++)
                // {
                //     cblas_gemm_s8u8s32(CblasRowMajor, CblasNoTrans, CblasNoTrans, offsetc,
                //                        M, N, K, alpha, A, K, ao, B, N, bo, beta, C, N, &co);
                // }

                double begin_time = dclock();
                for (int t = 0; t < loop; t++)
                {
                    cblas_gemm_s8u8s32(CblasRowMajor, CblasNoTrans, CblasNoTrans, offsetc,
                                       M, N, K, alpha, A, K, ao, B, N, bo, beta, C, N, &co);
                }
                double cost = (dclock() - begin_time) / loop;

                printf("MKL_INT8: M= %-10ld N=%-10ld K=%-10ld effic= %.3lf\n", M, N, K, ops / cost / INT8_GFLOPS * 100 / NUM);
                fprintf(fp, "MKL_INT8: M= %-10ld N=%-10ld K=%-10ld effic= %.3lf\n", M, N, K, ops / cost / INT8_GFLOPS * 100 / NUM);
                // fprintf(fp, "%-10d%-10d%-10d%.3lf\n", M, N, K, ops / cost / INT8_GFLOPS * 100 / NUM);

                free(A);
                free(B);
                free(C);
            }

    return 0;
}
