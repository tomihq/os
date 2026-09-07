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

## Ejercicio 3
¿A qué tipo de scheduler corresponde el siguiente diagrama de transición de estados de un proceso?

**Respuesta**: a uno non-preemptive. Esto es fácil de notar porque no existe ninguna flecha de **running** a **ready** (desalojo).
El proceso solo soltaría la CPU sí va a **blocked** (I/O) o **terminated** (terminó)

## Ejercicio 4
¿Cuáles de los siguientes algoritmos de *scheduling* pueden resultar en *starvation* y en qué condiciones?

- Round-robin
- Por prioridad
- SJF (Shortest-Job-First)
- SRTF (Shortest-Remaining-Time-First)
- FCFS (First-Come-First-Serve)
- MultiLevel Queue
- MultiLevel Feedback Queue

**Respuesta**

Primero recordemos qué es **starvation** y por qué es importante saberlo.

Starvation es: "nunca vas a llegar a tocar un proceso", lo cual es diferente de: "quizá tarda mucho en llegar, pero llega".

- Round-robin: no aparece. Se hace la ronda de los procesos con un quantum para cada proceso. Mientras un proceso esté READY, eventualmente tendrá su turno, por lo que no hay starvation por parte del algoritmo.
- Por prioridad: sí, puede aparecer starvation. La condición es que haya procesos con muchísima prioridad que sigan siendo elegidos antes que los de menor prioridad. Si continúan llegando procesos de alta prioridad, los de menor prioridad pueden quedar sin ejecutarse indefinidamente.
- SJF (Shortest-Job-First): sí, puede aparecer starvation. Si siempre entran procesos más cortos, el proceso largo puede quedar eternamente postergado. Por eso, SJF necesita conocer o estimar la duración de la próxima CPU burst para poder decidir cuál es el proceso más corto.
- SRTF (Shortest-Remaining-Time-First): sí, puede aparecer starvation. Si continuamente llegan procesos cuyo tiempo restante es menor que el del proceso largo, este puede quedar eternamente postergado. Además, al ser preemptive, un proceso que ya está ejecutando puede ser desalojado cuando llega otro con menor tiempo restante.
- FCFS (First-Come-First-Serve): no aparece por la política de scheduling. Como los procesos se atienden en orden de llegada, un proceso READY no puede ser continuamente salteado por procesos que llegan después. Puede tener un tiempo de espera enorme si hay un proceso muy largo adelante, pero si ese proceso eventualmente libera la CPU, el siguiente ejecutará.
- MultiLevel Queue: sí, puede aparecer starvation. Por ejemplo, si los procesos real-time tienen mayor prioridad y continuamente hay procesos real-time listos para ejecutarse, los procesos batch pueden quedar esperando indefinidamente y no llegar a ejecutarse nunca. Además, como la prioridad es estática, los procesos no pueden cambiar de cola como ocurre en Multilevel Feedback Queue, por lo que un proceso batch no puede aumentar su prioridad mediante Aging para pasar a una cola superior.
- MultiLevel Feedback Queue: sí, puede aparecer starvation, pero se puede reducir o evitar mediante Aging. Los procesos que permanecen mucho tiempo esperando en una cola de baja prioridad pueden aumentar progresivamente su prioridad y pasar a una cola de mayor prioridad, permitiendo que eventualmente sean ejecutados.

**Preguntar**: ¿está bien lo de multilevel queue? porque YO entendí que si tenés una queue real-time con máxima prioridad, si te entra algún batch, pero tenés 9999999999 real-time *(1 proceso nuevo READY por segundo)* y 1 batch, el batch no lo ejecutás nunca hasta que vacías los real-time.
Capaz SEGURO que existe una forma de decir: "ok, tomás alguno de otra queue y después seguís con la otra" pero yo interpreto que acá hasta que no vacías la máxima prioridad, no pasás a la otra.

## Ejercicio 5
Considere una modificación a *round-robin* en la que un mismo proceso puede estar encolado varias veces en la lista de procesos *ready*. Por ejemplo, en un RR normal se tendrían en la cola ready a P1, P2, P3, P4. Con esta modificación se podría tener P1, P1, P2, P1, P3, P1, P4.

