#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

// malloc() and free() are not async-signal-safe.
// If SIGINT interrupts the main process while malloc() or free() is manipulating
// the heap, the handler may call them again while their internal state or locks
// are in use. This can cause undefined behavior, such as a deadlock.
void custom_handler(int sig) {
    void *p = malloc(100);
    free(p);
}

int main() {
    signal(SIGINT, custom_handler);

    while(1){
        void *p = malloc(100000);
        free(p);
    }
    return 0;
}