#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void custom_handler(int sig) {
    printf("[PARENT] SIGUSR1 recibida!\n");
}

int main() {
    signal(SIGUSR1, custom_handler);

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    pid_t child = fork();

    if (child == 0) {
        pid_t parent = getppid();

        for (int i = 0; i < 1000; i++) {
            kill(parent, SIGUSR1);
        }

        printf("[CHILD] Mandé 1000 SIGUSR1\n");
        exit(EXIT_SUCCESS);
    }

    sleep(1);

    /*
     * Create the temporary mask used while suspended.
     * SIGUSR1 must NOT be blocked in this mask.
     */
    sigset_t suspend_mask;
    sigemptyset(&suspend_mask);

    printf("[PARENT] Waiting for SIGUSR1 with sigsuspend()...\n");

    /*
     * Atomically:
     * 1. Replace the current mask with suspend_mask.
     * 2. Sleep waiting for a signal.
     *
     * Since SIGUSR1 is pending and is not blocked in suspend_mask,
     * it is delivered and the handler runs.
     */
    sigsuspend(&suspend_mask);

    printf("[PARENT] Salí de sigsuspend()\n");

    return 0;
}