#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>

int main(int argc, char* argv[]) {
    printf("Process T launched\n");
    int semID = atoi(argv[1]);
    int memID = atoi(argv[2]);
    int pipeID = atoi(argv[3]);

    char* mem = (char*) shmat(memID, NULL, 0);

    char* buf = (char*) calloc(151, sizeof(char));
    for (int i = 0; i < 10; i++) {
        struct sembuf sem_op;
        sem_op.sem_num = 0;
        sem_op.sem_op = -1;   // Wait for value to become 1
        sem_op.sem_flg = 0;
        semop(semID, &sem_op, 1);
        int idx = 0;
        while (read(pipeID, &buf[idx], 1) > 0) {
            if (buf[idx] == '\n') {
                idx++;
                break;
            }
            idx++;
        }
        buf[idx - 1] = '\0';
        printf("T >> Read [%s]\n", buf);
        strcpy(mem, buf);
        // sleep(1);
    }

    free(buf);
    printf("T >> END\n");
    return 0;
}
