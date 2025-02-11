#include <stdint.h>

__attribute__((constructor)) void define_macros()
{
	asm volatile(
			".macro PACKB_N64K8                                          \n"
			// 加载数据，K8
			"  leaq    (%%r10, %%rax, 1), %%r11                          \n"
			"  leaq    (%%r11, %%rax, 1), %%r12                          \n"
			"  leaq    (%%r12, %%rax, 1), %%r13                          \n"
			"  vmovups (%%r10), %%zmm0                                   \n"
			"  vmovups (%%r11), %%zmm1                                   \n"
			"  vmovups (%%r12), %%zmm2                                   \n"
			"  vmovups (%%r13), %%zmm3                                   \n"
			"  leaq    (%%r13, %%rax, 1), %%r10                          \n"
			"  leaq    (%%r10, %%rax, 1), %%r11                          \n"
			"  leaq    (%%r11, %%rax, 1), %%r12                          \n"
			"  leaq    (%%r12, %%rax, 1), %%r13                          \n"
			"  vmovups (%%r10), %%zmm16                                  \n"
			"  vmovups (%%r11), %%zmm17                                  \n"
			"  vmovups (%%r12), %%zmm18                                  \n"
			"  vmovups (%%r13), %%zmm19                                  \n"
			"  leaq    (%%r13, %%rax, 1), %%r10                          \n"

			// 交错数据，K4
			"  vpunpcklbw  %%zmm1, %%zmm0, %%zmm4                        \n"
			"  vpunpckhbw  %%zmm1, %%zmm0, %%zmm5                        \n"
			"  vpunpcklbw  %%zmm3, %%zmm2, %%zmm6                        \n"
			"  vpunpckhbw  %%zmm3, %%zmm2, %%zmm7                        \n"

			"  vpunpcklwd  %%zmm6, %%zmm4, %%zmm8                        \n"
			"  vpunpckhwd  %%zmm6, %%zmm4, %%zmm9                        \n"
			"  vpunpcklwd  %%zmm7, %%zmm5, %%zmm10                       \n"
			"  vpunpckhwd  %%zmm7, %%zmm5, %%zmm11                       \n"

			"  vextracti32x4   $0x1, %%zmm8, %%xmm12                     \n"
			"  vextracti32x4   $0x1, %%zmm9, %%xmm13                     \n"
			"  vextracti32x4   $0x1, %%zmm10, %%xmm14                    \n"
			"  vextracti32x4   $0x1, %%zmm11, %%xmm15                    \n"
			"  vinserti32x4    $0x1, %%xmm13, %%zmm12, %%zmm12           \n"
			"  vinserti32x4    $0x2, %%xmm14, %%zmm12, %%zmm12           \n"
			"  vinserti32x4    $0x3, %%xmm15, %%zmm12, %%zmm12           \n"
			"  vmovups         %%zmm12, 1024(%%rcx)                      \n"

			"  vextracti32x4   $0x2, %%zmm8, %%xmm4                      \n"
			"  vextracti32x4   $0x2, %%zmm9, %%xmm5                      \n"
			"  vextracti32x4   $0x2, %%zmm10, %%xmm6                     \n"
			"  vextracti32x4   $0x2, %%zmm11, %%xmm7                     \n"
			"  vinserti32x4    $0x1, %%xmm5, %%zmm4, %%zmm4              \n"
			"  vinserti32x4    $0x2, %%xmm6, %%zmm4, %%zmm4              \n"
			"  vinserti32x4    $0x3, %%xmm7, %%zmm4, %%zmm4              \n"
			"  vmovups         %%zmm4, 2048(%%rcx)                       \n"

			"  vextracti32x4   $0x3, %%zmm8, %%xmm0                      \n"
			"  vextracti32x4   $0x3, %%zmm9, %%xmm1                      \n"
			"  vextracti32x4   $0x3, %%zmm10, %%xmm2                     \n"
			"  vextracti32x4   $0x3, %%zmm11, %%xmm3                     \n"
			"  vinserti32x4    $0x1, %%xmm1, %%zmm0, %%zmm0              \n"
			"  vinserti32x4    $0x2, %%xmm2, %%zmm0, %%zmm0              \n"
			"  vinserti32x4    $0x3, %%xmm3, %%zmm0, %%zmm0              \n"
			"  vmovups         %%zmm0, 3072(%%rcx)                       \n"

			"  vinserti32x4    $0x1, %%xmm9, %%zmm8, %%zmm8              \n"
			"  vinserti32x4    $0x2, %%xmm10, %%zmm8, %%zmm8              \n"
			"  vinserti32x4    $0x3, %%xmm11, %%zmm8, %%zmm8              \n"
			"  vmovups         %%zmm8, (%%rcx)                           \n"

			"  addq            $64, %%rcx                                \n" // k += 4

			// 交错数据，下一个K4
			"  vpunpcklbw  %%zmm17, %%zmm16, %%zmm20                     \n"
			"  vpunpckhbw  %%zmm17, %%zmm16, %%zmm21                     \n"
			"  vpunpcklbw  %%zmm19, %%zmm18, %%zmm22                     \n"
			"  vpunpckhbw  %%zmm19, %%zmm18, %%zmm23                     \n"

			"  vpunpcklwd  %%zmm22, %%zmm20, %%zmm24                     \n"
			"  vpunpckhwd  %%zmm22, %%zmm20, %%zmm25                     \n"
			"  vpunpcklwd  %%zmm23, %%zmm21, %%zmm26                     \n"
			"  vpunpckhwd  %%zmm23, %%zmm21, %%zmm27                     \n"

			"  vextracti32x4   $0x1, %%zmm24, %%xmm28                    \n"
			"  vextracti32x4   $0x1, %%zmm25, %%xmm29                    \n"
			"  vextracti32x4   $0x1, %%zmm26, %%xmm30                    \n"
			"  vextracti32x4   $0x1, %%zmm27, %%xmm31                    \n"
			"  vinserti32x4    $0x1, %%xmm29, %%zmm28, %%zmm28           \n"
			"  vinserti32x4    $0x2, %%xmm30, %%zmm28, %%zmm28           \n"
			"  vinserti32x4    $0x3, %%xmm31, %%zmm28, %%zmm28           \n"
			"  vmovups         %%zmm28, 1024(%%rcx)                      \n"

			"  vextracti32x4   $0x2, %%zmm24, %%xmm20                    \n"
			"  vextracti32x4   $0x2, %%zmm25, %%xmm21                    \n"
			"  vextracti32x4   $0x2, %%zmm26, %%xmm22                    \n"
			"  vextracti32x4   $0x2, %%zmm27, %%xmm23                    \n"
			"  vinserti32x4    $0x1, %%xmm21, %%zmm20, %%zmm20           \n"
			"  vinserti32x4    $0x2, %%xmm22, %%zmm20, %%zmm20           \n"
			"  vinserti32x4    $0x3, %%xmm23, %%zmm20, %%zmm20           \n"
			"  vmovups         %%zmm20, 2048(%%rcx)                      \n"

			"  vextracti32x4   $0x3, %%zmm24, %%xmm16                    \n"
			"  vextracti32x4   $0x3, %%zmm25, %%xmm17                    \n"
			"  vextracti32x4   $0x3, %%zmm26, %%xmm18                    \n"
			"  vextracti32x4   $0x3, %%zmm27, %%xmm19                    \n"
			"  vinserti32x4    $0x1, %%xmm17, %%zmm16, %%zmm16           \n"
			"  vinserti32x4    $0x2, %%xmm18, %%zmm16, %%zmm16           \n"
			"  vinserti32x4    $0x3, %%xmm19, %%zmm16, %%zmm16           \n"
			"  vmovups         %%zmm16, 3072(%%rcx)                      \n"

			"  vinserti32x4    $0x1, %%xmm25, %%zmm24, %%zmm24           \n"
			"  vinserti32x4    $0x2, %%xmm26, %%zmm24, %%zmm24           \n"
			"  vinserti32x4    $0x3, %%xmm27, %%zmm24, %%zmm24           \n"
			"  vmovups         %%zmm24, (%%rcx)                          \n"

			"  addq            $64, %%rcx                                \n" // k += 4

			".endm                                                       \n"
			:
			:
			: "rax", "rbx", "rcx", "rdx", "rdi", "rsi", "rbp", "r8", "r9", "r10", "r11", "r12",
				"r13", "r14", "r15", "zmm0", "zmm1", "zmm2", "zmm3", "zmm4", "zmm5",
				"zmm6", "zmm7", "zmm8", "zmm9", "zmm10", "zmm11", "zmm12", "zmm13",
				"zmm14", "zmm15", "zmm16", "zmm17", "zmm18", "zmm19", "zmm20", "zmm21",
				"zmm22", "zmm23", "zmm24", "zmm25", "zmm26", "zmm27", "zmm28", "zmm29",
				"zmm30", "zmm31", "memory", "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7",
				"xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "k1");
}

void packB_k64n64(uint8_t *B, uint8_t *Bc, long LN)
{
	asm volatile(
			"mov %[B], %%rbx                                             \n"
			"mov %[Bc], %%rcx                                            \n"
			"mov %[LN], %%rax                                         \n"

			"movq    %%rbx, %%r10                                        \n"

			"PACKB_N64K8                                                 \n"
			"PACKB_N64K8                                                 \n"
			"PACKB_N64K8                                                 \n"
			"PACKB_N64K8                                                 \n"
			"PACKB_N64K8                                                 \n"
			"PACKB_N64K8                                                 \n"
			"PACKB_N64K8                                                 \n"
			"PACKB_N64K8                                                 \n"

			:
			:
			[B] "m"(B),
			[Bc] "m"(Bc),
			[LN] "m"(LN) 
			: "rax", "rbx", "rcx", "rdx", "rdi", "rsi", "rbp", "r8", "r9", "r10", "r11", "r12",
				"r13", "r14", "r15", "zmm0", "zmm1", "zmm2", "zmm3", "zmm4", "zmm5",
				"zmm6", "zmm7", "zmm8", "zmm9", "zmm10", "zmm11", "zmm12", "zmm13",
				"zmm14", "zmm15", "zmm16", "zmm17", "zmm18", "zmm19", "zmm20", "zmm21",
				"zmm22", "zmm23", "zmm24", "zmm25", "zmm26", "zmm27", "zmm28", "zmm29",
				"zmm30", "zmm31", "memory", "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7",
				"xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "k1");
}

void packB(uint8_t *B, int k, int n, uint8_t *Bc)
{
	for (int i = 0; i < k; i += 64)
	{
		for (int j = 0; j < n; j += 64)
		{
			int remaining_n = n - j;
			if (remaining_n >= 64)
			{
				// printf("i=%-10d, j=%-10d\n", i, j);
				packB_k64n64(B + i * n + j, Bc, (long)n); // LN是int类型，强制转换为64位
				Bc += 64 * 64;
			}
			// else if (remaining_n >= 32) {
			//     packB_k64n32(B + i * n + j, Bpacked, n);
			//     Bpacked += 64 * 32;
			// } else if (remaining_n >= 16) {
			//     packB_k64n16(B + i * n + j, Bpacked, n);
			//     Bpacked += 64 * 16;
			// }
		}
	}
}