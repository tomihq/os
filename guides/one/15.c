
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void execute_child_1(int descriptor_escritura) {
    //Configuramos que el STDOUT apunte al mismo archivo/recurso al que apunta el descriptor_escritura. 
    dup2(descriptor_escritura, STDOUT_FILENO);

    close(descriptor_escritura);
    execl("/bin/sh", "sh", "-c", "ls -al", (char *)NULL); //si esto funciona NUNCA debería pasar a lo de abajo.
    
    perror("execl child 1");
}

void execute_child_2(int descriptor_lectura) {
    dup2(descriptor_lectura, STDIN_FILENO);
    close(descriptor_lectura);
    execl("/bin/sh", "sh", "-c", "wc -l", (char *)NULL); //si esto funciona NUNCA debería pasar a lo de abajo.

    perror("execl child 2");
}

int main() {

    int descriptores[2]; // [0]: lectura, [1]: escritura

    if (pipe(descriptores) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t child_1 = fork();

    if (child_1 < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child_1 == 0) {
        // Hijo 1 solo escribe. Cierro descriptor de lectura (ayuda a garantizar EOF)
        close(descriptores[0]);
        execute_child_1(descriptores[1]);
        exit(EXIT_FAILURE);
    }

    pid_t child_2 = fork();

    if (child_2 < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child_2 == 0) {
        // Hijo 2 solo lee. 
        close(descriptores[1]);
        execute_child_2(descriptores[0]);
        exit(EXIT_FAILURE);
    }

    close(descriptores[0]);
    close(descriptores[1]);

    waitpid(child_1, NULL, 0);
    waitpid(child_2, NULL, 0);

    return 0;
}