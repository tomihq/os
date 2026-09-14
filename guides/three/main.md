# Guía 3: Sincronización entre procesos

## Ejercicio 1
Se tienen dos programas, con procesos A y B que ejecutan concurrentemente. No se tiene información sobre cómo serán ejecutados por el scheduler. 

a) ¿Hay una única salida en pantalla posible para cada proceso?

b) Indicar todas las salidas posibles para cada caso.

```text
Programa 1

// variables compartidas
x = 0;

A(): 
    x = x + 1;
    printf("%d, x)

B():
    x = x + 1;
```

```text
Programa 2

// variables compartidas
x = 0;
y = 0; 

A(): 
    for(; x < 4; x++){
        y = 0; 
        printf("%d", x);
        y = 1;
    }

B():
    while (x<4){
        if(y == 1){
            printf("a");
        }
    }
```

a) No. 

b) Analicemos el por qué del "no" anterior.

### Programa 1
```text
Programa 1

// variables compartidas
x = 0;

A(): 
    x = x + 1;
    printf("%d, x)

B():
    x = x + 1;
```

Tiene dos procesos que se ejecutan concurrentemente. Eso quiere decir que no tenemos la garantía de saber quién va a ejecutar y por qué. Cuando tenemos este tipo de problema, tenemos que prestar atención a los recursos compartidos, porque puede existir una condición de carrera: el resultado depende del orden en que se intercalen las operaciones de los procesos.

Tanto el proceso A y B utilizan una variable compartida x. Eso quiere decir que el recurso el cual comparten aquí es esa misma variable. El problema aparece porque ambos procesos realizan una operación de lectura-modificación-escritura sobre la misma variable compartida.

La operación que hacen ambos, no es atómica. Como x = x + 1 no es atómica, conceptualmente podemos pensarla como varias operaciones:

1. READ:  obtener el valor actual de x
2. ADD:   sumarle 1 al valor obtenido
3. WRITE: guardar el nuevo valor en x

Por lo tanto, el proceso puede ser desalojado entre estas operaciones, permitiendo que el otro proceso ejecute sobre la misma variable.

Como estamos en memoria compartida, cuando un proceso hace una nueva operación de lectura sobre x, obtiene el valor que x tiene en ese momento. Sin embargo, hay que distinguir esto de una operación x = x + 1 que ya comenzó: si el proceso ya hizo el READ y es interrumpido, cuando luego continúe no vuelve a leer x para completar esa misma operación, sino que continúa utilizando el valor que había obtenido en el READ anterior para realizar el ADD y finalmente el WRITE.

Por ejemplo, si inicialmente:
```
x = 0
```
y A ejecuta:
```
READ x → obtiene 0
```
pero es interrumpido antes del WRITE, A conserva ese valor 0 como parte de la operación que estaba realizando. Si mientras tanto B modifica x, cuando A vuelva a ejecutarse no vuelve a obtener automáticamente el nuevo valor de x: continúa su operación a partir del 0 que ya había leído.

En cambio, si A posteriormente ejecuta:
```
printf("%d", x);
```
esto es una nueva lectura de x, por lo que obtiene el valor que x tenga en ese momento.

Veamos algunas posibles ejecuciones.

Ejecución posible (1):
```text
x = 0;
```
1. [A] busca el valor de `x`. Obtiene `0`.
2. [B] busca el valor de `x`. Obtiene `0`.
3. [A] muta el valor de `x`. `x = 1`.
4. [B] muta el valor de `x`. `x = 1`.

Notar que acá ya tuvimos un problema, porque si la variable es compartida, B ya había leído x = 0 como parte de su operación x = x + 1. Aunque A modifique posteriormente la variable compartida, B puede continuar utilizando ese 0 que había obtenido y finalmente escribir 1.

5. [A] muestra el valor: `1`.
6. [B] termina.
7. [A] termina.


Notar que acá [A] termina mostrando el valor de 1! Nosotros esperaríamos 2! 

El problema que hubo es que los modificaron la variable luego de haber leído el valor (0) en la operación inicial.

Ejecución posible (2):
```text
x = 0; 
```

1. [A] busca el valor de `x`. Obtiene `0`.
2. [A] muta el valor de `x`. `x = 1`.
3. [B] busca el valor de `x`. Obtiene `1`.
4. [B] muta el valor de `x`. `x = 2`.
5. [A] muestra el valor: `2`.
6. [B] termina.
7. [A] termina.


¡Notemos que acá sí hubo algo interesante!
1. En este caso, x termina valiendo 2 porque cada proceso completa su operación de lectura-modificación-escritura antes de que el otro comience la suya.
2. Como le dimos el tiempo para hacer la operación de (READ + WRITE) todo salió como esperábamos

