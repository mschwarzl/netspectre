#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <immintrin.h>


#define ARR_SIZE 1024
#define BUF_LEN 4


void *do_access(void *x) {
  
  __m256i a;
  __m256i b;
  

  // init socket server
  int listenfd = 0, connfd = 0;
  struct sockaddr_in serv_addr;

  char sendBuff[ARR_SIZE];

  listenfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  memset(&serv_addr, 0, sizeof(serv_addr));
  memset(sendBuff, 0, sizeof(sendBuff));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(6000);

  bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

  char buf[BUF_LEN];
  // just echo back for now
  char ret_val;
  int slen = sizeof(serv_addr);

  a = _mm256_or_si256(a, b);
  while (1) {
    // read from user
    recvfrom(listenfd, buf, BUF_LEN, 0, (struct sockaddr *) &serv_addr, &slen);
    asm volatile("mfence");
    b = _mm256_and_si256(a, b);
    asm volatile("mfence");
    sendto(listenfd, buf, BUF_LEN, 0, (struct sockaddr*) &serv_addr, slen);
    //asm volatile("cpuid"::"a"(0),"c"(0):"memory");
  }
}

int main(int argc, char *argv[]) {

  pthread_t thread_1, thread_2;

  int i, j = 0, k;
  //int t = pthread_create(&thread_1, NULL, do_flush, NULL);
  int t = pthread_create(&thread_2, NULL, do_access, NULL);

  //pthread_join(thread_1, NULL);
  pthread_join(thread_2, NULL);

  return 0;
}
