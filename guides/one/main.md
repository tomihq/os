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

1. Revisar `man system`, `man execl` y `man access`, ya que son las herramientas que vamos a utilizar y necesitamos entender bien su comportamiento.

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