Ejecución posible (3):
```text
x = 0;
```
1. [B] busca el valor de `x`. Obtiene `0`.
2. [A] busca el valor de `x`. Obtiene `0`.
3. [B] muta el valor de `x`. `x = 1`.
4. [A] muta el valor de `x`. `x = 1`.
5. [B] termina.
6. [A] muestra el valor: `1`.
7. [A] termina.


Notar que acá estamos en el mismo problema que en la ejecución posible 1! 

Ejecución posible (4):
```text
x = 0;
```
1. [B] busca el valor de `x`. Obtiene `0`.
2. [B] muta el valor de `x`. `x = 1`.
3. [A] busca el valor de `x`. Obtiene `1`.
4. [A] muta el valor de `x`. `x = 2`.
5. [B] termina.
6. [A] muestra el valor: `2`.
7. [A] termina.

¡Notar que acá tuvimos el mismo resultado que la ejecución 2! ¿Qué sucedió?

Le dimos el tiempo de hacer el READ + WRITE de un recurso. Es decir, el otro proceso no ejecutó hasta que la operación se hizo completa.

Esto muestra que el resultado depende de cómo se intercalen las operaciones de los procesos. Si queremos garantizar que x = x + 1 se ejecute completa antes de que el otro proceso acceda a x, necesitamos sincronización.

La introducción a esto es: usar secciones críticas con mutex para evitar que otro proceso no haga nada con un recurso si otro lo tiene tomado. 

### Programa 2
```text
Programa 2

// variables compartidas
x = 0;
y = 0; 

A(): 
    for((no hay inicialización); x < 4; x++){
        y = 0; 
        printf("%d", x);
        y = 1;
    }

B():
    while (x<4){
        if(y == 1){
            printf("a");
        }
    }
```
Ejecución posible (1)

Partimos de:

```text
x = 0;
y = 0;
```

Una posible intercalación de las operaciones de A y B es la siguiente:

* [A] busca el valor de `x`. Obtiene `0` (E1).
* [B] busca el valor de `x`. Obtiene `0` (E2).
* [A] evalúa `x < 4`. Se cumple (ref. E1).
* [B] evalúa `x < 4`. Se cumple (ref. E2).
* [A] busca el valor de `y`. Obtiene `0` (E3).
* [B] busca el valor de `y`. Obtiene `0` (E4).
* [A] ejecuta `y = 0`. `y` permanece en `0`.
* [B] evalúa `y == 1` utilizando el valor `0` que acaba de leer. No se cumple.
* [A] ejecuta `printf("%d", x)`. Como `x` vale `0`, muestra **`0`**.
* [B] busca nuevamente el valor de `x`. Obtiene `0` (E6).
* [A] busca el valor de `y`. Obtiene `0` (E7).
* [B] evalúa `x < 4`. Se cumple (ref. E6).
* [A] ejecuta `y = 1`. `y` pasa a valer `1`.
* [B] busca nuevamente el valor de `y`. Obtiene `1` (E8).
* [A] busca el valor de `x`. Obtiene `0` (E9).
* [B] evalúa `y == 1`. Se cumple (ref. E8).
* [A] ejecuta el `x++` del `for`. `x` pasa a valer `1`.
* [B] ejecuta `printf("a")`. Muestra **`a`**.
* [A] busca nuevamente el valor de `x`. Obtiene `1` (E10).
* [B] busca nuevamente el valor de `x`. Obtiene `1` (E11).
* [A] evalúa `x < 4`. Se cumple (ref. E10).
* [B] evalúa `x < 4`. Se cumple (ref. E11).
* [A] busca el valor de `y`. Obtiene `1` (E12).
* [A] ejecuta `y = 0`. `y` pasa a valer `0`.
* [A] ejecuta `printf("%d", x)`. Como `x` vale `1`, muestra **`1`**.
* [A] ejecuta `y = 1`. `y` vuelve a valer `1`.
* [B] busca nuevamente el valor de `y`. Obtiene `1` (E13).
* [B] evalúa `y == 1`. Se cumple.
* [B] ejecuta `printf("a")`. Muestra **`a`**.

Esta es **una única ejecución posible**. El scheduler podría intercalar las operaciones de A y B de muchas otras maneras, produciendo distintas salidas.

Por ejemplo, B podría ejecutar varias veces el `if (y == 1)` mientras `y` siga valiendo `1`, y por lo tanto podría imprimir varias `a` antes de que A vuelva a modificar `y`.

Ejecución posible (2)

Partimos de:

```text
x = 0;
y = 0;
```

Una posible intercalación de las operaciones de A y B es la siguiente:

