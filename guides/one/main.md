# Guía 1: Procesos y API del S.O.

## 1. ¿Cuáles son los pasos que deben llevarse a cabo para realizar un cambio de contexto? 
1. Guardar el contexto del proceso que estaba ejecutando en su PCB.
2. Restaurar desde la PCB el contexto del proceso que va a ejecutarse.

## 2. PCB
El PCB (Process Control Block) de un sistema operativo para una arquitectura de 16 bits es

```c
Todo es con respecto al proceso que va a ser desalojado.
    struct PCB {
        int STATUS;
        int P_ID; //ID del proceso
        int PC; //próxima instrucción a ejecutar
        int R0; //valor al momento de desalojar
        ...
        int R15; //valor del registro R15 al ser desalojado
        int CPU_TIME //tiempo de ejecución del proceso
    }

    Notar que hablamos de "al desalojar" para los registros, porque si fuese el activo los agarrás de registros, no de la PCB. 
```

1. Implementar la rutina *ke_context_switch(PCB* pcb_0, PCB* pcb_1)*, encargada de realizar el cambio de contexto entre dos procesos (cuyos programas ya han sido cargados en memoria) debido a que el primero ha consumido su *quantum*
*pcb_0* es el puntero al PCB del proceso a ser desalojado.
*pcb_1* es el puntero al PCB del proceso a ser ejecutado a continuación.

Para implementarla, se cuenta con un lenguaje que posee acceso a los registros de procesador R0, R1, ..., R15 y las siguientes operaciones
1. .=.; //asignación entre registros y memoria
2. int ke_current_user_time() // devuelve el valor del cronómetro.
3. void ke_reset_current_user_time() //resetea el cronómetro
4. void ret(); // desapila el tope de la pila y reemplaza el PC
5. void set_current_process(int pid) //asigna al proceso con el pid como el siguiente a ejecutarse.

```c
Preguntas:
¿Quién incrementa CPU_TIME?
¿En dónde tengo el PC? Como para agarrarlo.
¿Está bien asumir que el proceso a desalojar lo guardamos como ready porque se "acabó su quantum"? 
¿El orden de las operaciones es indistinto, sin contar el ret()? Porque supongo que si tenés el ret primero y no preparaste lo otro, rompiste todo. 
¿Los pasos que corresponden al paso 1, son los que necesita solo la CPU para seguir ejecutando? ¿O cosas como "CPU_TIME" realmente son importantes?

void ke_context_switch(PCB* pcb_0, PCB* pcb_1){
    //Preservo en la PCB_0 los valores de los registros
    pcb_0 -> R0 = R0;
    pcb_0 -> R1 = R1;
    ...
    pcb_0 -> R15 = R15;
    pcb_0 -> STAT = "KE_READY"; //xq se terminó su "quantum" entonces está listo para ser ejecutado luego de vuelta.
    pcb_0 -> CPU_TIME += ke_current_user_time();
    
    //Cargo el nuevo PID
    set_current_process(pcb_1 -> P_ID);

    //Cambio el STAT del nuevo proceso
    pcb_1 -> STAT = "KE_RUNNING";

    //Cargo los nuevos registros
    R0 = pcb_1 -> R0;
    R1 = pcb_1 -> R1;
    ...
    R15 = pcb_1 -> R15;

    //Reseteo el CPU_TIME
    ke_reset_current_user_time();

    //Reemplaza el PC
    ret();
}
```

2. Identificar en el programa escrito cuáles son los pasos del ejercicio 1.

Nos interesan aquellos pasos que afectan a la ejecución en la CPU para que el proceso pueda continuar exactamente donde estaba.
Por lo tanto, los pasos del ejercicio 1 son: 
- Preservar Contexto: PC, R0...R15
- Restaurar Contexto: R0...R15, PC.

## Ejercicio 3: Describir la diferencia entre un system call y una llamada a una función de biblioteca
Una llamada a función de biblioteca es una llamada a código que forma parte de una biblioteca y normalmente comienza/ejecuta en user space. 

Una system call es el mecanismo mediante el cual un proceso solicita un servicio al kernel, provocando una transición controlada de user mode a kernel mode. 

