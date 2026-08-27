#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void execute_child(int descriptor_escritura, int descriptor_lectura) {
    int valor;

    while (1) {
        read(descriptor_lectura, &valor, sizeof(valor));
        valor++;
    
        printf("[PONG] Valor: %d \n", valor);
        fflush(stdout);
        write(descriptor_escritura, &valor, sizeof(valor));
    }
}

void execute_parent(int descriptor_escritura, int descriptor_lectura, pid_t child) {
    int valor = 0;

    write(descriptor_escritura, &valor, sizeof(valor));

    while (1) {
        read(descriptor_lectura, &valor, sizeof(valor));

        valor++;
        printf("[PING] Valor: %d \n", valor);

        if (valor >= 50) {
            kill(child, SIGKILL);
            break;
        }

        fflush(stdout);
        write(descriptor_escritura, &valor, sizeof(valor));
    }
}

int main() {
    int padre_hijo[2]; // [0]: lectura, [1]: escritura
    int hijo_padre[2]; // [0]: lectura, [1]: escritura

    if (pipe(padre_hijo) == -1 || pipe(hijo_padre) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t child = fork();

    if (child < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child == 0) {
        //Cierro descriptores que no debería tocar.
        close(padre_hijo[1]);
        close(hijo_padre[0]);

        execute_child(hijo_padre[1], padre_hijo[0]);

        //Cierro descriptores que usó. 
        close(padre_hijo[0]);
        close(hijo_padre[1]);

        exit(EXIT_SUCCESS);
    }

    //Cierro los descriptores que no va a usar el padre.
    close(padre_hijo[0]);
    close(hijo_padre[1]);

    execute_parent(padre_hijo[1], hijo_padre[0], child);

    //Cierro los descriptores que usó el padre. 
    close(padre_hijo[1]);
    close(hijo_padre[0]);   
    
    //Libero los recursos del hijo 
    waitpid(child, NULL, 0);

    return 0;
}