1. [A] busca el valor de `x`. Obtiene `0`.
2. [A] evalúa `x < 4`. Se cumple.
3. [A] ejecuta `y = 0`. `y` permanece en `0`.
4. [A] ejecuta `printf("%d", x)`. Muestra **`0`**.
5. [A] ejecuta `y = 1`. `y` pasa a valer `1`.
6. [A] ejecuta `x++`. `x` pasa a valer `1`.
7. [A] vuelve a buscar `x`. Obtiene `1`.
8. [A] evalúa `x < 4`. Se cumple.
9. [A] ejecuta `y = 0`. `y` pasa a valer `0`.
10. [A] ejecuta `printf("%d", x)`. Muestra **`1`**.
11. [A] ejecuta `y = 1`. `y` pasa a valer `1`.
12. [A] ejecuta `x++`. `x` pasa a valer `2`.
13. [B] busca el valor de `x`. Obtiene `2`.
14. [B] evalúa `x < 4`. Se cumple.
15. [B] busca el valor de `y`. Obtiene `1`.
16. [B] evalúa `y == 1`. Se cumple.
17. [B] muestra **`a`**.
18. [B] vuelve a evaluar el `while`: busca `x`. Obtiene `2`.
19. [B] evalúa `x < 4`. Se cumple.
20. [B] busca nuevamente `y`. Obtiene `1`.
21. [B] evalúa `y == 1`. Se cumple.
22. [B] muestra **`a`**.
23. [B] vuelve a evaluar el `while`: busca `x`. Obtiene `2`.
24. [B] evalúa `x < 4`. Se cumple.
25. [B] vuelve a buscar `y`. Obtiene `1`.
26. [B] evalúa `y == 1`. Se cumple.
27. [B] muestra **`a`**.
28. ... y podría seguir imprimiendo `a` indefinidamente mientras A no vuelva a ejecutarse.

En esta ejecución, hasta ese momento, la salida sería:

```text
01aaa...
```

Conclusión: ¡cuidado con el uso de recursos compartidos!

## Ejercicio 2
Se tiene un sistema con 4 procesos accediendo a una variable compartida *x* y un *mutex*. Los 4 procesos ejecutan el siguiente código. 

Ciertas decisiones que toma cada proceso dependen del valor de la variable compartida.

Se debe asegurar que cada vez que un proceso lee la variable compartida, previamente solicita el *mutex* y luego lo libera.

¿Estos procesos cumplen con lo planteado? ¿Pueden ser víctimas de *race condition*?

```text
    x = 0; //variable compartida
    mutex(1); // mutex compartido

    while(1) {
        mutex.wait();
        y = x; //lectura de x
        mutex.signal();
        if(y <= 5){
            x++;
        }else{
            x--; 
        }
    }
```

**Respuesta**: No, no cumplen con lo planteado. Cuando solicitan el mutex, deberían ingresar a la sección crítica y realizar ahí toda la operación sobre x: leerla, decidir si incrementarla o decrementarla y modificarla. Recién después deberían liberar el mutex mediante signal().

El problema en este código es que se libera el mutex inmediatamente después de leer x, pero x se modifica posteriormente, cuando el proceso ya salió de la sección crítica. 

Por lo tanto, otro proceso puede acceder a x entre la lectura y la modificación, generando una condición de carrera.

El código debería ser algo así
```text
    x = 0; //variable compartida
    mutex(1); // mutex compartido

    while(1) {
        mutex.wait();
        y = x; //lectura de x
        if(y <= 5){
            x++;
        }else{
            x--; 
        }
        mutex.signal();
    }
```

## Ejercicio 3
La operación `wait` sobre semáforos suele utilizar una cola para almacenar los pedidos que se encuentran en espera. Si en lugar de una cola utilizara una pila (LIFO), determinar si habría `starvation` o funcionará correctamente.

**Respuesta**: Sí, podría haber starvation.
Supongamos que P1 y P2 quedan esperando el semáforo. Luego siguen llegando nuevos procesos P3, P4, ..., que también ejecutan wait y se apilan.

Si al liberar el semáforo siempre se atiende al proceso que está en el tope de la pila (LIFO), se atenderá primero a los procesos que llegaron más recientemente. Si continúan llegando nuevos procesos, P1 y P2 pueden quedar permanentemente debajo de ellos y nunca ser seleccionados para continuar.

Por lo tanto, el uso de una pila puede producir starvation. Una cola FIFO evita este problema al garantizar que los procesos sean atendidos en el orden en que llegaron.

Conclusión: priorizar siempre al último que llegó no es buena idea si no se considera el caso de que siempre podrían llegar más y más.

## Ejercicio 4
Demostrar que en caso de que las operaciones de semáforos `wait()` y `signal()` no se ejecuten atómicamente, entonces se viola la propidad de exclusión mutua *(un recurso no puede estar asignado a más de un proceso por vez)*

