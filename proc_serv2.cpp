#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
  
int main(int argc, char* argv[]){
    // printf("Process SERV2 launched\n");
    int port = atoi(argv[0]);

    char buffer[152]; 

    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr; 

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0 ) { 
        perror("Socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 

    // Fill sockaddr structs with 0
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 


    // Configuration
    servaddr.sin_family = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; // Any address 
    uint16_t converted_port = htons(port); // Convert port to network byte order
    servaddr.sin_port = converted_port; // Assign converted port

    int bind_result = bind(sockfd, (const struct sockaddr *)&servaddr,  sizeof(servaddr)); // Assing the address to the socket
    if(bind_result < 0){ 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    
    socklen_t len = sizeof(cliaddr);
    int outfile = open("serv2.txt", O_WRONLY | O_CREAT, 0666);
    for(int i = 0; i < 10; i++){
        int message_size = recvfrom(sockfd, (char *)buffer, 152, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len); 
        printf("SERV2: [%s]\n", buffer);
        write(outfile, buffer, message_size - 1);
        write(outfile, "\n", 1);
    }
    // kill(getppid(), SIGUSR2);
}