#include <immintrin.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdbool.h>

#define M 64
#define N 64
#define K 64

#define TILE_ROWS 16
#define TILE_COLS 64

#define ARCH_GET_XCOMP_PERM 0x1022
#define ARCH_REQ_XCOMP_PERM 0x1023
#define XFEATURE_XTILECFG 17
#define XFEATURE_XTILEDATA 18

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
      a[i * n + j] = (i + j) % 128;
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

/* Print int32_t buffer */
static void print_matrix_int32(int32_t *a, int32_t m, int32_t n)
{
  printf("[");
  for (int i = 0; i < M; i++)
  {
    for (int j = 0; j < n; j++)
    {
      printf("%d ", a[i * n + j]);
    }
    printf(",\n");
  }
  printf("]\n");
}

static void packB(int8_t *B, int k, int n, int8_t *Bpacked)
{
  for (int tk = 0; tk < k; tk += TILE_COLS)
    for (int tn = 0; tn < n; tn += TILE_ROWS)
      for (int i = 0; i < TILE_COLS; i += 4)
        for (int j = 0; j < TILE_ROWS; j++)
          for (int l = 0; l < 4; l++){
            Bpacked[tk * n + tn * TILE_COLS + i * TILE_ROWS + j * 4 + l] = B[tk * n + tn + i * n + j + l * n];
          }
}

int main()
{

  __tilecfg tile_data = {0};
  int8_t A[M * K]; // bug记录，如果用二维数组形式A[M][K],算地址不能直接加元素个数
  int8_t B[N * K];
  int32_t C[M * N];

  int8_t tmpA[16 * 64];
  int8_t tmpB[16 * 64];

  // Request permission to linux kernel to run AMX
  if (!set_tiledata_use())
    exit(-1);

  init_tile_config(&tile_data);

  // init data
  init_matrix_int8(A, M, K);
  printf("A = %x\n", A);
  print_matrix_int8(A, M, K);

  init_matrix_int8(B, K, N);
  printf("B = %x\n", B);
  print_matrix_int8(B, K, N);

  init_matrix_int32(C, M, N);
  printf("C = %x\n", C);
  print_matrix_int32(C, M, N);

  int8_t Bpacked[K * N];
  printf("Bpacked = %x\n", Bpacked);
  packB(B, K, N, Bpacked);
  print_matrix_int8(Bpacked, K, N);

  printf("A = %x, B = %x \n", A, B);

  for (int i = 0; i < M; i += TILE_ROWS)
  {
    for (int j = 0; j < N; j += TILE_ROWS)
    {
      _tile_loadd(0, C + i * N + j, N * 4); // Segmentation fault
      for (int k = 0; k < K; k += TILE_COLS)
      {
        printf("k = %d, ptrA = %x, ptrB = %x \n", k, A + i * K + k, Bpacked + k * N + j * TILE_COLS);
        _tile_loadd(1, A + i * K + k, K);
        _tile_loadd(2, Bpacked + k * N + j * TILE_COLS, 64);
        _tile_dpbssd(0, 1, 2);
      }
      _tile_stored(0, C + i * N + j, N * 4);
    }
  }

  printf("C: \n");
  print_matrix_int32(C, M, N);
  _tile_release();
}