## Ejercicio 5
Se tienen *n* procesos: P1, P2, ..., Pn que ejecutan el siguiente código. 

Se espera que todos los procesos terminen de ejecutar la función `preparado()` antes de que alguno de ellos llame a la función `critica()`. ¿Por qué la siguiente solución permite `starvation`? Modificar el código para arreglarlo.

```text
    preparado()

    mutex.wait()
    count = count+1
    mutex.signal()

    if(count == n){
        barrera.signal()
    }

    barrera.wait()

    critica()
```

**Respuesta:** Sí, puede suceder. Veamos por qué.

Recordemos que `wait()` puede interpretarse como **pedir un permiso** y `signal()` como **otorgar un permiso**.

Analicemos el programa original.

Primero, ¿tenemos un problema de *race condition* con `count`? **No.** El acceso a `count` está protegido correctamente por `mutex`: cada proceso hace `mutex.wait()`, modifica `count` y luego libera el mutex mediante `mutex.signal()`.

Por lo tanto, todos los procesos P1, P2, ..., Pn llegan eventualmente hasta el `mutex.signal()`.

Ahora bien, ¿qué ocurre con P1, P2, ..., Pn-1? Como todavía no se alcanzó `count == n`, ninguno ejecuta el `if`. Por lo tanto, todos continúan hasta:

```c
barrera.wait();
```

y quedan bloqueados esperando atravesar la barrera.

Finalmente llega Pn. En este momento `count == n`, por lo que Pn entra al `if` y ejecuta:

```c
barrera.signal();
```

Ese único `signal()` permite que **uno solo** de los procesos que estaban esperando en `barrera.wait()` continúe.

Supongamos que se desbloquea P1. Entonces P1 continúa con:

```c
critica();
```

Mientras tanto, P2, ..., Pn-1 siguen bloqueados en `barrera.wait()`.

¿Y qué ocurre con Pn? Pn continúa su flujo normal y también llega a:

```c
barrera.wait();
```

Pero el único permiso producido por el `signal()` ya fue consumido por P1. Por lo tanto, Pn también queda bloqueado.

En consecuencia, **P2, ..., Pn también quedan bloqueados indefinidamente**.

El problema, entonces, es que el último proceso realiza solamente **un `signal()`**, cuando hay `n` procesos que deben poder atravesar la barrera.

Por lo tanto, la implementación original de la barrera es incorrecta: **solo permite que un proceso continúe, mientras que los demás quedan bloqueados.**

La solución sería emitir `barrera.signal()` **n** veces.
```text
 preparado()

    mutex.wait()
    count = count+1
    mutex.signal()

    if(count == n){
        for(int i = 0; i<n; i++){
            barrera.signal();
        }
    }

    barrera.wait()

    critica()
```

## Ejercicio 6
Cambiar la solución del ejercicio anterior por una solución basada solamente en las herramientas atómicas vistas en las clases, que se implementen a nivel de hardware, y responder las siguientes preguntas:

- ¿Cuál de las dos soluciones genera un código más legible?
- ¿Cuál de ellas es más eficiente? ¿Por qué?
- ¿Qué soporte require cada una de ellas del SO y del HW?

**Respuesta**: las herramientas atómicas vistas en las clases a través de HW son implementaciones del TAS.

¿Qué es el *TAS*? TestAndSet y básicamente significa: "fijate si alguien tiene el recurso, y si no lo tiene, apropiátelo". Es una operación atómica.

Hay varias implementaciones modernas tales como:

- SpinLock (TASLock)
- TTASLock

¿Cuál es la desventaja de TAS con respecto a Semáforos? que hacen busy waiting.

SpinLock constantemente está preguntando: "¿puedo tener el acceso?" mientras que TTASLock tiene una estrategia un poco más inteligente aprovechando el caché, pero también tiene busy waiting.

¿Cuál es más legible? la solución con semáforos es más legible, porque las operaciones `wait()` y `signal()` expresan directamente la intención de bloquear y liberar el acceso a la sección crítica. La solución con TAS requiere entender la primitiva atómica y el busy waiting.

¿Cuál es más eficiente? la de semáforos. Lo comentamos anteriormente. En los semáforos, utilizamos `wait()` para básicamente para irnos a dormir hasta que nos levantan con `signal()` y volver a consumir CPU.
No obstante, si la sección crítica es extremadamente corta, un spinlock puede ser más eficiente al evitar el costo de bloquear y despertar un proceso.

¿Qué soporte requiere del SO y del Hardware cada una de ellas?

La solución basada en TAS requiere una primitiva atómica proporcionada por el hardware, como TestAndSet, que permita leer y modificar el valor del lock de manera indivisible. La espera se realiza mediante busy waiting, por lo que no requiere que el SO bloquee al proceso que está esperando.

