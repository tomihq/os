#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

void execute_child_1(){
    while(1){

    }
}

void execute_child_2(){

}

int main() {
    pid_t child_1 = fork(); 
    if(child_1 == 0){
        execute_child_1();
    }
    //garantizar que el child_1 nunca llegue acá.

    pid_t child_2 = fork();
    if(child_2 == 0){
        execute_child_2();
    }

    wait(NULL);
    wait(NULL);

    exit(EXIT_SUCCESS);

}