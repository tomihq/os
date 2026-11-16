/*
Queremos 2 procesos que tomen trabajos de una cola de trabajo y los procesen.
Tenemos N procesos que generan trabajos y los agregan a la cola de trabajo
*/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define PRODUCERS 5
#define CONSUMERS 2
#define EXIT_SUCCESS 0

void consumer_code(int pipe){
    int val = 0; 
    int done_jobs = 0; 
    while((read(pipe, &val, sizeof(int))>0)){
        printf("[CONSUMER %d] Job %d processed \n", getpid(), val);
        done_jobs++;
    }
      printf("[CONSUMER %d] TOTAL Jobs Processed %d \n", getpid(), done_jobs);
    return; 
}

void producer_code(int from_job_id, int pipe, int jobs_amount){
    for(int i = from_job_id; i<jobs_amount; i++){
        printf("[PRODUCER %d] Job %d created \n", getpid(), i);
        //Simula que van llegando de a poco
        sleep(1);
        write(pipe, &i, sizeof(int));
    }
    return; 
}

int main(){
    int pipes[2];
    int JOBS_AMOUNT = 100; 
    int JOBS_PER_PRODUCER = JOBS_AMOUNT / PRODUCERS;
    pid_t producers[PRODUCERS];
    pid_t consumers[CONSUMERS];
    pipe(pipes);

    for(int i = 0; i<PRODUCERS; i++){
        pid_t producer = fork(); 
        if(producer == 0){
            close(pipes[0]);
            producer_code(i * JOBS_PER_PRODUCER, pipes[1], JOBS_PER_PRODUCER);
            close(pipes[1]);
            exit(EXIT_SUCCESS);
        }
        producers[i] = producer;
    }

    for(int i = 0; i<CONSUMERS; i++){
        pid_t consumer = fork(); 
        if(consumer == 0){
            close(pipes[1]);
            consumer_code(pipes[0]);
            close(pipes[0]);
            exit(EXIT_SUCCESS);
        }
        consumers[i] = consumer;
    }
    
    close(pipes[0]);
    close(pipes[1]);
    for(int i = 0; i<PRODUCERS; i++){
        waitpid(producers[i], NULL, 0);
    }

    for(int i = 0; i<CONSUMERS; i++){
        waitpid(consumers[i], NULL, 0);
    }
    return EXIT_SUCCESS;
}