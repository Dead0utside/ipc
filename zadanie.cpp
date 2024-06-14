#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <cstring>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>

using namespace std;

void reverse(char s[]);

int createProcess(char* argv[]);

void launchProcess(char* name, char* argv[]);

int pipes();

int semaphores(int fdStart, char* port);

void itoa(int n, char s[]);

void server(char* port1, char* port2);

int proc_s_pid, proc_p1_pid, proc_p2_pid;

int father_waiting;

void handle_USR1(int sig){
	// Just do nothing
	printf("FATHER GOT USR1\n");
}

void handle_USR2(int sig){
	// Escape USR2
	printf("FATHER GOT USR2\n");
    father_waiting = 0;
}

int main(int argc, char *argv[]) {
    signal(SIGUSR1, handle_USR1);
    signal(SIGUSR2, handle_USR2);
    father_waiting = 1;
    //PREPARATIONS
    if (argc < 3) {
        printf("Too few arguments!\n");
        return 1;
    }

    //PIPES
    int R2pipeID = pipes();
    // printf("Pipes done\n");
    sleep(5);
    //SERVER
    server(argv[1], argv[2]);
    sleep(1);
    //SEMAPHORES
    semaphores(R2pipeID, argv[1]);
    

    return 0;
}


void reverse(char s[]) {
    int i, j;
    char c;

    for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void itoa(int n, char s[]) {
    int i, sign;

    if ((sign = n) < 0)
        n = -n;
    i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

int createProcess(char* argv[]) {
    int pid = fork();
    if (pid == 0) {
        launchProcess(argv[0], argv);
    }
    else {
        // printf("Child process %s is created\n", argv[0]);
    }
    return pid;
}

void launchProcess(char* name, char* argv[]) {
    if (execve(name, argv, NULL) == -1) {
        printf("Failed to launch %s process;\n Errno: %d\n", name, errno);
        perror("");
        exit(1);
    }
}

int pipes() {
    int R1[2];
    int R2[2];
    if (pipe(R1) == -1 || pipe(R2) == -1) {
        return 1;
    }

    proc_p1_pid = fork();
    // printf("PID1 %d\n", proc_p1_pid);
    char *pMass_1 = (char *) calloc(sizeof(R1[1]) + 1, 8);
    itoa(R1[1], pMass_1);
    char *arr[] = {pMass_1, NULL};
    if (proc_p1_pid == 0) {
        execve("proc_p1", arr, NULL);
    }
    proc_p2_pid = fork();
    // printf("PID2 %d\n", proc_p2_pid);
    if (proc_p2_pid == 0) {
        execve("proc_p2", arr, NULL);
    }
    free(pMass_1);
    sleep(1);
    char *pMass_P1 = (char *) calloc(sizeof(R1[1]) + 1, 8);
    char *pMass_P2 = (char *) calloc(sizeof(R1[1]) + 1, 8);
    char *pMass_R10 = (char *) calloc(sizeof(R1[1]) + 1, 8);
    char *pMass_R21 = (char *) calloc(sizeof(R1[1]) + 1, 8);
    itoa(proc_p1_pid, pMass_P1);
    itoa(proc_p2_pid, pMass_P2);
    itoa(R1[0], pMass_R10);
    itoa(R2[1], pMass_R21);
    char name[] = "proc_pr";
    //printf("P1: %s\nP2: %s\n", pMass_P1, pMass_P2);
    char *arr_pr[] = {name, pMass_P1, pMass_P2, pMass_R10, pMass_R21, NULL};
    pid_t pid_PR = fork();
    if (pid_PR == 0) {
        printf("Launching pr\n");
        int a = execve("proc_pr", arr_pr, NULL);
        if (a == -1) {
            printf("Execve failed: %d\n", errno);
            perror("");
        }
    }
    free(pMass_P1);
    free(pMass_P2);
    free(pMass_R10);
    // free(pMass_R21);
    // wait(0);
    return R2[0];
}

int semaphores(int pipeID, char* port) {    
    char* pipeIDstr = (char*) calloc(10, sizeof(char));

    itoa(pipeID, pipeIDstr);

    key_t key1 = ftok("p1.txt", 1);
    key_t key2 = ftok("p2.txt", 1);

    int sem1 = semget(key1, 2, IPC_CREAT | 0644);
    int sem2 = semget(key2, 2, IPC_CREAT | 0644);

    int mem1 = shmget(IPC_PRIVATE, 150, 0644 | IPC_CREAT);
    int mem2 = shmget(IPC_PRIVATE, 150, 0644 | IPC_CREAT);
    
    if (sem1 < 0) {
        perror("Something is wrong with S1 id");
        return -1;
    }
    if (sem2 < 0) {
        perror("Something is wrong with S2 id");
        return -1;
    }

    //create argument strings
    char* sem1IDstr = (char*) calloc(10, sizeof(char));
    char* sem2IDstr = (char*) calloc(10, sizeof(char));
    char* mem1IDstr = (char*) calloc(10, sizeof(char));
    char* mem2IDstr = (char*) calloc(10, sizeof(char));
    
    itoa(sem1, sem1IDstr);
    itoa(sem2, sem2IDstr);
    itoa(mem1, mem1IDstr);
    itoa(mem2, mem2IDstr);

    char proc_t[] = "proc_t";
    char proc_d[] = "proc_d";
    char proc_s[] = "proc_s";

    char* argT[] = {proc_t, sem1IDstr, mem1IDstr, pipeIDstr, NULL};
    char* argD[] = {proc_d, sem2IDstr, mem2IDstr, port, NULL};
    char* argS[] = {proc_s, mem1IDstr, sem1IDstr, mem2IDstr, sem2IDstr, NULL};

    createProcess(argT);
    createProcess(argD);

    proc_s_pid = createProcess(argS);
    
    sleep(1);

    semctl(sem1, 0, SETVAL, 1); //run process T 
    semctl(sem2, 0, SETVAL, 1); //run process S


    int status = 0;
    while(father_waiting);
    // printf("Father going\n");

    kill(proc_p1_pid, SIGUSR2);
    kill(proc_p2_pid, SIGUSR2);
    kill(proc_s_pid, SIGTERM);

    semctl(sem1, 2, IPC_RMID);
    semctl(sem2, 2, IPC_RMID);
    shmctl(mem1, IPC_RMID, NULL);
    shmctl(mem2, IPC_RMID, NULL);

    free(pipeIDstr);
    free(sem1IDstr);
    free(sem2IDstr);
    free(mem1IDstr);
    free(mem2IDstr);
    
    printf("Zadanie END\n");
    return 0;
}

void server(char* port1, char* port2){
    // printf("-------Server part------\n");
    pid_t serv1_pid = fork();
    char name[] = "proc_serv1";
    char* args1[] = {name, port1, port2, NULL};
    if (serv1_pid == 0) {
        execve("proc_serv1", args1, NULL);
        printf("PROBLEM WITH SERV1");
    }

    pid_t serv2_pid = fork();
    char *args2[] = {port2, NULL};
    if (serv2_pid == 0) {
        execve("proc_serv2", args2, NULL);
        printf("PROBLEM WITH SERV2");
    }
    // printf("-------Server part ended------\n");
}
