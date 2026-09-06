# Guía 2: Scheduling

## Ejercicio 1

La siguiente secuencia describe la forma en la que un proceso utiliza el procesador

| Tiempo | Evento                |
| -----: | --------------------- |
|      0 | `load` → `store`      |
|      1 | `add` → `store`       |
|      2 | `read` de archivo     |
|      3 | Espera E/S            |
|      … | …                     |
|     10 | Espera E/S            |
|     11 | `store` → `increment` |
|     12 | `inc`                 |
|     13 | `write` en archivo    |
|     14 | Espera E/S            |
|      … | …                     |
|     20 | Espera E/S            |
|     21 | `load` → `store`      |
|     22 | `add` → `store`       |

a) Identificar las ráfagas de CPU y las ráfagas de E/S
b) ¿Qué duración tiene cada ráfaga? 

**Respuesta**:

Antes de todo, ¿qué es una ráfaga? Una ráfaga es un período continuo de tiempo durante el cual un proceso está haciendo una misma clase de actividad sin interrupción.

Ahora sí, este proceso tiene mucho más I/O que CPU. 

Ráfagas de CPU
- [0, 3): load store, add store, read de archivo. Duración: 3 unidades de tiempo.
- [11, 14): store increment, inc, write en archivo. Duración: 3 unidades de tiempo.
- [21, 23): load store, add store. Duración: 2 unidades de tiempo.

Ráfagas de I/O: 
- [3, 11): Duración: 8 unidades de tiempo.
- [14, 21): Duración: 7 unidades de tiempo.

**Pregunta**: ¿usamos alguna medida particular, o hablamos solamente de n unidades de tiempo?

## Ejercicio 2

Sean P0, P1 y P2 tales que

* P0 tiene ráfagas cortas de I/O a ciertos dispositivos.
* P1 frecuentemente se bloquea leyendo de la red.
* P2 tiene ráfagas prolongadas de alto consumo de CPU y luego de escritura a disco.

Para planificar estos procesos ¿convendría usar un algoritmo de Round Robin? ¿convendría usar uno de prioridades? Justifique su respuesta.

**Respuesta**: empecemos recordando qué es Round Robin.

Round Robin es un algoritmo de **scheduling preemptive**. Esto quiere decir que el **scheduler** le da un **quantum** de tiempo a cada proceso y, una vez terminado ese quantum, si el proceso todavía necesita utilizar la CPU, se la quita y se la da a otro proceso.

Es un algoritmo equitativo, porque intenta repartir el tiempo de CPU entre los procesos de manera justa. Es especialmente útil en sistemas donde pueden existir distintos tipos de procesos y donde es importante tener un buen **tiempo de respuesta**, especialmente para procesos interactivos.

Cada vez que un proceso pasa a estar **ready**, se coloca al final de la **ready queue**.

Una de las cosas más importantes de Round Robin es elegir un buen quantum. ¿Por qué?

1. **Si elegimos un quantum muy chico:** tendremos muchos **context switches**, lo que genera overhead y hace que aprovechemos menos la CPU para ejecutar trabajo útil.

2. **Si elegimos un quantum muy grande:** Round Robin comienza a comportarse de manera similar a FCFS, ya que cada proceso puede mantener la CPU durante períodos prolongados antes de que otro proceso tenga la oportunidad de ejecutarse. Esto empeora el tiempo de respuesta de los procesos que están esperando.

Un detalle importante es que **el quantum representa el tiempo máximo que un proceso puede utilizar la CPU de manera continua, pero no significa que conserve la CPU aunque se bloquee por I/O**.

Por ejemplo, si tenemos un **quantum de 4** y P2 está ejecutando:

```text
t=0 → P2 empieza a ejecutar

t=1 → P2 ejecuta

t=2 → P2 ejecuta

t=3 → P2 ejecuta

t=4 → termina su quantum
```

En ese momento, el scheduler busca otro proceso que esté **ready**. Pero supongamos que P0 y P1 están bloqueados realizando I/O:

```text
P0 → BLOCKED (I/O)

P1 → BLOCKED (I/O)

P2 → READY
```

Entonces **P2 vuelve a recibir la CPU**, aunque haya terminado su quantum, porque es el **único proceso disponible para ejecutar**.

Podría suceder:

```text
CPU:  P2 | P2 | P2 | P2 | P2 | P0 | P1 | ...

       ↑    ↑    ↑    ↑

       quantum = 4
```

Cada vez que P2 termina su quantum, si P0 y P1 continúan bloqueados por I/O y P2 es el único proceso **ready**, el scheduler puede volver a darle la CPU a P2. Esto continúa hasta que P0 o P1 terminan su operación de I/O y vuelven a estar **ready**.

