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

a) Analicemos cada programa por separado.

El Programa 1 tiene dos procesos que se ejecutan concurrentemente. Eso quiere decir que no tenemos la garantía de saber quién va a ejecutar y por qué. Cuando tenemos este tipo de problema, tenemos que prestar atención a los recursos compartidos, porque puede existir una condición de carrera: el resultado depende del orden en que se intercalen las operaciones de los procesos.

Tanto el proceso A y B utilizan una variable compartida x. Eso quiere decir que el recurso el cual comparten aquí es esa misma variable. El problema aparece porque ambos procesos realizan una operación de lectura-modificación-escritura sobre la misma variable compartida.

La operación que hacen ambos, no es atómica. Como x = x + 1 no es atómica, el proceso puede ser desalojado entre la lectura y la escritura de x, permitiendo que el otro proceso ejecute sobre la misma variable.

Como estamos en memoria compartida, podemos asumir que **cualquier operación que utiliza x, va a ir a buscar el valor EN EL MOMENTO a la memoria**.

Veamos algunas posibles ejecuciones.

Ejecución posible (1):

x = 0;

1. [A] busca el valor de x. Obtiene 0.
2. [B] busca el valor de x. Obtiene 0.
3. [A] muta el valor de x. x = 1.
4. [B] muta el valor de x. x = 1.

Notar que acá ya tuvimos un problema, porque si la variable es compartida, B ya había leído x = 0 como parte de su operación x = x + 1. Aunque A modifique posteriormente la variable compartida, B puede continuar utilizando ese 0 que había obtenido y finalmente escribir 1.

5. [A] muestra el valor: 1.
6. [B] termina.
7. [A] termina.

Notar que acá [A] termina mostrando el valor de 1! Nosotros esperaríamos 2! 

El problema que hubo es que los modificaron la variable luego de haber leído el valor (0) en la operación inicial.

Ejecución posible (2):

x = 0; 

1. [A] busca el valor de x. Obtiene 0.
2. [A] muta el valor de x. x = 1.
3. [B] busca el valor de x. Obtiene 1.
4. [B] muta el valor de x. x = 2.
5. [A] muestra el valor: 2.
6. [B] termina. 
7. [A] termina.

¡Notemos que acá sí hubo algo interesante!
1. En este caso, x termina valiendo 2 porque cada proceso completa su operación de lectura-modificación-escritura antes de que el otro comience la suya.
2. Como le dimos el tiempo para hacer la operación de (READ + WRITE) todo salió como esperábamos

Ejecución posible (3):

x = 0;

1. [B] busca el valor de x. Obtiene 0.
2. [A] busca el valor de x. Obtiene 0.
3. [B] muta el valor de x. x = 1.
4. [A] muta el valor de x. x = 1.
5. [B] termina
6. [A] muestra el valor: 1.
7. [A] termina

Notar que acá estamos en el mismo problema que en la ejecución posible 1! 

Ejecución posible (4):

x = 0;
1. [B] busca el valor de x. Obtiene 0.
2. [B] muta el valor de x. x = 1.
3. [A] buscas el valor de x. Obtiene 1.
4. [A] muta el valor de x. x = 2.
5. [B] termina
6. [A] muestra el valor: 2.
7. [A] termina

¡Notar que acá tuvimos el mismo resultado que la ejecución 2! ¿Qué sucedió?

Le dimos el tiempo de hacer el READ + WRITE de un recurso. Es decir, el otro proceso no ejecutó hasta que la operación se hizo completa.

Esto muestra que el resultado depende de cómo se intercalen las operaciones de los procesos. Si queremos garantizar que x = x + 1 se ejecute completa antes de que el otro proceso acceda a x, necesitamos sincronización.

La introducción a esto es: usar secciones críticas con mutex para evitar que otro proceso no haga nada con un recurso si otro lo tiene tomado. 

