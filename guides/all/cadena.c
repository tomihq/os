#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#define EXIT_SUCCESS 0 

/*  
    Replicar traza (10)
    Para exportar output: strace -q -f -o salida-cadena.txt ./cadena
*/

int main(){
    pid_t julieta = fork(); 
    if(julieta == 0){
        printf("Soy Julieta \n");
        sleep(1);
        pid_t jennifer = fork(); 
        if(jennifer == 0){
            printf("Soy Jeniffer \n");
            sleep(1); 
            exit(EXIT_SUCCESS); 
        }
        exit(EXIT_SUCCESS);
    }
    
    printf("Soy Juan \n");
    sleep(1);
    wait(NULL);
    
    pid_t jorge = fork(); 
    if(jorge == 0){
        printf("Soy Jorge \n");
        sleep(1);
        exit(EXIT_SUCCESS);
    }

    return 0; 
}