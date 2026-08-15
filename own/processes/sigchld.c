#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdint.h>

void custom_handler(int sig) {
    printf("[PARENT] Hello from SIGCHLD handler!  \n");
}

int main() {
    signal(SIGCHLD, custom_handler);

    pid_t child = fork(); 
    if(child < 0){
        printf("Something went wrong  \n");
    }
    
    if(child == 0){
        printf("[CHILD] Hello! \n");
        exit(EXIT_SUCCESS);
    }

    int status;

    wait(&status);

    printf("[PARENT] Ending \n");

    exit(EXIT_SUCCESS);
}