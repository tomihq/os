#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#define EXIT_SUCCESS 0 

int pipeW[2]; //3, 4
pid_t parent, child = 0; 

/*  
    Replicar traza (18)
    Para exportar output: strace -q -f -o significado-vida.txt ./significado-vida
    

     --- SIGINT {si_signo=SIGINT, si_code=SI_USER, si_pid=6590} 
     codigo handler
      
     rt_sigreturn()
     termina handler

     Análisis del código:
     - A la traza mostrada le faltan los pause. ¿Por qué? Porque el padre le manda un mensaje al hijo, y el mensaje "se levanta". Sin un pause() el hijo funcionaría y terminaría de corrido.
    Un ejemplo claro de esto es cuando el padre envía: "¿Cuál es el significado de la vida?", si el padre no hiciera "pause" entonces seguiría el código su flujo normal. Ese mensaje se escribe directo a STDOUT por lo que el mensaje aparece consumido "casi inmediatamente", si fuese que está escribiendo al extremo de escritura del Pipe tendría sentido que se bloquee hasta que el hijo no lo consuma, pero eso no sucede. Por lo que confirmamos que hay un pause que no sale en el trace. En particular, cuando el hijo escribe al pipe, envía kill para despertar al padre. 
     - el padre termina su trabajo dentro del handler. No hay un rt_sigreturn() cuando se ejecuta SIGINT del padre.
     - el hijo hace prácticamente todo el trabajo en cada handler. Fuera del handler necesitamos dos pause: una por cada señal que va a recibir.
     - La función sleep(1) de la libc hace 4 syscalls:
        1. rt_sigprocmask(SIG_BLOCK, [CHLD], ..)
        2. rt_sigaction(SIGCHLD, NULL, ...)
        3. rt_sigprocmask(SIG_SETMASK, ...)
        4. nanosleep({time, 0}, ...)


    */
void handler(int sig){
    char buf[3] = {0};
    read(pipeW[0], buf, 3);                                   

    write(STDOUT_FILENO, "Mirá vos. El significado de la vida es 42\n", 44);
    write(STDOUT_FILENO, "¡Bang Bang, estás liquidado!\n", 31);
    kill(child, SIGHUP);
    sleep(10);                                                
    write(STDOUT_FILENO, "Te voy a buscar en la oscuridad.\n", 33);
    close(pipeW[0]);
    exit(EXIT_SUCCESS);
}

void handler2(int sig){
    write(STDOUT_FILENO, "Dejame pensarlo...\n", 19);
    sleep(5);
    write(STDOUT_FILENO, "Ya sé el significado de la vida...\n", 34);
    write(pipeW[1], "42", 2);
    kill(parent, SIGINT);
}

void handler3(int sig){
    write(STDOUT_FILENO, "Me voy a mirar crecer las flores...\n", 46);
    close(pipeW[1]);
    exit(EXIT_SUCCESS);
}

int main(){
    pipe(pipeW);
    child = fork(); 
    if(child == 0){
        close(pipeW[0]);
        parent = getppid();
        signal(SIGINT, handler2);
        signal(SIGHUP, handler3);
        pause();
        pause();
        
    }
    close(pipeW[1]);
    signal(SIGINT, handler);
    signal(SIGCHLD, NULL);
    sleep(1);
    printf("¿Cuál es el significado de la vida? \n");
    kill(child, SIGINT);
    pause(); 
    return EXIT_SUCCESS;
}