Una función de biblioteca puede o no realizar internamente una system call.

## Ejercicio 4.
En el esquema de transición de estados que se incluye a continuación:

![Process State Machine](process-state-machine.png)

a) Dibujar las puntas de flechas que correspondan.
    - New
        -> Ready (crear el proceso)
    - Ready
        -> Running (ejecutar el proceso)
    - Running
        -> Ready (proceso perdió su quantum)
        -> Blocked (debe esperar algo para seguir ejecutando)
        -> Terminated (terminó su ejecución)
    - Blocked
        -> Ready (recibió lo que necesitaba para seguir ejecutando)

b) Explicar qué causa cada transición y qué componentes *(scheduler, proceso, etc.)* estarían involucrados.
    - New 
        -> Ready (Proceso hace fork())
    - Ready 
        -> Running (scheduler. quantum.)
    - Running
        -> Ready (quantum. scheduler.)
        -> Blocked (interrupción. scheduler.)
        -> Terminated (¿código + scheduler?)
    - Blocked
        -> Ready (interrupción + scheduler)

**Preguntar**:
1. ¿Qué pasa en el caso de que en la PCB el proceso está BLOCKED pero un padre te tira un SIGTERM? ¿No pasaría de BLOCKED a TERMINATED? Para mí faltan flechas o este es un modelo simplificado que "asume que termina" solo si antes estaba running.
2. ¿Para qué queremos el estado new? Para solamente "notar" que no debería estar en la PCB? Porque en realidad, en la PCB ya arrancaría en READY.
3. ¿Qué componentes estarían involucrados en el New -> Ready? Xq desde un punto de vista sencillo, con un "fork()" un proceso crea otro proceso y listo. 

## Ejercicio 5.
a) Utilizando únicamente la llamada al sistema *fork()*, escribir un programa tal que construya un árbol de procesos que represente la siguiente genealogoía:
- Abraham es padre de Homero
- Homero es padre de Bart, Lisa y Maggie. 
Cada proceso debe imprimir por pantalla el nombre de la persona que representa.

Asumimos que Abraham es el primero proceso del programa. 

El problema inicial que vemos, está en el archivo *5-initial-problem.c*

El problema de este código, es que nos muestra algo así: 
```s
[ABRAHAM][PID: 21200] Hola! 
[HOMERO][PID: 21201][PPID: 21200] douh! 
[BART][PID: 21202][PPID: 21201] que hay de nuevo viejo 
[LISA][PID: 21203][PPID: 21201] todo el maldito sistema está mal! 
[MAGGIE][PID: 21204][PPID: 3394] chup-chup 
[LISA][PID: 21205][PPID: 21202] todo el maldito sistema está mal! 
[MAGGIE][PID: 21207][PPID: 3394] chup-chup 
[MAGGIE][PID: 21206][PPID: 21202] chup-chup 
[MAGGIE][PID: 21209][PPID: 21205] chup-chup
```
¿Cuál es el problema? Que LISA está naciendo como hija de Homero y Bart. Lisa sólo debería ser hija de Homero. 
Notar que cuando hacemos pid_child_bart = fork() tenemos dos ramificaciones en el mismo lugar:
- Homero
- Bart
Entonces ambos van a llegar a hacer pid_child_lisa = fork(). 
Acá necesitamos evitar que Bart continúe ejecutando el código que crea a Lisa y Maggie. Es decir, sólo Homero debe alcanzar esos fork().

Algunas instancias de Maggie aparecen con un PPID correspondiente a otro proceso porque su padre original terminó antes de que Maggie ejecutara getppid(). Al quedar huérfana, el sistema la reparenta a otro proceso.

Una solución "no tan buena" sería como está en el archivo *5-first-solution.c*. Tiene cosas buenas, pero analicemos por separado.

