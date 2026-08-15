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


     /*
     * Experiment:
     * Block SIGUSR1 and let the child send it 1000 times.
     * Since standard signals are not queued, only one SIGUSR1 will remain
     * pending. When the parent unblocks it, the handler will run once.
     *
     * Then, the parent calls pause(). However, the signal it was interested
     * in has already been delivered, so pause() will block waiting for a
     * new signal.
     *
     * This demonstrates the race condition between unblocking a signal and
     * calling pause(), which can be avoided with sigsuspend().
     */
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

    printf("[PARENT] Desbloqueando SIGUSR1...\n");

    sigprocmask(SIG_UNBLOCK, &mask, NULL);


    printf("[PARENT] Ahora voy a hacer pause()...\n");

    pause(); 

    printf("[PARENT] Salí del pause()\n");

    return 0;
}