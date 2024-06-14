#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 

int main(int argc, char* argv[]) {
    // printf("Process D launched\n");
    int semID = atoi(argv[1]);
    int memID = atoi(argv[2]);
    int port = atoi(argv[3]);
    char* mem = (char*) shmat(memID, NULL, 0);

// ==
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){ 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 

    struct sockaddr_in serv_addr; 
    serv_addr.sin_family = AF_INET; // IPv4
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    uint16_t converted_port = htons(port); // Convert port to network byte order
    serv_addr.sin_port = converted_port; // Assign port
    
    int connect_result = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if(connect_result < 0){ 
        perror("\nConnection Failed\n");
        printf("Port: %d\n", port);
        return -1; 
    } 
// ==
    char* buf = (char*) calloc(151, sizeof(char));
    for (int i = 0; i < 10; i++) {
        struct sembuf sem_op;
        sem_op.sem_num = 1;
        sem_op.sem_op = -1;   // Wait for value to become 1
        sem_op.sem_flg = 0;
        semop(semID, &sem_op, 1);
        // sleep(2);
        strcpy(buf, mem);
        size_t size = strlen(buf);
        buf[size] = '\0';
        printf("D >> got [%s]\n", buf);
        send(sock, buf, size + 1, 0);
    }

    free(buf);

    kill(getppid(), SIGUSR2);

    return 0;
}