1. Lo bueno es que claramente entendí que después de fork(), padre e hijo tienen espacios de direcciones separados, inicialmente con el mismo contenido. El SO suele implementar esto con copy-on-write, pero conceptualmente cada proceso tiene su propia memoria.
2. Solucioné el estado huérfano de Maggie usando un sleep(1000), que es totalmente impreciso, porque sería mejor que Homero permanezca vivo hasta que sus hijos terminen. Pero obviamente ahí nos metemos en otro mundo. Esta excepción se usó solamente asumiendo que los hijos van a tardar menos de 1s en crearse *(o podrían no hacerlo)*
```s
[ABRAHAM][PID: 22114] Hola! 
[HOMERO][PID: 22115][PPID: 22114] douh! 
[BART][PID: 22116][PPID: 22115] que hay de nuevo viejo 
[LISA][PID: 22117][PPID: 22115] todo el maldito sistema está mal! 
[MAGGIE][PID: 22118][PPID: 22115] chup-chup 
```
Una solución "mejor" sería como está en el archivo *5-second-solution.c*.

1. En vez de utilizar la condición de *parent == getpid()* opté por "terminar" cada proceso hijo en el mismo lugar. ¿Para qué? Para que no tenga que asumir que todo el código que sigue debajo, debe ser ejecutado por el resto. Y; en este contexto tiene sentido, porque si estamos hablando de "Crear Hijos" no tiene sentido que "Bart" deba hacer una excepción para no tener una hija Lisa. En este caso, la semántica está bien.
2. El problema sigue siendo el sleep(). ¿Por qué? Porque estamos confiando en que en el contexto de "Abraham", si asumimos que los forks tardan mucho, podría suceder que te duermas por 1s pero no sea suficiente para que el resto de forks se hayan hecho.
```s
[ABRAHAM][PID: 26087] Hola! 
[HOMERO][PID: 26088][PPID: 26087] douh! 
[LISA][PID: 26090][PPID: 26088] todo el maldito sistema está mal! 
[BART][PID: 26089][PPID: 26088] que hay de nuevo viejo 
[MAGGIE][PID: 26091][PPID: 26088] chup-chup 
```
La mejor solución, en mi opinión, es *5-third-solution.c*. Acá usamos wait() para bloquear al padre hasta que alguno de sus hijos termine y así poder recolectar su estado de terminación.
1. Le dimos la semántica correcta. Bart, Lisa y Maggie no llegan a ejecutar los fork() destinados a crear a sus hermanos, porque cada uno termina inmediatamente después de imprimir.
2. Ya no usamos tiempos que podrían cambiar, sino que usamos wait(NULL) para esperar a que alguno de los procesos hijo termine y recolectar su estado de terminación.
Notar que acá asumimos que van a terminar.

b) Modificar el programa anterior para que cumpla con las siguientes condiciones: 1) Homero termine
sólo después que terminen Bart, Lisa y Maggie, y 2) Abraham termine sólo después que termine
Homero

Es el paso natural hecho con el wait() para sacarnos el sleep de encima.

## Ejercicio 6.
Se agrega la llamada al sistema *void exec(const char *arg)*. 

Esta llamada al sistema reemplaza el programa actual por el código localizado en el string *(char *arg)*. 

Implementar una llamada al sistema que tenga el mismo comportamiento que la llamada *void system(const char *arg)*, usando las llamadas al sistema ofrecidas por el sistema operativo. 

--

Aclaraciones de la solución en *6.c*

1. Revisar `man system`, `man execl`, ya que son las herramientas que vamos a utilizar y necesitamos entender bien su comportamiento.

2. ¿Por qué necesitamos que `execl()` sea ejecutado por un proceso hijo? Para responder esto, primero hay que pensar qué comportamiento queremos de `system()`: necesitamos ejecutar un comando, esperar a que termine y luego devolver el control al programa que llamó a `system()`.

   El problema es que `execl()` reemplaza el programa que está ejecutando el proceso actual. Si tiene éxito, no retorna. Por lo tanto, si ejecutáramos `execl()` directamente desde el proceso que llamó a `system()`, perderíamos el programa original y nunca podríamos retornar a él.

   La solución es:

   ```
   crear hijo -> hijo ejecuta execl() -> padre espera al hijo -> system() retorna
   ```

   De esta manera, el proceso hijo es el que reemplaza su programa mediante `execl()`, mientras que el padre conserva el programa original y queda esperando su terminación.

   Si `fork()` falla y no podemos crear al hijo, `system()` retorna inmediatamente con `-1`. El responsable de indicar la causa del error es `fork()`, que deja seteado `errno`.

