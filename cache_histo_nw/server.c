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

#define ARR_SIZE 1024
#define BUF_LEN 4


unsigned char *data = "dataSECRET";

char *mem;


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

void access_array_all_bits(int x)
{
    //maccess(data);
    //maccess(data + 8); // data is cached, x is cached anyways
    //flush(&x);
    //flush(data)
    int bit = 0;
    if(x < strlen(data) - strlen("SECRET"))
    {
        for(int i = 7; i >= 0; i--)
        {
          bit = (data[x] >> i) & 1;
          //only make accesses for 1s
          if(bit == 1)
          {
            maccess(mem + (x*8*4096+i*4096));
          }
        }
    }
}

void access_array_single(int byte,int bit)
{
   //maccess(data);
   //maccess(data + 8); // data is cached, x is cached anyways
   //flush(&x);
   //flush(data)
   maccess(mem + (byte*8*4096 + bit * 4096));
}

void *do_flush(void *x) {
  // init socket server
  int listenfd = 0, connfd = 0;
  struct sockaddr_in serv_addr;

  char sendBuff[ARR_SIZE];

  listenfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  memset(&serv_addr, 0, sizeof(serv_addr));
  memset(sendBuff, 0, sizeof(sendBuff));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(5000);

  bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

  char buf[BUF_LEN];
  // just echo back for now
  int slen = sizeof(serv_addr);
  int received_val = 0;

  while (1) {
    // read from user
    recvfrom(listenfd, &received_val, sizeof(received_val), 0, (struct sockaddr *) &serv_addr, &slen);
    received_val = ntohl(received_val);
    asm volatile("mfence");
    for(int i = 0; i <  8; i++)
    {
      flush(mem + (received_val*8*4096+i*4096));
    }
    asm volatile("mfence");
    sendto(listenfd, buf, BUF_LEN, 0, (struct sockaddr*) &serv_addr, slen);
  }
}

void *do_access(void *x) {
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
  int received_val = 0;
  int received_bit = 0;
  while (1) {
    // read from user
    recvfrom(listenfd, &received_val, sizeof(received_val), 0, (struct sockaddr *) &serv_addr, &slen);
    received_val = ntohl(received_val);
    recvfrom(listenfd, &received_bit, sizeof(received_bit), 0, (struct sockaddr *) &serv_addr, &slen);
    received_bit = ntohl(received_bit);
    asm volatile("mfence");
    access_array_single(received_val,received_bit);
    asm volatile("mfence");
    sendto(listenfd, buf, BUF_LEN, 0, (struct sockaddr*) &serv_addr, slen);
  }
}

int main(int argc, char *argv[]) {

  pthread_t thread_1, thread_2;

  // init mem
  char *_mem = malloc(4096 * 300);
  mem = (char *)(((size_t)_mem & ~0xfff) + 0x1000 * 2 + 512);
  memset(mem, 0, 4096 * 290);

  int i, j = 0, k;

  //flush whole memory beforehand
  for(j = 0; j < 256; j++) 
  {
    flush(mem + j * 4096);
  }

  
  int t = pthread_create(&thread_1, NULL, do_flush, NULL);
  t = pthread_create(&thread_2, NULL, do_access, NULL);

  pthread_join(thread_1, NULL);
  pthread_join(thread_2, NULL);

  return 0;
}
