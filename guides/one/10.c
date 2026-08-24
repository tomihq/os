/*
Voy a explicar lo relevante que veo del trace. Ojo, el trace puede no ser exactamente igual, porque el scheduler puede tomar distintas decisiones. Sin embargo, el comportamiento general y las relaciones de orden que dependen del programa deberían mantenerse.

Lo primero que veo relevante es:

clone(child_stack=NULL, flags=CLONE_CHILD_CLEARTID|CLONE_CHILD_SETTID|SIGCHLDstrace: Proce, child_tidptr=0x2460b50) = 10552
[pid 10551] write(1, "Soy Juan\n\0", 10) = 10
[pid 10552] write(1, "Soy Julieta\n", 12 <unfinished ...>
[pid 10551] clock_nanosleep(CLOCK_REALTIME, 0, {tv_sec=1, tv_nsec=0}, <unfinished ...>
[pid 10552] <... write resumed>) = 12
[pid 10552] clock_nanosleep(CLOCK_REALTIME, 0, {tv_sec=1, tv_nsec=0}, <unfinished ...>
[pid 10551] <... clock_nanosleep resumed>0x7ffe6cd07ca0) = 0

1. Juan tiene un hijo.
2. Juan saluda.
3. Julieta saluda.
4. Juan se duerme por 1 segundo.
5. Julieta se duerme por 1 segundo.

Después, viene esto:

[pid 10551] <... clock_nanosleep resumed>0x7ffe6cd07ca0) = 0
[pid 10551] wait4(-1, <unfinished ...>

1. Juan se despierta.
2. Juan se pone en pausa esperando que Julieta finalice.

Después viene esto:

[pid 10552] <... clock_nanosleep resumed>0x7ffe6cd07ca0) = 0
[pid 10552] clone(child_stack=NULL, flags=CLONE_CHILD_CLEARTID|CLONE_CHILD_SETTID|SIGCHLDs, child_tidptr=0x2460b50) = 10557
[pid 10557] write(1, "Soy Jennifer\n\0", 14 <unfinished ...>

1. Julieta se despierta del sleep.
2. Julieta tiene una hija. Se llama Jennifer.
3. Jennifer saluda.

Luego viene esto:

[pid 10552] exit_group(0) = ?
[pid 10557] clock_nanosleep(CLOCK_REALTIME, 0, {tv_sec=1, tv_nsec=0}, <unfinished ...>
[pid 10552] +++ exited with 0 +++

Eso quiere decir que después de que nació Jennifer:

1. Julieta manda exit().
2. Jennifer se duerme por 1 segundo.
3. Julieta termina.

Luego viene esto:

[pid 10551] <... wait4 resumed>[{WIFEXITED(s) && WEXITSTATUS(s) == 0}], 0, NULL) = 10552
[pid 10551] --- SIGCHLD {si_signo=SIGCHLD, si_code=CLD_EXITED, si_pid=10552, si_uid=1000,
[pid 10551] clone(child_stack=NULL, flags=CLONE_CHILD_CLEARTID|CLONE_CHILD_SETTID|SIGCHLDstrace: Process 10558 attached
[pid 10551] exit_group(0) = ?
[pid 10558] write(1, "Soy Jorge\n", 10) = 10
[pid 10558] clock_nanosleep(CLOCK_REALTIME, 0, {tv_sec=1, tv_nsec=0}, <unfinished ...>

1. Juan continúa su ejecución.
2. Juan recibe que su hija, Julieta, terminó (relacionado al wait de antes).
3. Juan tiene un nuevo hijo.
4. Juan manda exit().
5. Jorge nace e imprime el write.
6. Jorge se duerme por 1 segundo.

Luego viene esto:

[pid 10551] +++ exited with 0 +++
[pid 10557] <... clock_nanosleep resumed>0x7ffe6cd07ca0) = 0
[pid 10557] exit_group(0) = ?
[pid 10558] <... clock_nanosleep resumed>0x7ffe6cd07ca0) = 0
[pid 10558] exit_group(0) = ?
[pid 10557] +++ exited with 0 +++
+++ exited with 0 +++

1. Juan termina.
2. Jennifer se despierta del sleep.
3. Jennifer manda exit().
4. Jorge se despierta del sleep.
5. Jorge manda exit().
6. Jennifer termina.
7. Jorge termina.

exit_group: solicita terminar el programa.
exited: avisa que ya terminó.

Ejecuto mi código con:

strace -q -f ./archivo
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