3. El caso `command == NULL` es especial. En este caso no se intenta ejecutar ningún comando, sino que `system()` debe comprobar si existe una shell disponible:

   * Retorna un valor distinto de `0` si hay una shell disponible.
   * Retorna `0` si no la hay.

4. Después de `fork()`, padre e hijo siguen caminos de ejecución diferentes.

   El hijo entra en:

   ```
   if (child == 0)
   ```

   y ejecuta `execl()`.

   Si `execl()` tiene éxito, el hijo deja de ejecutar nuestro programa y pasa a ejecutar `/bin/sh`. Por lo tanto, nunca alcanza las instrucciones posteriores al `execl()`.

   Si `execl()` falla, retorna `-1` y el hijo continúa ejecutando nuestro código. Por eso, en ese caso sí alcanza el `printf()` del error y posteriormente `_exit(127)`.

   Mientras todo esto ocurre, el padre nunca ejecuta `execl()`: como para él `child != 0`, no entra en ese `if` y continúa hasta `waitpid()`, donde queda bloqueado esperando la terminación del hijo.

   Cuando el hijo finalmente termina, `waitpid()` retorna, el padre continúa su ejecución y `system()` puede devolver el control al programa original.

5. Si asumimos que no tenemos acceso a `access` que nos dice si hay una shell disponible, tenemos la solución `6-without-access.c` que intenta ejecutar un comando básico con `execl` en una shell. Si da todo ok, significa que la shell está disponible y pudo ejecutarse correctamente en ese momento. 
¿Qué significa acá el "tener shell o no"? ¿Alguna condición de carrera por limitar la cantidad de ejecuciones de shell? No.
Una shell podría no estar disponible por:
- /bin/sh no existe.
- /bin/sh existe pero no puede ejecutarse por permisos
- El entorno del proceso no permite acceder a ella.
- Hay algún problema del sistema que impide ejecutar la shell.

Si no ocurre ninguna de esas condiciones, esperamos que system(NULL) indique que hay una shell disponible. Esto no significa que exista una instancia de shell reservada o esperando ser utilizada, sino que el sistema puede ejecutar una shell en ese momento. 

Múltiples procesos pueden ejecutar /bin/sh simultáneamente, por lo que no existe una condición de carrera relacionada con “quedarse sin shells” ni es necesario utilizar locks.

## Ejercicio 7
Programar en C el ejercicio 5b y 6. Ya están hechos, ja.

## Ejercicio 8
Veamos el siguiente fragmento de código de un fork
```c
int main(int argc, char const *argv[]) {
    int dato = 0;

    pid_t pid = fork();

    // Si no hay error, pid vale 0 para el hijo
    // y el valor del process id del hijo para el padre
    if (pid == -1)
        exit(EXIT_FAILURE); // si es -1, hubo un error
    else if (pid == 0) {
        for (int i = 0; i < 3; i++) {
            dato++;
            printf("Dato hijo: %d\n", dato);
        }
    }
    else {
        for (int i = 0; i < 3; i++) {
            printf("Dato padre: %d\n", dato);
        }
    }

    exit(EXIT_SUCCESS); // cada uno finaliza su proceso
}
```
¿Son iguales los resultados mostrados de la variable `dato` para el padre y para el hijo? ¿Qué está sucediendo?
No. No son iguales. Cuando hacemos un fork(), padre e hijo tienen espacios de direcciones virtuales separados, aunque inicialmente contienen la misma información. Mediante copy-on-write, sus páginas pueden apuntar inicialmente a las mismas páginas físicas. Cuando alguno de los procesos intenta escribir en una de ellas, el SO crea una copia privada de esa página para ese proceso.

## Ejercicio 9
Dado un programa de dos procesos, padre e hijo, se quiere tener el siguiente comportamiento:

- Uno de los dos procesos debe escribir en pantalla ping y su número de PID. 
- Automáticamente el otro proceso debe escribir pong con su número de PID. 
Se quiere repetir este comportamiento 3 veces.

