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
    
    int descriptores[2]; //0: lectura, 1: escritura
    pipe(descriptores);

    pid_t child_1 = fork(); 
    
    if(child_1 == 0){
        //escribe
        //garantizo que el child_1 no tenga acceso al descriptor de lectura.
        close(descriptores[0]);
        execute_child_1(descriptores[1]);

        exit(EXIT_SUCCESS);
    }

    pid_t child_2 = fork();
    if(child_2 == 0){
        //lee
        //garantizo que el child_2 no tenga acceso al descriptor de escritura.
        close(descriptores[1]);
        execute_child_2(descriptores[0]);
        exit(EXIT_SUCCESS);
    }

    wait(NULL);
    wait(NULL);

    return 0; 
}