a) ¿Qué impacto tendría esta modificación?

b) Dar ventajas y desventajas de este esquema. Piense en el efecto logrado.

c) ¿Se le ocurre alguna otra modificación para mantener las ventajas sin tener que duplicar las entradas en la lista de procesos *ready*?

**Respuesta**

### a) ¿Qué impacto tendría esta modificación?

El impacto principal es que un proceso podría aparecer varias veces en la cola `ready`, por lo que tendría **más oportunidades de obtener la CPU** que los demás procesos.

Por ejemplo:

```text
RR normal:
P1 → P2 → P3 → P4 → P1 → P2 → P3 → P4

RR modificado:
P1 → P1 → P2 → P1 → P3 → P1 → P4
```

En este caso, **P1 recibe la CPU con mucha mayor frecuencia** que los demás procesos.

### b) Ventajas y desventajas

**Ventajas**

* Permite darle a determinados procesos **más oportunidades de utilizar la CPU**.
* Puede utilizarse para favorecer procesos que necesitan una respuesta más rápida o que se consideran más importantes.

**Desventajas**

* Si tenemos un proceso repetido **n veces** de manera continua, los demás procesos pueden quedar esperando demasiado tiempo.
* En un caso extremo, el comportamiento podría parecerse a un **FCFS con un proceso que tiene una ráfaga de CPU extremadamente larga**, perjudicando especialmente a los procesos interactivos.

### c) ¿Otra modificación sin duplicar entradas?

Sí. Se podría asignar a cada proceso una **mayor prioridad** o un **quantum diferente**.

Por ejemplo, un proceso más importante podría tener un quantum mayor, permitiéndole utilizar la CPU durante más tiempo sin necesidad de que aparezca varias veces en la cola.

Otra alternativa sería utilizar **Round Robin con prioridades**, donde los procesos de mayor prioridad reciben la CPU antes que los de menor prioridad.

## Ejercicio 6
Considerar el siguiente conjunto de procesos:
---------------------------------------
| Proceso | Ráfaga de CPU | Prioridad |
| ------- | ------------: | --------: |
| P1      |            10 |         3 |
| P2      |             1 |         1 |
| P3      |             2 |         3 |
| P4      |             1 |         4 |
| P5      |             5 |         2 |
---------------------------------------

Se supone que los procesos llegan en el orden P1, P2, P3, P4, P5 en el instante 0.

1.  Dibujar los diagramas de Gantt para ilustrar la ejecución de estos procesos usando los algoritmos de scheduling: FCFS, SJF, prioridades sin desalojo, round-robin (quantum de 1 unidad de tiempo, ordenados por el número de proceso)
2. ¿Cuál es el waiting time promedio y de turnaround para cada algoritmo?
3. ¿Cuál es de los algoritmos obtiene el menor waiting time promedio, y el menor turn around? 

**Respuesta**

a) 
![FCFS](fcfs.png)
![SJF](sjf.png)
![Con Prioridades](pp.png)
![Round Robin](rr.png)

b)

Preguntar. No lo entendí bien. 

1) Creo que los cálculos están bien.
2) Creo que "con prioridades" tiene dos posibles ejecuciones.

3. FCFS
   1. Waiting Time Promedio: (0+10+11+13+14)/5 = 9.6
   2. Turn Around Promedio: (10+11+13+14+19)/5 = 13.4
4. SJF
   1. Waiting Time Promedio: (0+1+2+4+9)/5 = 3.2
   2. Turn Around Promedio: (1+2+4+9+19)/5 = 7
5. Con Prioridades (1)
   1. Waiting Time Promedio: (0+1+6+8+18)/5 = 6.6
   2. Turn Around Promedio: (1+6+8+18+19)/5 = 10.4  
6. Con Prioridades (2)
   1. Waiting Time Promedio: (0+1+6+16+18)/5 = 8.2
   2. Turn Around Promedio: (1+6+16+18+19)/5 = 12
7. Round Robin
   1. Waiting Time Promedio: (10+1+5+3+9)/5 = 5.6
   2. Turn Around Promedio: (19+2+7+4+15)/5 = 9.4

c) SJF. SJF.

## Ejercicio 7

## Ejercicio 8
