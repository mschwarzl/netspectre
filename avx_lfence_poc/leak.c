#include <immintrin.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

uint64_t rdtsc() {
  uint64_t a, d;
  asm volatile("mfence");
  asm volatile("rdtsc" : "=a"(a), "=d"(d));
  a = (d << 32) | a;
  asm volatile("mfence");
  return a;
}

void flush(void *p) { asm volatile("clflush 0(%0)\n" : : "c"(p) : "rax"); }

__m256i a;
__m256i b;

void execute_avx(int real_execute) {
  flush(&real_execute); // ensure speculation window is long enough
  asm volatile("mfence");

  if (real_execute) {
#ifdef LFENCE
    asm volatile("lfence");
#endif
#ifdef AVX2
    a = _mm256_or_si256(a, b); // power on AVX2 unit
#endif
  }
}

int executed[2000], not_executed[2000];

int main(int argc, char **argv) {
  int i, j = 0;

  memset(executed, 0, sizeof(executed));
  memset(not_executed, 0, sizeof(not_executed));

  usleep(50000);

  for (j = 0; j < 2000; j++) {
    asm volatile("mfence");
    for (i = 0; i < 10; i++) {
      execute_avx(1); // mistrain branch
    }
    usleep(1000);
    asm volatile("mfence");
    execute_avx(0); // speculatively execute AVX2 instruction
    asm volatile("mfence");

    size_t start = rdtsc();
    b = _mm256_and_si256(a, b); // measure AVX2 execution time
    size_t end = rdtsc();

    int delta = (int)(end - start);
    if (delta >= 0 && delta < 2000)
      executed[delta]++;

    asm volatile("mfence");
    usleep(1000);
  }

  usleep(50000);

  for (j = 0; j < 2000; j++) {
    asm volatile("mfence");
    for (i = 0; i < 10; i++) {
      execute_avx(1);
    }
    usleep(1000);
    asm volatile("mfence");
    // do not execute AVX2 instruction

    size_t start = rdtsc();
    b = _mm256_and_si256(a, b); // measure AVX2 execution time
    size_t end = rdtsc();

    int delta = (int)(end - start);
    if (delta >= 0 && delta < 2000)
      not_executed[delta]++;

    asm volatile("mfence");
    usleep(1000);
  }

  int group = 10;
  printf("T     Executed \tNotExecuted\n");
  for (i = 0; i < 2000; i += group) {
    int sum_executed = 0, sum_not_executed = 0;
    for (j = 0; j < group; j++) {
      sum_executed += executed[i + j];
      sum_not_executed += not_executed[i + j];
    }
    printf("%4d: %d \t%d\n", i, sum_executed, sum_not_executed);
  }
  return 0;
}
