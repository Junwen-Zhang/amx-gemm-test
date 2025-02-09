#include <immintrin.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>
#include "mkl.h"
#include "packB.h"

static double gtod_ref_time_sec = 0.0;

#define INT8_GFLOPS 4096 // 2MHz
#define NUM 1

#define TILE_ROWS 16
#define TILE_COLS 64

#define ARCH_GET_XCOMP_PERM 0x1022
#define ARCH_REQ_XCOMP_PERM 0x1023
#define XFEATURE_XTILECFG 17
#define XFEATURE_XTILEDATA 18

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

// Define tile config data structure
typedef struct __tile_config // 刚好512位
{
  uint8_t palette_id;
  uint8_t start_row;
  uint8_t reserved_0[14];
  uint16_t colsb[16];
  uint8_t rows[16];
} __tilecfg;

/* Initialize tile config */
static void init_tile_config(__tilecfg *tileinfo)
{
  int i;
  tileinfo->palette_id = 1; // ???
  tileinfo->start_row = 0;

  tileinfo->rows[0] = TILE_ROWS; // C 16 * 16 * 4 bytes
  tileinfo->colsb[0] = TILE_COLS;

  tileinfo->rows[1] = TILE_ROWS; // A 16 * 64 * 1 bytes
  tileinfo->colsb[1] = TILE_COLS;

  tileinfo->rows[2] = TILE_ROWS; // B 64 * 16 * 1 bytes
  tileinfo->colsb[2] = TILE_COLS;

  _tile_loadconfig(tileinfo);
}

/* Initialize int8_t matrix */
static void init_matrix_int8(int8_t *a, int m, int n)
{
  int i, j;
  for (i = 0; i < m; i++)
    for (j = 0; j < n; j++)
      a[i * n + j] = (int8_t)(i + j);
}

static void init_matrix_uint8(uint8_t *a, int m, int n)
{
  int i, j;
  for (i = 0; i < m; i++)
    for (j = 0; j < n; j++)
      a[i * n + j] = (uint8_t)(i + j);
}

/* Initialize int32_t matrix */
static void init_matrix_int32(int32_t *a, int m, int n)
{
  int i, j;
  for (i = 0; i < m; i++)
    for (j = 0; j < n; j++)
    {
      a[i * n + j] = 0;
    }
}

/* Set_tiledata_use() - Invoke syscall to set ARCH_SET_STATE_USE */
static bool set_tiledata_use()
{
  if (syscall(SYS_arch_prctl, ARCH_REQ_XCOMP_PERM, XFEATURE_XTILEDATA))
  {
    printf("\n Fail to do XFEATURE_XTILEDATA \n\n");
    return false;
  }
  else
  {
    printf("\n TILE DATA USE SET - OK \n\n");
    return true;
  }

  return true;
}

/* Print int8_t buffer */
static void print_matrix_int8(int8_t *a, int32_t m, int32_t n)
{
  printf("[");
  for (int i = 0; i < m; i++)
  {
    for (int j = 0; j < n; j++)
    {
      printf("%d ", a[i * n + j]);
    }
    printf(",\n");
  }
  printf("]\n");
}

static void print_matrix_uint8(uint8_t *a, int32_t m, int32_t n)
{
  printf("[");
  for (int i = 0; i < m; i++)
  {
    for (int j = 0; j < n; j++)
    {
      printf("%u ", a[i * n + j]);
    }
    printf(",\n");
  }
  printf("]\n");
}

/* Print int32_t buffer */
static void print_matrix_int32(int32_t *a, int32_t m, int32_t n)
{
  printf("[");
  for (int i = 0; i < m; i++)
  {
    for (int j = 0; j < n; j++)
    {
      printf("%d ", a[i * n + j]);
    }
    printf(",\n");
  }
  printf("]\n");
}

static int Check_result(int *C, int *C1, long M, long N)
{

  int i, j, flag = 0;

  for (i = 0; i < M; i++)
  {
    for (j = 0; j < N; j++)
    {
      if (C[i * N + j] - C1[i * N + j] != 0)
      {
        printf("i = %-10d, j= %-10d\n", i, j);
        printf("C= %-10d , C1= %-10d\n", C[i * N + j], C1[i * N + j]);
        printf("结果错误!\n");
        return 0;
      }
    }
  }
  // printf("结果正确\n");
  return 1;
}

void AMX_GEMM_INT8(int32_t *C, int8_t *A, uint8_t *B, long M, long N, long K)
{
  __tilecfg tile_data = {0};
  init_tile_config(&tile_data);

  uint8_t *Bc = (uint8_t *)malloc(N * K * sizeof(uint8_t));
  packB(B, K, N, Bc);

  for (int i = 0; i < M; i += TILE_ROWS)
  {
    for (int j = 0; j < N; j += TILE_ROWS)
    {
      _tile_loadd(0, C + i * N + j, N * 4);
      // __tile_zero(0); // 为了多次计算保证结果正确，第一次加载C初始化为0
      for (int k = 0; k < K; k += TILE_COLS)
      {
        _tile_loadd(1, A + i * K + k, K);
        _tile_loadd(2, Bc + k * N + j * TILE_COLS, 64);
        _tile_dpbsud(0, 1, 2);
      }
      _tile_stored(0, C + i * N + j, N * 4);
    }
  }
  _tile_release();
}

int main()
{
  // Request permission to linux kernel to run AMX
  if (!set_tiledata_use())
    exit(-1);

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
        int8_t *A = (int8_t *)malloc(K * M * sizeof(int8_t));
        uint8_t *B = (uint8_t *)malloc(N * K * sizeof(uint8_t));
        int32_t *C = (int32_t *)malloc(M * N * sizeof(int32_t));
        int32_t *C_MKL = (int32_t *)malloc(M * N * sizeof(int32_t));

        init_matrix_int8(A, M, K);
        init_matrix_uint8(B, K, N);
        init_matrix_int32(C, M, N);
        memcpy(C_MKL, C, M * N * sizeof(int32_t));

        AMX_GEMM_INT8(C, A, B, M, N, K);
        cblas_gemm_s8u8s32(CblasRowMajor, CblasNoTrans, CblasNoTrans, offsetc,
                           M, N, K, alpha, A, K, ao, B, N, bo, beta, C_MKL, N, &co);
        
        // print_matrix_int32(C, M, N);
        // print_matrix_int32(C_MKL, M, N);
        bool flag = Check_result(C, C_MKL, M, N);

        double begin_time = dclock();
        for (int t = 0; t < loop; t++)
        {
          AMX_GEMM_INT8(C, A, B, M, N, K);
        }
        double cost = (dclock() - begin_time) / loop;

        if (flag)
        {
          printf("AMX_INT8: M= %-10ld N=%-10ld K=%-10ld effic= %.3lf\n", M, N, K, ops / cost / INT8_GFLOPS * 100 / NUM);
          fprintf(fp, "AMX_INT8: M= %-10ld N=%-10ld K=%-10ld effic= %.3lf\n", M, N, K, ops / cost / INT8_GFLOPS * 100 / NUM);
        }
        else
        {
          printf("AMX_INT8: M= %-10ld N=%-10ld K=%-10ld error!\n", M, N, K);
          fprintf(fp, "AMX_INT8: M= %-10ld N=%-10ld K=%-10ld error!\n", M, N, K);
        }

        free(A);
        free(B);
        free(C);
        free(C_MKL);
      }

  return 0;
}
