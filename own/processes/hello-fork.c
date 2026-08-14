#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(){
    pid_t pid = getpid();
    printf("Hallo, meine pid ist: %d \n", pid);
    
    pid_t child = fork();
    if(child < 0){
        printf("Nein! Etwas ist schiefgelaufen \n");
        exit(EXIT_FAILURE);
    }

    // Note that after the fork(), both processes start from the same IP. We don't have a guarantee about the execution order. Consider these problems as independent.
    if(child == 0){
        printf("Hallo! Dies ist ein Kindprozess, und ich habe die PID: %d und meine Dad ist PID: %d \n", getpid(), getppid());
    }else{
        printf("Hallo! Das ist Dad.\n");
    }
    return 0;
}