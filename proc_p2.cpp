#include<stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

int global_fd_R1_W;//descriptor R1 Write
FILE *fp;

void my_handlerP2(int signum)
{
    if (signum == SIGUSR1)
    { 
      char pMass_BUF[100];
      fgets(pMass_BUF, sizeof(pMass_BUF), fp);
      printf("P2 >> %s\n",pMass_BUF);
      write(global_fd_R1_W,&pMass_BUF,strlen(pMass_BUF));
    }
}

void handle_USR2(int sig){
    exit(0);
}

int main(int argc,char* argv[]){
    fp = fopen("p2.txt","r"); 
    int fd_R1_W=atoi(argv[0]);
  global_fd_R1_W=fd_R1_W;
    signal(SIGUSR1, my_handlerP2);
    signal(SIGUSR2, handle_USR2);
    
    // printf("Received P2 %d \n", global_fd_R1_W);
      for(int i=1;;i++){    //Infinite loop
    sleep(1);  // Delay for 1 second
  }
    fclose(fp);  
    return 0;
}