La solución basada en semáforos requiere soporte del sistema operativo para bloquear y despertar procesos y administrar los procesos que esperan el recurso. Internamente, el SO puede utilizar primitivas atómicas de hardware para implementar correctamente las operaciones sobre el estado del semáforo.

TAS → Hardware

Es una operación atómica provista por el hardware.
El HW garantiza que el test + set sea indivisible.
A partir de TAS podés construir mecanismos como Spinlocks.
El Spinlock típico hace busy waiting.

Semáforos → Sistema Operativo

wait() / signal() son mecanismos de sincronización gestionados por el SO.
El SO puede bloquear y despertar procesos.
Por eso no necesitás que el proceso se quede consumiendo CPU mientras espera.
Internamente, el SO puede apoyarse en operaciones atómicas de hardware, pero eso queda debajo de la abstracción del semáforo.

## Ejercicio 7
Se tienen N procesos, P0 , P1 , ..., PN −1 (donde N es un parámetro). Se requiere sincronizarlos de
manera que la secuencia de ejecución sea Pi , Pi+1 , ..., PN −1 , P0 , ..., Pi−1 (donde i es otro parámetro).

Escribir el código que deben ejecutar cada uno de los procesos para cumplir con la sincronización requerida utilizando semáforos (no olvidar los valores iniciales).

**Respuesta**: necesitamos que todos arranquen bloqueados excepto el proceso Pi. Vamos a optar por una solución en la cual los procesos se conocen entre sí, es decir, cada proceso va a despertar al siguiente.

¿Qué significa que uno va a despertar a otro? Que, cuando termina de ejecutar, cada proceso hace `signal()` sobre el semáforo correspondiente al siguiente proceso, dándole el permiso para ejecutar.

Cada hijo va a hacer `wait()` sobre su propio semáforo.

De esta manera, no tenemos problemas con el orden en que el scheduler ejecuta los procesos: aunque cualquier proceso pueda recibir CPU primero, solo el proceso cuyo semáforo fue habilitado puede continuar. El turno queda representado por los semáforos.

Pseudocódigo:
```c

semáforo(sem[0..N-1])

para j = 0 hasta N-1:
    si j == i:
        sem[j] = 1
    sino:
        sem[j] = 0

para j = 0 hasta N-1:
    crear_proceso(Pj)


Proceso Pj:

    sem[j].wait();

    ejecutar()

    sem[(j+1) % N].signal();
```

Notar que cada proceso hijo *(thread)* comparten el mismo espacio de memoria, por lo que pueden mutar la misma variable *sem*.

## Ejercicio 8
Considerar cada uno de los siguientes enunciados. Para cada caso, escribir el código que permita la ejecución de los procesos según la forma de sincronización planteada utilizando **semáforos**. Se debe argumentar por qué cada solución evita *starvation*.

1. Se tienen tres procesos (A, B, C). Se desea que el orden en que se ejecutan sea el orden alfabético, es decir que las secuencias normales deben ser: ABC, ABC, ABC.
2. Idem anterior, pero se desea que la secuencia normal sea: BBCA, BBCA, BBCA


**Respuesta**: en el contexto de semáforos, tendríamos starvation si el proceso se duerme y nunca más lo despiertan. Es decir, no obtiene su turno.

1. Inicializamos un semáforo para cada proceso, todos en 0 excepto el de la posición `i`, que se inicializa en 1. Cada proceso escucha su propio semáforo. Si `wait(&sem[j])` puede continuar, significa que el proceso `Pj` tiene el permiso y se desbloquea. Cuando termina de ejecutar, hace `signal(&sem[(j+1) % N])`, entregándole el permiso al siguiente proceso.
La ejecución inicial es posible porque antes de crear los procesos ya dejamos preparado el primer permiso: `sem[i] = 1`. Por lo tanto, cuando `Pi` se crea y hace `wait(&sem[i])`, consume inmediatamente ese permiso que ya tenía asignado y puede ejecutar.
A partir de ahí, cada proceso le va dando el permiso al siguiente: `Pi → Pi+1 → Pi+2 → ... → Pi`. De esta forma, el permiso va circulando entre todos los participantes en cada ronda. Esto evita inanición porque ningún proceso depende de competir por el permiso: el proceso anterior se lo entrega directamente al siguiente, y así todos reciben el turno eventualmente.
2. Este es más interesante. 
   1. (la que hice): Voy a asumir que BB significa: "B hace su acción 2 veces en su turno". Para eso podemos básicamente hacer una variable dentro del proceso que nos diga cuantas veces ejecutar la acción que hace el proceso. Si `j==1` entonces tenemos que ejecutarlo más de una vez. Lo que evita que se ejecute muchas veces (uno diferente de B) es que el while adentro tiene un for que justamente dice cuantas veces tiene que hacer su proceso por cada vez que tiene la ejecución. 
   2. (la que quería hacer): ¿no se puede hacer algo recursivo solo en B hasta que acabe sus "N ejecuciones?" porque no sé, para mí el BB es: "B suelta el control y lo toma B de vuelta" y no "B hace dos veces lo mismo sin soltarlo".
   
   No hay starvation en ninguna porque es similar a la mencionada anteriormente. Siempre arrancamos con un proceso que tiene permiso, y luego el resto se va despertando a medida que su semáforo se pone en 1.
