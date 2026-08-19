/*
    ¿Qué hace system(const char *command)? (man system)

1. Firma:
int system(const char *command);

2. Comportamiento:
    system() ejecuta un comando utilizando una shell y espera a que dicha ejecución termine antes de retornar al programa que llamó a system().

    Conceptualmente, su funcionamiento es:
    * Crea un proceso hijo.
    * El proceso hijo reemplaza el programa que está ejecutando utilizando exec():
    execl("/bin/sh", "sh", "-c", command, (char *) NULL);
    * Si exec() tiene éxito, no retorna. El proceso hijo deja de ejecutar el programa original y pasa a ejecutar /bin/sh.
    * Mientras tanto, el proceso padre espera a que el hijo termine.
    * Cuando el hijo termina, system() retorna y el proceso original puede continuar su ejecución.

Esquemáticamente:

```
Programa original
      |
   system()
      |
    fork()
   /      \
padre     hijo
  |         |
  |       exec()
  |         |
  |    /bin/sh -c command
  |         |
espera    termina
  |         |
  \---------/
      |
system() retorna
      |
programa original continúa
```

La diferencia fundamental con utilizar exec() directamente es que exec() reemplazaría el programa que está ejecutando el proceso actual. En ese caso, si exec() tiene éxito, nunca se podría retornar desde system() al main() original.

Por eso se necesita crear primero un proceso hijo: el hijo puede ser reemplazado mediante exec(), mientras que el padre conserva la ejecución del programa original y espera su terminación.

3. Señales:

Mientras system() espera la ejecución del comando:

* SIGCHLD se bloquea en el proceso que llamó a system().
* SIGINT y SIGQUIT son ignoradas por el proceso que llamó a system().
* El proceso hijo configura las señales según corresponda antes de ejecutar la shell.

4. Valores de retorno:
* Si command == NULL:
  * Devuelve un valor distinto de 0 si hay una shell disponible.
  * Devuelve 0 si no hay una shell disponible.

* Si no se pudo crear el proceso hijo:
  * Devuelve -1.
  * errno queda seteado.
  * En este caso, el error puede provenir de fork().

* Si no se pudo obtener el estado de terminación del proceso hijo:
  * Devuelve -1.
  * errno queda seteado.
  * En este caso, el error puede provenir de wait()/waitpid().

* Si el proceso hijo fue creado correctamente pero no pudo ejecutar la shell:
  * El resultado es equivalente a que el hijo hubiese terminado utilizando _exit(127).

* Si todo funciona correctamente:
  * system() devuelve el estado de terminación de la shell hija utilizada para ejecutar command.

Resumen conceptual:

```
system()
   |
   +-- fork()
          |
          +-- padre -> wait()/waitpid() -> return status
          |
          +-- hijo  -> exec("/bin/sh", ...)
                          |
                          +-- éxito -> ejecuta shell
                          |
                          +-- error -> _exit(127)
```

*/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>


int system(const char *command)
{
    pid_t child = fork(); 
    if(child < 0){
        //el fork() ya setea errno
        printf("[ERROR] fork failed: %d\n", errno);
        return -1;
    }

    if(child == 0){
        execl("/bin/sh", "sh", "-c", command, (char *)NULL);
        //response = execl("/ruta/que/no/existe", "sh", "-c", command, (char *) NULL);
        printf("[ERROR] exec failed. errno: %d\n", errno);
        _exit(127);
      
    }

    int status;
    
    if (waitpid(child, &status, 0) == -1) {
        printf("[ERROR] waitpid failed: %d\n", errno);
        return -1;
    }

    return status;
}

int main(void)
{
    printf("[MAIN] PID: %d\n", getpid());

    int status = system("echo 'Soy el comando'");
    // int status = system(NULL);
    printf("[MAIN] system devolvió: %d\n", status);
    printf("[MAIN] sigo ejecutando\n");

    return 0;
}