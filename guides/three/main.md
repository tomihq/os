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