3. Si bien el enunciado dice que el orden es "A**" tenemos que asumir que no sabemos con qué certeza los procesos se crean. 
    - **B** y **C** deben arrancar bloqueados.
    - **A** produce 2 recursos por turno. 
    - **A** no se vuelve a ejecutar hasta que le hayan consumido los dos recursos.
    - **B** y **C** pueden ejecutarse en cualquier orden. 
    - **B** puede consumir los dos recursos o bien, los consume **C**. Notar que acá hay una condición de carrera que tenemos que **evitar**. 
    - Siempre que haya **un recurso**, el cada proceso agarra uno por vez. 
        - B agarra un recurso 1 - C agarra recurso 2 (notar que el orden es indistinto. También podría pasar que C se quede re contra colgado y B termine de consumir uno y agarre el otro)
        - A recibe que ya se consumieron los recursos. A produce más.
        - A emite la señal de que están listos.
        - C termina de consumir recurso. B termina de consumir recurso. 
        - Como A ya emitió que hay recursos, C y B pueden agarrarlos.

    ¿Cómo hacemos esto? Como los semáforos representan permisos, podemos usar uno para representar la disponibilidad de recursos y otro para representar los recursos que ya fueron consumidos. `B` y `C` esperan mediante `wait()` a que haya un recurso disponible. Cada vez que uno consume un recurso, hace `signal()` sobre el semáforo de consumidos. A espera recibir dos señales de consumo y, recién entonces, vuelve a producir los dos recursos y libera dos permisos para `B` y `C`.

    De esta forma, no necesitamos consultar el valor del semáforo. Cada `wait()` representa la espera de un evento concreto, y los dos `wait()` (o un for de dos iteraciones) garantizan que A no produzca nuevamente hasta que se hayan consumido ambos recursos.
    
    ```text
        semáforo(recursos) = 0
        semáforo(consumidos) = 0

        Productor A
            mientras verdadero:
                para i = 1 hasta 2:
                    consumidos.wait()

                producir recurso 1
                producir recurso 2

                recursos.signal()
                recursos.signal()

        Productor B/C
            mientras verdadero:
                wait(recursos)

                consumir 1 recurso

                consumidos.signal()
    ```
4. Se le agregan unas restricciones al 3.
   - **A** interviene en cada ronda *(llamemos ronda a la ejecución de la producción hasta el consumo de todos los recursos)* y aparece una vez por turno *(cuando no hay recursos)*. 
   - **B** consume los dos recursos en orden. Primero el primer recurso, y luego, en otro turno, el otro recurso. 
   - **C** **siempre** consume ambos recursos en un único turno. 
   - La ejecución entre **B** y **C** es alternada.

  ¿Qué problemáticas hay acá? 
  - Si es el turno de **B**. Consume un recurso y tiene el control nuevamente para consumir el otro recurso. Siempre tiene dos turnos, en cada uno consume un recurso.
  - Si es el turno de **C**, consume AMBOS recursos a la vez.
  - Hay exclusión mutua sobre quien tiene el control: más allá de que haya **dos recursos**, solo **uno** opera con esos recursos en la ronda. Lo que necesitamos es que haya un permiso para modificar los recursos, y el que lo toma, hace lo que quiere con los N recursos. 
  - Hay que decidir cómo definir el mecanismo de ejecución alternada. Podríamos definir un semáforo que lo usen B y C para que se digan entre ellos: "es tu turno". Entonces cuando el productor dice que ya están los recursos, ellos se fijan si ese "semáforo" de permiso lo tienen ellos. 
  - **Preguntar**: asumo que arranca SIEMPRE B porque así está en el enunciado. Sino habría que tirar algun random() para que varíe eso. 
    **Respuesta**: **sí, correcto. es una decisión tuya**.
  - **Preguntar**: ¿En qué se diferencia **BB** de **C** a nivel de consumo de recursos? ¿**C** consume ambos en un solo turno, mientras que **B** consume 1 recurso por turno? Porque en mi pseudocódigo entonces estaría medio raro eso. Porque **C** entonces tendría sentido que tenga **wait(), wait()**, pero B debería tener: **wait()**, hacer una especie de **dejar el control**, tomarlo de vuelta y hacer **wait()** de vuelta. 
    **Respuesta**: **Sí, la idea es que B haga un loop en sí mismo hasta que no haya recursos mientras que C consume dos cosas hardcodeadas** 

     ```text
        semáforo(recursos) = 0
        semáforo(consumidos) = 0

        //esto es un inicializador. es un pseudocodigo
        semáforo consumidores[2];
        consumidores[0] = 1;
        consumidores[1] = 0; 

        Productor A
            mientras verdadero:
          
                producir recurso 1
                producir recurso 2
                recursos.signal()
                recursos.signal()

                para i = 1 hasta 2:
                    consumidos.wait()

        Consumidor B
            ejecucionesRestantesB = 2; 
            mientras verdadero:
                consumidores[0].wait() 
                
                if(ejecucionesRestantesB != 0){
                    recursos.wait()
                    consumir 1 recurso
                    ejecucionesRestantesB--;
                }else{
                    ejecucionesRestantesB = 2;
                    consumidores[1].signal();
                }

                //Le avisamos a A que consumimos los recursos
                consumidos.signal()


        Consumidor C
            mientras verdadero:
                consumidores[1].wait()
                
                recursos.wait()
                recursos.wait()
                consumir 2 recursos
                
                //Le avisamos a A que consumimos los recursos
                consumidos.signal()
                consumidos.signal()

                consumidores[0].signal() //turno de B

    ```