Luego de esto, se desea preguntar al usuario si quiere finalizar la ejecución o no. En caso que conteste
que si, el padre debe terminar con la ejecución de su hijo y finalizar. En caso que se conteste que no,
se vuelve a repetir el proceso antes dicho

--

Necesito:
1. Mantener contador de veces en padre (PING).
2. Mantener contador de veces en hijo (PONG).
3. Mantener pausado un proceso mientras que el otro hace lo que tiene que hacer.
   1. Padre - Hijo (arrancan).
   2. Padre dice PING (manda señal) - Hijo pausado
   3. Padre pausado - Hijo dice PONG (manda señal)
   4. ... hasta que se haga 3 veces.
4. El padre es el que nos dice: "¿Querés finalizar la ejecución?" (xq viene después del último PONG).
   1. Si damos SÍ, terminamos la ejecución del hijo y liberamos los recursos y finalizamos el programa padre.
   2. Si damos NO, volvemos a ejecutar todo desde 0 (el flujo de PING/PONG).

Necesito señales, implementar alguna señal en particular que ambos entiendan y verificar que son ellos quién la envió antes de hacer algo.
Puedo usar `pause()` para colgar un proceso hasta que llegue una señal, pero esa señal me la podría mandar cualquier proceso *(y además ser cualquier señal que no espero)*. 

Por lo tanto necesito:
- Espero la señal X particular, que el proceso padre e hijo escuchan. El resto no las escucho, no me interesan.
- Verifico que cuando llegue la señal X, sea o bien el padre o el hijo.

--

Acerca de la solución
1. Asumo que el SIGTERM al hijo solo lo envía el padre. De lo contrario, el hijo debería verificar que la señal provenga efectivamente de su padre.
2. Como padre e hijo necesitan la misma configuración inicial de señales, definimos las máscaras y handlers antes del `fork()`. El hijo hereda esa configuración al ser creado.
3. `while(1)` equivale a un bucle infinito. En este caso, permite que el hijo permanezca vivo esperando nuevas señales hasta que el padre decida terminarlo.
4. `kill(getpid(), SEÑAL)` permite que un proceso se envíe una señal a sí mismo.
5. Para finalizar correctamente la ejecución del padre, primero esperamos específicamente la terminación del hijo mediante: `waitpid(child, NULL, 0);` 
6. Manejamos la sincronización de turnos mediante una variable global `signal_received`. El handler modifica esta variable cuando llega la señal esperada, y el flujo normal del proceso continúa únicamente cuando dicha señal fue recibida.

## Ejercicio 10
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

## Ejercicio 11
```c
void bsend(pid dst, int msg): envia el valor msg al proceso dst
int breceive(pid src): recibe un mensaje del proceso src.

Las llamadas son bloqueantes (no se pueden encolar mensajes. Eso quiere decir que si mandás un mensaje, tenés que esperar sí o sí que te lo reciban).
No tenemos ningún buffer que nos permita seguir "trabajando". 

pid get_current_pid(): devuelve el process id que hace la llamada a bsend o breceive.

a) Escribir un programa que cree un segundo proceso, para luego efectuar la siguiente secuencia de
mensajes entre ambos
Padre envía a Hijo el valor i
Hijo envía a Padre el valor i+1
Padre envía a Hijo el valor i+2
...

pseudocódigo:
    1. Obtenés el PID del padre.
    2. Creás el hijo.
    3. Verificás child == 0
        a. Caso True: hacés un while(1) porque te tenés que quedar escuchando. Hacés el receive del padre, tomás el valor, le sumás uno y lo mandás al padre. Luego, repetís.
        b. Caso False: hacés un while(1) porque te tenés que quedar escuchando. Mandás al hijo el valor, luego te quedás escuchando el valor que te manda. Lo recibís, y se repite el proceso.
código:
int main() {
    pid_t parent = getpid();
    pid_t child = fork();

    if (child == 0) {
        while (1) {
            int i = breceive(parent);
            bsend(parent, i + 1);
        }

    } else {
        int i = 0;

        while (1) {
            bsend(child, i);
            i = breceive(child) + 1;
        }
    }
}
```