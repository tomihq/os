#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

void custom_handler(int signal_number) {
    write(STDOUT_FILENO, "custom handler\n", 15);
    signal(SIGINT, SIG_DFL);
    raise(SIGINT);
}

int main() {
    signal(SIGINT, custom_handler);

    while (1) {
        pause();
    }

    return 0;
}