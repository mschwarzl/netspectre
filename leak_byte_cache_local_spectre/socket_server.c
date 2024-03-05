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


#define ARR_SIZE 1024

char* data = "dataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRET";

char* mem;

//note this size depends strongly on your CPU
char spam[1024*1024*12];


uint64_t rdtsc() {
  uint64_t a, d;
  asm volatile ("mfence");
  asm volatile ("rdtsc" : "=a" (a), "=d" (d));
  a = (d<<32) | a;
  asm volatile ("mfence");
  return a;
}


#define CACHE_MISS 180
void maccess(void* p)
{
  asm volatile ("movq (%0), %%rax\n"
    :
    : "c" (p)
    : "rax");
}

void flush(void* p) 
{
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

char access_array(int x,int bit) 
{
    //maccess(data);
    //maccess(data + 8); // data is cached, x is cached anyways
    
    //if variable gets flushed beforehand speculation
    //is more likely
    flush(&x);
    flush(&bit);
    flush(data);

    int index = x * 8 * 4096 + bit*4096;
    if(x < (strlen(data)) - strlen("SECRET") - 1990)
    {
      //patch
      //asm volatile("lfence");
      return mem[index];
    }
}

#define BUF_LEN 4
void* do_flush(void *x) {
  // init socket server
  int listenfd = 0;
  struct sockaddr_in serv_addr;

  char sendBuff[ARR_SIZE];

  listenfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  memset(&serv_addr, 0, sizeof(serv_addr));
  memset(sendBuff, 0, sizeof(sendBuff));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(6001);

  bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

  char buf[BUF_LEN];
  // just echo back for now
  socklen_t slen = sizeof(serv_addr);
  int received_val = 0;

  while (1) {
    // read from user
    recvfrom(listenfd, &received_val, sizeof(received_val), 0, (struct sockaddr *) &serv_addr, &slen);
    received_val = ntohl(received_val);
    asm volatile("mfence");
    //memset(spam,0,sizeof(spam));
    //using flush its a lot faster, but memset simulates the file download
    for(int i = 0; i <  8; i++)
    {
      flush(mem + (received_val*8*4096+i*4096));
    }
    asm volatile("mfence");
    sendto(listenfd, buf, BUF_LEN, 0, (struct sockaddr*) &serv_addr, slen);
  }
}

void* restricted_access(void *x)
{
    //init socket server
    struct sockaddr_in serv_addr,cli_addr; 

    int sockfd; 
    socklen_t slen = sizeof(cli_addr), recv_len;     
    //create a UDP socket
    if ((sockfd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
      printf("Error when creating socket\n");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);

    //bind socket to port
    if(bind(sockfd,(struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
    {
      printf("error on binding...");
    }
    
    unsigned int received_num = 0;
    //just echo back for now

    //blocking on
    //int iMode = 0;
    //ioctl(listenfd, FIONBIO, &iMode);
    char ret_val[] = "OK";

    int received_byte = 0;
    int received_bit = 0;
    while(1)
    {
      //read from user
      //this should really block
      if((recv_len = recvfrom(sockfd,&received_num,sizeof(received_num), 0, (struct sockaddr *) &cli_addr, &slen)) == -1)
      {
        perror("recv error");
      }
      
      received_num = ntohl(received_num);
      received_byte = received_num / 8;
      received_bit = received_num % 8;
      //flush(data);
      //asm volatile("mfence");

      volatile int bit = (data[received_byte] >> received_bit) & 1;
      //asm volatile("":::"memory");
      if(bit)
      {
        access_array(received_byte,received_bit);
      }

      //respond
      sendto(sockfd,ret_val,strlen(ret_val),0,(struct sockaddr*) &cli_addr, slen);
    }
}

void* free_access(void *x)
{
   //init socket server
    struct sockaddr_in serv_addr,cli_addr; 

    int sockfd;
    socklen_t slen = sizeof(cli_addr) , recv_len;     
    //create a UDP socket
    if ((sockfd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
      printf("Error when creating socket\n");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(6000);

    //bind socket to port
    if(bind(sockfd,(struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
    {
      printf("error on binding...");
    }
    
    unsigned int received_num = 0;
    unsigned int received_byte = 0;
    unsigned int received_bit = 0;
    //just echo back for now

    //blocking on
    //int iMode = 0;
    //ioctl(listenfd, FIONBIO, &iMode);
    char ret_val[] = "KO";

    char leaked[80];
    memset(leaked,'0',sizeof(leaked));
    leaked[80] = 0;
    while(1)
    {
      //read from user
      //this should really block
      //receive byte
      recvfrom(sockfd,&received_byte,sizeof(received_byte), 0, (struct sockaddr *) &cli_addr, &slen);
      
      received_num = ntohl(received_byte);

      received_byte = received_num / 8;
      received_bit = received_num % 8;

      //printf("received_byte: %d %d",received_byte,received_bit);

      
      asm volatile("mfence");
      maccess(mem + (received_byte*8*4096 + received_bit * 4096));
      asm volatile("mfence");
      

      //Test locally to check whether spectre is working
      /*if(flush_reload(mem + (received_byte*8*4096 + received_bit * 4096))) {
                        printf("Hit: %d %d\n",received_byte,received_bit);
                        fflush(stdout);
                        sched_yield();
      }*/
      //respond
      sendto(sockfd,ret_val,strlen(ret_val),0,(struct sockaddr*) &cli_addr, slen);
    }
}


int main(int argc, char *argv[])
{

    pthread_t thread_1,thread_2,thread_3;


    //init mem
    char* _mem = malloc(4096*300); 
    mem = (char*)(((size_t)_mem & ~0xfff) + 0x1000*2 + 1024);
    memset(mem, 0, 4096 * 290);

    
    int j = 0;

    for(j = 0; j < 256; j++) 
    {
      flush(mem + j * 4096);
    }

    int t = pthread_create(&thread_1,NULL,restricted_access,NULL);
    t = pthread_create(&thread_2,NULL,free_access,NULL);
    t = pthread_create(&thread_3,NULL,do_flush, NULL);

    pthread_join(thread_1,NULL);
    pthread_join(thread_2,NULL);
    pthread_join(thread_3,NULL);

    return 0;
}