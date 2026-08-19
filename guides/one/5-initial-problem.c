   #include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int main()
{
    printf("[ABRAHAM][PID: %d] Hola! \n", getpid());

    // Homero. A partir de acá asumir que tengo dos ejecuciones del programa. Cuidado con los ifs.
    pid_t pid_child = fork();
    if (pid_child < 0)
    {
        printf("[ABRAHAM] Oops, something went wrong");
        exit(EXIT_FAILURE);
    }

    // Si entra al if estoy solo en contexto de homero.
    if (pid_child == 0)
    {
        printf("[HOMERO][PID: %d][PPID: %d] douh! \n", getpid(), getppid());
        // la idea de hacer 3 variables distintas es para que vivan los hijos a la vez. Seguro haya alguna condición de carrera entre estos fork() xq seguro son async. 
        pid_t pid_child_bart = fork();
        if (pid_child_bart == 0)
        {
            printf("[BART][PID: %d][PPID: %d] que hay de nuevo viejo \n", getpid(), getppid());
        }
        pid_t pid_child_lisa = fork();
        if (pid_child_lisa == 0){
            printf("[LISA][PID: %d][PPID: %d] todo el maldito sistema está mal! \n", getpid(), getppid());
        }
        pid_t pid_child_maggie = fork();
        if (pid_child_maggie == 0)
        {
            printf("[MAGGIE][PID: %d][PPID: %d] chup-chup \n", getpid(), getppid());
        }
    }

    // Acá las variables de pid_child_bart, ... no viven en el ctx de [ABRAHAM], solo en el contexto de [HOMERO]

    exit(EXIT_SUCCESS);
}