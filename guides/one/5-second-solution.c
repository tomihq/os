#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int main()
{
    printf("[ABRAHAM][PID: %d] Hola! \n", getpid());

    // Homero. A partir de acá asumir que tengo dos ejecuciones del programa. Cuidado con los ifs.
    pid_t homer_pid = fork();
    if (homer_pid < 0)
    {
        printf("[ABRAHAM] Oops, something went wrong");
        exit(EXIT_FAILURE);
    }

    // Si entra al if estoy solo en contexto de homero.
    if (homer_pid == 0)
    {
        printf("[HOMERO][PID: %d][PPID: %d] douh!\n",
               getpid(), getppid());

        pid_t bart_pid = fork();

        if (bart_pid == 0)
        {
            printf("[BART][PID: %d][PPID: %d] que hay de nuevo viejo\n",
                   getpid(), getppid());
            exit(EXIT_SUCCESS);
        }

        pid_t lisa_pid = fork();

        if (lisa_pid == 0)
        {
            printf("[LISA][PID: %d][PPID: %d] todo el maldito sistema está mal!\n",
                   getpid(), getppid());
            exit(EXIT_SUCCESS);
        }

        pid_t maggie_pid = fork();

        if (maggie_pid == 0)
        {
            printf("[MAGGIE][PID: %d][PPID: %d] chup-chup\n",
                   getpid(), getppid());
            exit(EXIT_SUCCESS);
        }
    }

    // Acá las variables de pid_child_bart, ... no viven en el ctx de [ABRAHAM], solo en el contexto de [HOMERO]
    sleep(1000);

    exit(EXIT_SUCCESS);
}