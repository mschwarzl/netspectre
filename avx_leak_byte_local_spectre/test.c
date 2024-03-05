#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <sched.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <immintrin.h>

uint64_t rdtsc() {
  uint64_t a, d;
  asm volatile ("mfence");
  asm volatile ("rdtsc" : "=a" (a), "=d" (d));
  a = (d<<32) | a;
  asm volatile ("mfence");
  return a;
}

int main(int argc, char *argv[])
{
  __m256i a; __m256i b;
  uint64_t start = 0,end=0,delta;

  while(1)
  {
    asm volatile("lfence");
    start = rdtsc();
    b = _mm256_and_si256(a, b);
    end = rdtsc();
    printf("Cold: %ld\n",end-start);

    for(int i = 0;i < 15;i++)
      b = _mm256_and_si256(a, b);
    
    start = rdtsc();
    b = _mm256_and_si256(a, b);
    end = rdtsc();  
    printf("warm: %ld\n",end-start);

    asm volatile("mfence");
    usleep(100);
    asm volatile("mfence");
  }

  return 0;
}
