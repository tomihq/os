#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

// Note that standard signals are not queued. If you send SIGINT one million
// times while its handler is running, at most one SIGINT remains pending.
// The remaining signals are lost. The pending SIGINT can be delivered once
// the current handler finishes and SIGINT becomes unblocked again.
void custom_handler(int sig) {
    while (true) {

    }
}

int main() {
    signal(SIGINT, custom_handler);
}