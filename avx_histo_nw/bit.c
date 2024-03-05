#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sched.h>
#include <string.h>
#include <unistd.h>
#include <immintrin.h>

unsigned char* data = "dataSECRET";

uint64_t rdtsc() {
  uint64_t a, d;
  asm volatile ("mfence");
  asm volatile ("rdtsc" : "=a" (a), "=d" (d));
  a = (d<<32) | a;
  asm volatile ("mfence");
  return a;
}


void maccess(void* p)
{
  asm volatile ("movq (%0), %%rax\n"
    :
    : "c" (p)
    : "rax");
}

void flush(void* p) {
    asm volatile ("clflush 0(%0)\n"
      :
      : "c" (p)
      : "rax");
}


int flush_reload(void* ptr) {
        uint64_t start = 0, end = 0;
        
        start = rdtsc();
        maccess(ptr);
        end = rdtsc();
        
        flush(ptr); 
        
        if(end - start < CACHE_MISS) {
            return 1;
        }
        return 0;
}

char* mem;



int hist_valid[1000];
int hist_invalid[1000];

int main(int argc, char **argv) {
    
    int j = 0;
    size_t rep = 0;
    srand(rdtsc());
    
    __m256i a;
    __m256i b;
    
    for(j = 0; j < 100000; j++) {
        b = _mm256_and_si256(a, b);
        a = _mm256_or_si256(a, b);
        b = _mm256_and_si256(a, b);
        
        size_t start = rdtsc();
        b = _mm256_and_si256(a, b);
        //a = _mm256_or_si256(a, b);
        //b = _mm256_and_si256(a, b);
        size_t end = rdtsc();
        rep++;
        int delta = (int)(end - start);
        if(delta < 1000) hist_invalid[delta]++;
    }
    printf("Warmed up done\n");
    
    usleep(5);
    
    for(j = 0; j < 100000; j++) {
        size_t start = rdtsc();
        b = _mm256_and_si256(a, b);
        //a = _mm256_or_si256(a, b);
        //b = _mm256_and_si256(a, b);
        size_t end = rdtsc();
        //usleep(1);
        asm volatile("cpuid"::"a"(0),"c"(0):"memory");
        usleep(1);
        rep++;
        int delta = (int)(end - start);
        if(delta < 1000) hist_valid[delta]++;
    }
    
  FILE* f = fopen("hist.csv", "w");
  int step = 1;
  for(i = 0; i < 1000 - step; i += step) {
      int sum = 0, sum_flush = 0;
      for(j = 0; j < step; j++) {
          sum += hist_valid[i + j];
          sum_flush += hist_invalid[i + j];
      }
      fprintf(f, "%d,%d,%d\n", i, sum_flush, sum);
  }
  fclose(f);
    
}
