#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

int main(){
    pid_t pid = getpid();
    printf("[PARENT] Hello, my PID is: %d \n", pid);

    uint32_t bitrate = 1024;
    
    pid_t child = fork();
    if(child < 0){
        printf("[PARENT] Oops, something went wrong while creating the child \n");
        exit(EXIT_FAILURE);
    }

    // Note that after the fork(), both processes start from the same IP. We don't have a guarantee about the execution order. Consider these problems as independent.
    if(child == 0){
        printf("[CHILD] PID: %d und meine Parent ist PID: %d \n", getpid(), getppid());
        bitrate = 2048;
        printf("[CHILD] Bitrate set with the value: %d, its virtual address is: %p \n", bitrate, (void *)&bitrate);
    }else{
        bitrate = 3096;
        printf("[PARENT] Bitrate set with the value: %d, its virtual address is: %p \n", bitrate, (void *)&bitrate);
    }
    return 0;
}