## Ejercicio 9
Suponer que se tienen $N$ procesos $P_i$, cada uno de los cuales ejecuta un conjunto de sentencias $a_i$ y $b_i$. 

¿Cómo se pueden sincronizar estos procesos de manera tal que los $b_i$ se ejecuten después de que se hayan ejecutado todos los $a_i$ ?

**Respuesta**: Necesitamos tener una especie de barrera que bloquee a todos. El último que ejecute $a_i$ debe enviar la señal (N señales) al resto para que comiencen a ejecutar $b_i$. Cada proceso consumiría una $b_i$. 

Notar que enviamos $N$ señales desde el último que ejecuta $a_i$ *(necesitamos tener un mutex sobre el contador)* pues el mismo va a bloquearse, pero consumir el mismo permiso que otorgó.

```text
    semaforo(barrera) = 0 
    semáforo(mutex) = 1
    volatile count = 0; 

    Proceso i: 
        Ejecutar a_i

        mutex.wait()
        count++;

        if(count == N){
            for(int i = 0; i<N; i++){
                barrera.signal();
            }
        }
        
        mutex.signal();
        
        barrera.wait()

        Ejecutar b_i 

```

## Ejercicio 10
Se tienen los siguientes dos procesos, $foo$ y $bar$, que son ejecutados concurrentemente. Además comparten los semáforos $S$ y $R$, ambos inicializados en $1$, y una variable global $x$, inicializada en $0$.

```text
void foo( ) {
    do {
        semWait(S);
        semWait(R);
        x++;
        semSignal(S);
        semSignal(R);
    } while (1);
}

void bar( ) {
    do {
        semWait(R);
        semWait(S);
        x--;
        semSignal(S);
        semSignal(R);
    } while (1);
}
```

a) ¿Puede alguna ejecución de estos procesos terminar en *deadlock*? En caso afirmativo, describir una traza.

b) ¿Puede alguna ejecución generar *starvation* para alguno de los procesos? En caso afirmativo, describir una traza.

**Respuesta**: 

a) sí, puede terminar en deadlock. Un deadlock se produce cuando tenemos un ciclo dentro de un grafo, o, para mí mejor dicho, dependencia circular entre componentes donde uno está esperando al otro que haga algo pero no arranca hasta que el otro lo haga.

Similar a cuando dos personas enamoradas no se hablan hasta que el otro le hable.

Traza

```text
    - [foo] semWait(S); consume un permiso (S = 1 -> S = 0).
    - [bar] semWait(R); consume un permiso (R = 1 -> R = 0).
    - [foo] semWait(R); no avanza (R = 0).
    - [bar] semWait(S); no avanza (S = 0).
        
    ¡deadlock! 

```

Ya con que **cada uno avance un solo paso** tenemos el deadlock, porque básicamente en el próximo paso que les toca, se empiezan a esperar uno al otro.

b) **Preguntar**: Creo que solo habría starvation si da la casualidad que el scheduler le da siempre el permiso a uno mismo entre que termina de hacer los $semSignal()$ y los $semWait()$. Después no veo otra. **Respuesta**: sí, correcto

## Ejercicio 11
Se quiere simular la comunicación mediante pipes entre dos procesos mediante las syscalls read()
y write(), pero usando memoria compartida (sin usar file descriptors). 

