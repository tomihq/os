#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

volatile sig_atomic_t signal_received = 0;

void handler_sigusr1(int sig)
{
    signal_received = 1;
}

int main(void)
{
    signal(SIGUSR1, handler_sigusr1);

    sigset_t blocked_mask;
    sigemptyset(&blocked_mask);
    sigaddset(&blocked_mask, SIGUSR1);
 
    sigprocmask(SIG_BLOCK, &blocked_mask, NULL);


    sigset_t wait_mask;
    sigfillset(&wait_mask);
    sigdelset(&wait_mask, SIGUSR1);
    sigdelset(&wait_mask, SIGTERM);

    pid_t child = fork();

    if (child == -1) {
        exit(EXIT_FAILURE);
    }

    if (child == 0) {
        pid_t parent = getppid();
        while (1) {
            signal_received = 0;

            while (!signal_received) {
                sigsuspend(&wait_mask);
            }
            printf("[CHILD][PID: %d] PONG!\n", getpid());
            kill(parent, SIGUSR1);
        }
    }

    int continue_execution = 1;

    while(continue_execution){
        for (int i = 0; i < 3; i++) {

            printf("[PARENT][PID: %d] PING\n", getpid());
            kill(child, SIGUSR1);

            signal_received = 0;

            while (!signal_received) {
                sigsuspend(&wait_mask);
            }
        }

        char response;

        printf("Wanna end the execution? [s/n]: ");
        scanf(" %c", &response);

        if (response == 's') {
            continue_execution = 0;
        }
    }

    kill(child, SIGTERM);
    waitpid(child, NULL, 0);

    exit(EXIT_SUCCESS);
}