Ahora sí, planteemos qué nos convendría en este caso.

* **P0 tiene ráfagas cortas de I/O a ciertos dispositivos:** esto nos dice que P0 pasa frecuentemente a realizar operaciones de I/O y, mientras está bloqueado esperando que estas terminen, **no necesita utilizar la CPU**. Por lo tanto, otro proceso puede aprovechar la CPU durante ese tiempo.

* **P1 frecuentemente se bloquea leyendo de la red:** la red también es una operación de I/O. Por lo tanto, P1 frecuentemente pasa de estar ejecutándose a quedar bloqueado esperando datos. Cuando esto sucede, otro proceso puede utilizar la CPU.

* **P2 tiene ráfagas prolongadas de alto consumo de CPU y luego escritura a disco:** acá tenemos dos cosas:

  1. Tiene **ráfagas prolongadas de CPU**, por lo que necesita utilizar la CPU durante períodos relativamente largos.

  2. Luego realiza una **escritura a disco**, que es una operación de I/O, por lo que también puede quedar bloqueado mientras espera que termine.

Podemos entonces considerar a **P2 como un proceso principalmente CPU-bound**, mientras que **P0 y P1 son procesos más I/O-bound**.

### ¿Round Robin sería una buena opción?

Sí. De hecho, **Round Robin es una opción razonable para este escenario**.

P0 y P1, al tener frecuentes operaciones de I/O, suelen bloquearse. Mientras ellos están bloqueados, P2 puede aprovechar la CPU. Cuando P0 o P1 terminan su operación de I/O y vuelven a estar **ready**, Round Robin les permite obtener la CPU relativamente rápido, lo que favorece su tiempo de respuesta.

P2, aunque tenga ráfagas largas de CPU, también puede avanzar. Si su quantum termina antes de que termine su ráfaga de CPU, será desalojado. Sin embargo, **si P0 y P1 están bloqueados por I/O y P2 es el único proceso ready, P2 puede volver a recibir la CPU inmediatamente**.

Por lo tanto, **no es necesario encontrar un quantum que coincida con las ráfagas de los tres procesos**. El quantum debe elegirse buscando un equilibrio entre un buen tiempo de respuesta y una cantidad razonable de **context switches**.

### ¿Y un algoritmo de prioridades?

También podría utilizarse, pero **no parece necesario en este caso**.

Podríamos asignar una prioridad mayor a P0 y P1, ya que son procesos I/O-bound y pueden beneficiarse de obtener la CPU rápidamente cuando terminan sus operaciones de I/O. P2 podría tener una prioridad menor, ya que posee ráfagas prolongadas de CPU.

Sin embargo, esto introduce el problema de **starvation**. Si P0 y P1 tienen siempre mayor prioridad y frecuentemente vuelven a estar **ready**, P2 podría quedar esperando durante demasiado tiempo.

Para evitar esto habría que utilizar algún mecanismo adicional, como **aging**, que aumente progresivamente la prioridad de los procesos que llevan mucho tiempo esperando.

Además, **Round Robin ya resuelve de manera natural el problema que tenemos en este escenario**: cuando P0 y P1 están bloqueados haciendo I/O, P2 puede aprovechar la CPU; y cuando P0 o P1 vuelven a estar **ready**, pueden obtener la CPU rápidamente gracias al reparto mediante quantums.

Por lo tanto, **no necesitamos introducir prioridades para que los procesos I/O-bound tengan oportunidades de ejecutar**. Su propio comportamiento de bloqueo por I/O hace que liberen la CPU, y Round Robin se encarga de repartirla cuando vuelven a estar disponibles.

### Conclusión

El algoritmo que parece más adecuado es **Round Robin**, utilizando un quantum razonable.

La idea es que **cuando P0 y P1 estén bloqueados haciendo I/O, P2 pueda aprovechar la CPU para avanzar en sus largas ráfagas de CPU**. Si P2 termina su quantum mientras P0 y P1 siguen bloqueados, **P2 puede continuar ejecutando porque es el único proceso ready**.

Cuando P0 o P1 se desbloqueen y vuelvan a estar **ready**, Round Robin les permite obtener la CPU rápidamente, favoreciendo su tiempo de respuesta.

Un algoritmo de prioridades también podría funcionar, pero **no aporta una ventaja necesaria en este escenario y agrega el riesgo de starvation de P2**, que obligaría a utilizar mecanismos como aging.

Por lo tanto, **Round Robin alcanza y es una alternativa simple y adecuada para estos tres procesos**.
