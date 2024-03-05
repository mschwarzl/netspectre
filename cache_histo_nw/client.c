#include <arpa/inet.h> //inet_addr
#include <netdb.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define CACHE_MISS 19500
#define ARR_SIZE 1024
#define RECV_SIZE 3
#define BUF_LEN 4


uint64_t rdtsc() {
  uint64_t a, d;
  asm volatile("mfence");
  asm volatile("rdtsc" : "=a"(a), "=d"(d));
  a = (d << 32) | a;
  asm volatile("mfence");
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

char recvline[RECV_SIZE];

#define HIST_SIZE 1500000

#define REP 1500000

int hist_flush[HIST_SIZE];
int hist[HIST_SIZE];

int main(int argc, char **argv) {
  int flush_fd, access_fd, n;
  struct sockaddr_in flush_socket, access_socket;
  
  char* addr = "127.0.0.1";
  if(argc > 1) addr = argv[1];
  
  printf("Victim @ %s\n", addr);

  flush_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  access_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  bzero(&flush_socket, sizeof(flush_socket));
  bzero(&access_socket, sizeof(access_socket));

  flush_socket.sin_addr.s_addr = inet_addr(addr);
  flush_socket.sin_family = AF_INET;
  flush_socket.sin_port = htons(5000);

  access_socket.sin_addr.s_addr = inet_addr(addr);
  access_socket.sin_family = AF_INET;
  access_socket.sin_port = htons(6000);

  int i, j = 0, k = 0;
  uint64_t start_time = 0, end_time = 0, delta = 0;
  uint64_t start = 0, end = 0;
  char buf[BUF_LEN];

  int slen = sizeof(flush_socket);

  int send_val = 0;
  
  for (i = 0; i < REP; i++) {
    send_val = htonl(i % 11);
    
    // flushes bitwise
    sendto(flush_fd, &send_val, sizeof(send_val), 0, (struct sockaddr*) &flush_socket, slen);
    recvfrom(flush_fd, &send_val, sizeof(send_val), 0, (struct sockaddr *) &flush_socket, &slen);   
    for(int j = 0; j < 8; j++)
    {
      // access
      send_val = htonl(i % 11);
      sendto(access_fd,&send_val,sizeof(send_val), 0, (struct sockaddr*) &access_socket, slen);
      send_val = htonl(j);
      sendto(access_fd,&send_val,sizeof(send_val), 0, (struct sockaddr*) &access_socket, slen);
      start = rdtsc();
      recvfrom(access_fd, buf, BUF_LEN, 0, (struct sockaddr *) &access_socket, &slen);
      end = rdtsc();
      //printf("%s\n", recvline);
      
      delta = end - start;
      if(delta > 0 && delta < HIST_SIZE) hist_flush[delta]++;
      else printf("OOB: %zd\n", delta);
    }
  }
  
  for (i = 0; i < REP; i++) {
    // access    
    for(int j = 0; j < 8; j++)
    {
      send_val = htonl(i % 11);
      sendto(access_fd,&send_val,sizeof(send_val), 0, (struct sockaddr*) &access_socket, slen);
      send_val = htonl(j);
      sendto(access_fd,&send_val,sizeof(send_val), 0, (struct sockaddr*) &access_socket, slen);
      start = rdtsc();
      recvfrom(access_fd, buf, BUF_LEN, 0, (struct sockaddr *) &access_socket, &slen);
      end = rdtsc();
      
      delta = end - start;
      if(delta > 0 && delta < HIST_SIZE) hist[delta]++;
    }
  }
  
  // find start and end
  int min_idx = 0, max_idx = 0;
  for(i = 0; i < HIST_SIZE; i++) {
      if((hist[i] > 0 || hist_flush[i] > 0) && min_idx == 0) {
        min_idx = i;
      }
      if(hist[i] > 0 || hist_flush[i] > 0) {
          max_idx = i;
      }
  }
  
  // print
  int step = 10;
  for(i = min_idx; i < max_idx - step; i += step) {
      int sum = 0, sum_flush = 0;
      for(j = 0; j < step; j++) {
          sum += hist[i + j];
          sum_flush += hist_flush[i + j];
      }
        printf("%7d: %7d     %7d\n", i, sum, sum_flush);
  }

  
  
  FILE* f = fopen("hist.csv", "w");
  for(i = min_idx; i < max_idx - step; i += step) {
      int sum = 0, sum_flush = 0;
      for(j = 0; j < step; j++) {
          sum += hist[i + j];
          sum_flush += hist_flush[i + j];
      }
      fprintf(f, "%d,%d,%d\n", i, sum, sum_flush);
  }
  fclose(f);

}
