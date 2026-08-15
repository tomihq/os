#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

int main(){
    pid_t pid = getpid();
    printf("[PARENT] Hallo, meine pid ist: %d \n", pid);
    
    pid_t child = fork();
    if(child < 0){
        printf("[PARENT] Nein! Etwas ist schiefgelaufen \n");
        exit(EXIT_FAILURE);
    }

    // Note that after the fork(), both processes start from the same IP. We don't have a guarantee about the execution order. Consider these problems as independent.
    if(child == 0){
        printf("[CHILD] ich habe die PID: %d und meine Parent ist PID: %d \n", getpid(), getppid());
        exit(EXIT_SUCCESS);
    }

    sleep(5000000);
}