Se puede pensar al pipe
como un buffer de tamaño N, donde en cada posición se le puede escribir un cierto mensaje. 

El read() debe ser bloqueante en caso que no haya ningún mensaje, y si el buffer está lleno, el write() también
debe ser bloqueante. 

No puede haber condiciones de carrera y se puede suponer que el buffer tiene los siguientes métodos: pop() (saca el mensaje y lo desencola), push() (agrega un mensaje al buffer).

**Respuesta**: lo importante de acá es que el read() o write() son bloqueantes. Esto quiere decir que si el buffer está vacío, el read() se queda esperando a que haya datos, y si el buffer está lleno, el write() se queda esperando a que se libere espacio.

Además, para evitar que tanto el productor como el consumidor modifiquen la memoria compartida a la vez (evitando condiciones de carrera), debemos usar un mutex que garantice exclusión mutua.

El productor va a poder producir tantos recursos como quiera y almacenarlos en el buffer siempre que haya espacio. Si hay espacio, puede producir. Así en loop. Para denotar el espacio en el buffer podemos usar un semáforo contador que indique la cantidad de espacio libre disponible (que bloquee al escritor si llega a 0).

El consumidor va a poder leer siempre y cuando haya algo en el buffer. Para denotar la cantidad de mensajes que hay en el buffer podemos usar otro semáforo contador que indique los mensajes disponibles (que bloquee al lector si llega a 0).

El orden en que se ejecutan el productor y consumidor no podemos conocerlo de antemano pues al ser concurrentes, su ejecución es no determinística.

Pero lo que sí debe cumplirse es que:

- El productor no genera más si el buffer está lleno.
- El consumidor no hace nada si no hay nada en el buffer.

```text

    semáforo(espacios) = N
    semáforo(mensajes) = 0
    semáforo(mutex) = 1

    Productor:
        repetir siempre:
            espacios.wait(); //si está en 0 no hay espacio
            mutex.wait();
            
            buffer.push(mensaje)

            mutex.signal();
            mensajes.signal();

    Consumidor:
        repetir siempre:
            mensajes.wait();
            mutex.wait();

            mensaje = buffer.pop()

            mutex.signal();
            espacios.signal();

```

## Ejercicio 12
Un grupo de $N$ estudiantes se dispone a hacer un TP de su materia favorita (Sistemas Operativos).
Cada estudiante conoce a la perfección cómo `implementarTp()` y cómo `experimentar()`. 

Curiosamente, cada una de estas acciones puede ser llevada acabo de manera independiente por cada uno,
así que decidieron dividirse el trabajo.

Acordaron dividir el trabajo en varias etapas. En cada etapa, todos los estudiantes deben primero
`implementarTp()`, y recién cuando todos hayan terminado, pueden empezar a `experimentar()`. 

Luego, para poder comenzar la siguiente etapa y volver a implementar, todos deben haber terminado de
experimentar con la etapa anterior.

Se pide diseñar un programa concurrente que utilice procesos y que modele esta situación utilizando
semáforos.

**Respuesta**: es similar al ejercicio 9. La diferencia es que acá hay dos barreras: una para hacer `implementarTp()` y otra para `experimentar()`.

Primero hagamos el ciclo de implementarTp y experimentar() por única vez.

```text

    semáforo(barrera) = 0; 
    semáforo(mutex) = 1;
    volatile count = 0;

    Estudiante j:
        implementarTP()

        mutex.wait();
        count++;

        if(count == N){
            for(int i = 0; i<N; i++){
                barrera.signal();
            }
        }

        mutex.signal();

        barrera.wait();
        
        experimentar();
```

Ahora agreguemos el flujo de que podemos hacer esto N veces. Necesitamos que todos los que terminaron de experimentar() se queden colgados esperando que les den el "avancen a implementar". *(ojo con la primera vez, deberíamos inicializar con el permiso total para que todos puedan ejecutar)*

Solo el último que hizo `experimentar()` debería enviar N señales para que empiecen a implementar de vuelta.

```text

    semáforo mutex1 = 1, mutex2 = 1;
    semáforo barrera1 = 0, barrera2 = 0;
    volatile int count1 = 0, count2 = 0;

    Estudiante j:
        while (true) {
            implementarTp();

            mutex1.wait();
            count1++;

            if (count1 == N) {
                count2 = 0;
                for (int i = 0; i < N; i++) {
                    barrera1.signal();
                }
            }
            mutex1.signal();

            barrera1.wait();

            experimentar(); 

            mutex2.wait();
            count2++;
            if (count2 == N) {
                count1 = 0; 
                for (int i = 0; i < N; i++) {
                    barrera2.signal();
                }
            }
            mutex2.signal();

            barrera2.wait();
        }
```

## Ejercicio 13
