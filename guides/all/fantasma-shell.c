#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#define EXIT_SUCCESS 0
/*
    ls -al | wc -l
    
    Necesito 1 pipe.
    ls -al stdout a pipe.
    wc -l stdin es el read del pipe.

*/
int main(){
    int pipes[2];
    pipe(pipes);

    pid_t cmd1 = fork();
    if(cmd1 == 0){
        //Cerramos el extremo de lectura del pipe. No lo va a usar. Si lo dejamos abierto, el programa no terminaría.
        close(pipes[0]);
        
        //El STDOUT del primer comando va a ser al extremo de escritura del pipe por el cual el siguiente comando va a escuchar.
        dup2(pipes[1], STDOUT_FILENO);
        
        //Cerramos el extremo de escritura porque el proceso ya tiene a STDOUT_FILENO apuntando a ese fd. Si no cerraramos este, entonces el que use pipes[0] para escuchar, estaría pendiente de que en algún momento pipes[1] le mande EOF (todos cierran el pipes[1])
        close(pipes[1]);

        //Ejecuta el comando. Cuando el programa termina, hace exit() y libera la referencia a pipes[1]. Por lo que elimina la referencia a ese pipe. 
        execl("/bin/sh", "sh", "-c", "ls -al", (char *) NULL);
        
        //no deberÍa llegar aca
        exit(EXIT_SUCCESS);
    }

    pid_t cmd2 = fork(); 
    if(cmd2 == 0){
        //No va a usar el extremo de escritura del pipe.
        close(pipes[1]);

        //Queremos que el stdin de este comando sea lo que escriban en el pipe. STDIN_FILENO ahora apunta al extremo de lectura de pipes.
        dup2(pipes[0], STDIN_FILENO);
        
        //Cerramos inmediatamente pipes[0] porque STDIN_FILENO ya tiene la referencia. pipes[0] no lo usamos mas. Si no hacemos esto, se colgaria porque habria alguna referencia sin cerrar.
        close(pipes[0]);
        
        //Ejecutamos el comando. Su entrada va a ser a traves del extremo read del pipe. No se va a ejecutar hasta que alguien escriba ahi porque wc -l espera stdin con los archivos del directorio.
        execl("/bin/sh", "sh", "-c", "wc -l", (char * ) NULL);
        exit(EXIT_SUCCESS);
    } 

    //Primero cierro pipes. Si hago "wait" antes, entonces el resto de programas NO podrían finalizar porque observan que alguien todavía tiene los pipes.
    //Notar que siempre el programa que ESCUCHA EL PIPE es el que no puede terminar si hay alguno que todavía tiene el canal de WRITE abierto porque el programa podría decir "alguien no me mandó el EOF"
    close(pipes[0]);
    close(pipes[1]);

    for(int i = 0; i<2 ; i++){
        wait(NULL);
    }

    
    return 0;
}