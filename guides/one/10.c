/*
Voy a explicar lo relevante que veo del trace. Ojo, no tengo la certeza de que las cosas estaban en el código con ese orden.

Lo primero que veo relevante es
clone(child_stack=NULL, flags=CLONE_CHILD_CLEARTID|CLONE_CHILD_SETTID|SIGCHLDstrace: Proce, child_tidptr=0x2460b50) = 10552
[pid 10551] write(1, "Soy Juan\n\0", 10) = 10
[pid 10552] write(1, "Soy Julieta\n", 12 <unfinished ...>
[pid 10551] clock_nanosleep(CLOCK_REALTIME, 0, {tv_sec=1, tv_nsec=0}, <unfinished ...>
[pid 10552] clock_nanosleep(CLOCK_REALTIME, 0, {tv_sec=1, tv_nsec=0}, <unfinished ...>

Esto quiere decir que, porque el primer clone no tiene [pid] asumo que es el padre (Juan) que crea un hijo llamada Julieta.
Notar que el clone está antes del print
Asi que seguro sea 

fork()
imprimir soy juan
imprimir soy julieta

Luego, Juan se pone a dormir por 1 segundo. Luego Julieta hace lo mismo.

Después, viene esto
[pid 10551] wait4(-1, <unfinished ...> Juan empieza a escuchar que Julieta termine.

Después viene esto
[pid 10552] clone(child_stack=NULL, flags=CLONE_CHILD_CLEARTID|CLONE_CHILD_SETTID|SIGCHLDs, child_tidptr=0x2460b50) = 10557
[pid 10557] write(1, "Soy Jennifer\n\0", 14 <unfinished ...>

Eso nos dice que Julieta hizo un fork(), es decir, tuvo una hija y se llama Jennifer.

Luego viene esto
[pid 10552] exit_group(0) = ?
[pid 10557] clock_nanosleep(CLOCK_REALTIME, 0, {tv_sec=1, tv_nsec=0}, <unfinished ...>
[pid 10552] +++ exited with 0 +++
Eso quiere decir que después de que nació Jennifer:
1. Julieta manda exit.
2. Jennifer se duerme por 1 segundo.
3. Julieta termina

Luego viene esto
[pid 10551] <... wait4 resumed>[{WIFEXITED(s) && WEXITSTATUS(s) == 0}], 0, NULL) = 10552
[pid 10551] clone(child_stack=NULL, flags=CLONE_CHILD_CLEARTID|CLONE_CHILD_SETTID|SIGCHLDstrace: Process 10558 attached
[pid 10551] exit_group(0) = ?
[pid 10558] write(1, "Soy Jorge\n", 10) = 10
[pid 10558] clock_nanosleep(CLOCK_REALTIME, 0, {tv_sec=1, tv_nsec=0}, <unfinished ...>

1. Como Julieta (hija de Juan) terminó, ahora Juan tiene un nuevo hijo.
2. Juan manda exit().
3. Nace Jorge, imprime el write y se duerme 1s.

Luego viene esto
[pid 10557] exit_group(0) = ?
Eso nos quiere decir que Jennifer termina su proceso.

Luego viene esto
[pid 10558] exit_group(0) = ?
Eso nos quiere decir que Jorge termina su proceso.

Por último, termina el proceso padre.
[pid 10557] +++ exited with 0 +++

exit_group: solicita terminar el programa.
exited: avisa que ya terminó

Ejecuto mi código con: stract -q -f ./archivo 


*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

int main(){
    pid_t julieta_pid = fork();

    if(julieta_pid == 0){
        printf("Soy Julieta \n");
        sleep(1);
        pid_t jennifer_pid = fork();
        if(jennifer_pid == 0){
            printf("Soy Jennifer \n");
            sleep(1);
            exit(EXIT_SUCCESS);
        }
        exit(EXIT_SUCCESS);
    }else{
        printf("Soy Juan \n");
        sleep(1);
        wait(NULL);
    }

    pid_t jorge_pid = fork();
    if(jorge_pid == 0){
        printf("Soy Jorge \n");
        sleep(1);
        exit(EXIT_SUCCESS);
    }
    
    exit(EXIT_SUCCESS);
}