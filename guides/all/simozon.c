#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#define EXIT_SUCCESS 0

// Código Lester y Eliza
void tirar_dado(int pipe[2]){
    srand(time(NULL) ^ getpid());   
    int n = rand() % 5;
    write(pipe[1], &n, sizeof(int));
}

// Homero
int main(){
    int pipes[2][2]; //Lester - Eliza. 
    pipe(pipes[0]); //Inicializa a Lester. Cada extremo es un FD diferente.
    pipe(pipes[1]); //Inicializa a Eliza. Cada extremo es un FD diferente.
    pid_t lester = fork();
    if(lester == 0){
        close(pipes[0][0]); //no usa el Read.
        
        // Cierro Pipes Eliza 
        for(int i = 0; i<2; i++){
            close(pipes[1][i]);
        }

        tirar_dado(pipes[0]);
        close(pipes[0][1]); //cierro el Write porque ya no lo uso más.
        exit(EXIT_SUCCESS);
    }
    pid_t eliza = fork();  
    if(eliza == 0){
        close(pipes[1][0]); //no usa el Read.
        
        //Cierro Pipes Lester.
        for(int i = 0; i<2; i++){
            close(pipes[0][i]);
        }
        
        tirar_dado(pipes[1]); 
        close(pipes[1][1]); //cierro el Write porque ya no lo uso más. 
        exit(EXIT_SUCCESS);
    } 

    //Acá asumimos que los dos terminan en algun momento y el dato se da "al mismo tiempo". Si nos ponemos quisquillosos del orden, tendríamos que escuchar "ambos a la vez". 
    int numero_lester = 0;
    read(pipes[0][0], &numero_lester, sizeof(int));

    int numero_eliza = 0; 
    read(pipes[1][0], &numero_eliza, sizeof(int));

    if(numero_lester > numero_eliza){
        printf("Ganó Lester");
        fflush(stdout);
    }else if(numero_eliza > numero_lester){
        printf("Ganó Eliza");
        fflush(stdout);
    }else{
        printf("Empataron");
        fflush(stdout);
    }
    
    for(int i = 0; i<2; i++){
        wait(NULL);
    }

    for(int i = 0; i<2; i++){
        close(pipes[i][i]);
    }

    return 0; 
}