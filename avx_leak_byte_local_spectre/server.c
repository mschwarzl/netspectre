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

#define ARR_SIZE 1024

char* data = "dataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRETdataSECRET";


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

char access_array(int x,int bit,int isBitSet) 
{
    //maccess(data);
    //maccess(data + 8); // data is cached, x is cached anyways
    //flush(&x);
    //flush(&bit);
    //flush(data);

    __m256i a; __m256i b;

    // x < 4 :)
    //if(x < strlen(data))// - strlen("SECRET") - 1990) 
    //{
	if(isBitSet)
	{
	  asm volatile("mfence");
	  b = _mm256_and_si256(a, b);
	  asm volatile("mfence");
	}
  return 0;
    //}
}

#define BUF_LEN 4

void* do_access(void *x) {
  
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
  int slen = sizeof(serv_addr);

  a = _mm256_or_si256(a, b);
  char ret_val[] = "KO";

  unsigned int received_num = 0;
  unsigned int received_byte = 0;
  unsigned int received_bit = 0;

  while (1) 
  {
      recvfrom(listenfd,&received_byte,sizeof(received_byte), 0, (struct sockaddr *) &serv_addr, &slen);
      received_num = ntohl(received_byte);
      sendto(listenfd,ret_val,strlen(ret_val),0,(struct sockaddr*) &serv_addr, slen);


      if(received_num != 0)
      {
        recvfrom(listenfd, buf, BUF_LEN, 0, (struct sockaddr *) &serv_addr, &slen);
        asm volatile("mfence");
        b = _mm256_and_si256(a, b);
        asm volatile("mfence");
        sendto(listenfd, buf, BUF_LEN, 0, (struct sockaddr*) &serv_addr, slen);
      }
      else
      {
        recvfrom(listenfd,&received_num,sizeof(received_num), 0, (struct sockaddr *) &serv_addr, &slen);
      	received_byte = ntohl(received_num) / 8;
        received_bit = ntohl(received_num) % 8;
        //flush(data);
        //asm volatile("mfence");
        volatile int bit = (data[received_byte] >> received_bit) & 1;
        //asm volatile("":::"memory");
        //access_array(received_byte,received_bit,bit);
        flush(&received_byte);
        flush(data);
        
        if(bit)
        {
           if(received_byte < strlen(data) - strlen("SECRET") - 1990)
           {
              //asm volatile("mfence");
              //fix using multiple lfences
              //asm volatile("lfence");
              b = _mm256_and_si256(a, b);
              //asm volatile("mfence");
            }
        }
    	//printf("Hit: %d %d %d\n",received_byte,received_bit,bit);
    	//respond
    	sendto(listenfd,ret_val,strlen(ret_val),0,(struct sockaddr*) &serv_addr, slen);
      }

    // read from user
    
    //asm volatile("cpuid"::"a"(0),"c"(0):"memory");
  }
}

int main(int argc, char *argv[])
{

    pthread_t thread_1,thread_2,thread_3;

    //int t = pthread_create(&thread_1,NULL,restricted_access,NULL);
    int t = pthread_create(&thread_2,NULL,do_access,NULL);
    //t = pthread_create(&thread_3,NULL,do_flush, NULL);

    //pthread_join(thread_1,NULL);
    pthread_join(thread_2,NULL);
    //pthread_join(thread_3,NULL);

    return 0;
}
