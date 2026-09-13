#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>

int N;
int i;

sem_t *sem;

void *proceso(void *arg) {
    int j = *(int *)arg; 

    sem_wait(&sem[j]);

    printf("Soy el proceso %d\n", j);
    fflush(stdout);

    int siguiente = (j + 1) % N;
    sem_post(&sem[siguiente]);

    return NULL;
}

int main() {
    N = 5;
    i = 2;

    pthread_t threads[N];
    int indices[N];

    sem = malloc(N * sizeof(sem_t));

    for (int j = 0; j < N; j++) {
        sem_init(&sem[j], 0, j == i ? 1 : 0);
        indices[j] = j;
    }

    for (int j = 0; j < N; j++) {
        pthread_create(&threads[j], NULL, proceso, &indices[j]);
    }

    for (int j = 0; j < N; j++) {
        pthread_join(threads[j], NULL);
    }

    for (int j = 0; j < N; j++) {
        sem_destroy(&sem[j]);
    }

    free(sem);

    return 0;
}