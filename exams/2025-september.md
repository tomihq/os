## Ejercicio 1

**Respuesta**:

b) Usaría el arreglo $m$ como variable compartida y un **mutex**. El padre debería recibir una señal por cada hijo que termina, para notificarle a los hijos qué tiene que procesar el que envió la señal. El padre recorrería el array y se fijaría que índice no fue llenado, y le diría al hijo: "hacé este índice ahora", y el hijo cuando termina, pone el valor en el array y envía la señal.  


## Ejercicio 2
Tenemos un sistema donde existen dos tipos de procesos:

- Proceso Tipo 1: Recibe una solicitud de un usuario y agrega un trabajo a una cola de tareas. Esta cola tiene un tamaño máximo N, ya que no puede tener muchas tareas esperando en la memoria.
- Proceso Tipo 2: Toma trabajos de la cola (usando pop()) y realiza un trabajo de procesamiento.

Si la cola se llena, no se le deben agregar nuevos trabajos. La cola de trabajos no es atómica. La interacción de cada proceso con la cola se implementó hace poco y su comportamiento se puede representar con el siguiente pseudocódigo.

```text
// Variables compartidas
mutex = sem_init(1);
jobs = sem_init(0);
buffer cola(N); //buffer de tamaño N

//Proceso tipo 1
trabajo = esperarPromptUsuario();
mutex.wait();
cola.add(trabajo);
mutex.signal();
jobs.signal();

//Proceso tipo 2
mutex.wait();
jobs.wait();
trabajo = cola.pop();
mutex.signal();
enviarResultados(trabajo);
```

Sin embargo, se reportó que esta parte del código no está funcionando correctamente y que no cumple exactamente con lo pedido. Nos encargaron la tarea de identificar qué está ocurriendo. Se pide:

a) Encontrar qué problema tiene este código con respecto a la sincronización de los procesos. Justificar detalladamente la/s causa/s y corregirlo, cumpliendo con la consigna del problema

b) Ahora se desea que $k$ procesos Tipo 2 (siendo k mucho menor que N) trabajen concurrentemente, pero sólo pueden enviar un resultado cuando k procesos hayan extraído una tarea de la cola. Es decir, ningún proceso puede enviar el resultado inmediatamente después de retirar el trabajo de la cola, debe esperar a los otros procesos. Extender la solución del inciso anterior con esta restricción, asegurando que no haya problemas de concurrencia. Justificar la solución

**Respuesta**:

a) Hay dos.
- Si arranca el proceso 2 (jobs = 0, mutex = 1, buffer vacío) y hace `mutex.wait()`, se queda colgado para siempre en `jobs.wait()` ya que el proceso tipo 1 no puede agregar ningún trabajo a la cola pues `mutex.wait()` no se satisface nunca. Deadlock. 
    - La solución para 1) es que el proceso tipo 2 no se pelee con el tipo 1 inicialmente. El proceso 2 tiene que ESPERAR a que haya trabajos primero.
- Si siempre ejecuta el proceso de tipo 1 puede pasar que se desborde el buffer. 
    - La solución para 2) es tener un semáforo de longitud N. El proceso de tipo 1 empieza a ejecutar si hay alguno que está esperando. 

```text
// Variables compartidas
mutex = sem_init(1);
jobs = sem_init(0);
buffer cola(N); //buffer de tamaño N
slots_disponibles = sem_init(N);

//Proceso tipo 1
trabajo = esperarPromptUsuario();
slots_disponibles.wait()
mutex.wait();
cola.add(trabajo);
mutex.signal();
jobs.signal();

//Proceso tipo 2
jobs.wait();
mutex.wait();
trabajo = cola.pop();
mutex.signal();
slots_disponibles.signal();
enviarResultados(trabajo);
```

b) Hay que usar una barrera. Cada proceso va a tener que quedarse colgado si todavía falta alguno por terminar. Como son concurrentes, cada uno va a tener que ir tomando los trabajos que haya disponibles. El que haya tomado el último trabajo y terminado (el k), mandará una señal al resto para que se desbloqueen y volverán a esperar a que haya trabajos disponibles.

```text
// Variables compartidas
mutex = sem_init(1);
jobs = sem_init(0);
buffer cola(N); //buffer de tamaño N
slots_disponibles = sem_init(N);

//variables nuevas
barrera = sem_init(0);
volatile int count = 0; 


//Proceso tipo 1
trabajo = esperarPromptUsuario();
slots_disponibles.wait()
mutex.wait();
cola.add(trabajo);
mutex.signal();
jobs.signal();

//Proceso tipo 2
jobs.wait();
mutex.wait();
trabajo = cola.pop();
count++; 
if(count == k){
    for(int i = 0; i<k; i++){
        barrera.signal();
    }
    count = 0; 
}
mutex.signal();
slots_disponibles.signal();
barrera.wait();
enviarResultados(trabajo);
```

## Ejercicio 3 
Se quiere avanzar en el diseño de un sistema de control para un dron autónomo de rescate. Este sistema debe gestionar múltiples tareas simultáneamente para garantizar un funcionamiento eficiente del dron. El dron debe ser capaz de ejecutar tareas con periocidad fija y *deadlines* estrictos (se garantiza que siempre se pueden cumplir), tales como el escaneo de obstáculos y la estabilización de vuelo.

Además, el sistema debe procesar *logs* en segundo plano sin afectar el rendimiento general del sistema. También, debe ser capaz de gestionar tareas disparadas remotamente por el operador, las cuales pueden tener diferentes niveles de prioridad (urgente, media, baja), permitiendo al operador priorizar ciertas tareas según la urgencia.

a) Proponer un esquema de *scheduling* para este sistema que tenga en cuenta todas las restricciones y requerimientos. Justificar detalladamente cada decisión.

b) La siguiente tabla representa el comportamiento de las tareas periódicas con *deadlines* específicos. Dibujar el diagrama de Gantt resultante de aplicar el esquema escrito en el inciso anterior, desde el momento 0 hasta los 25ms.

**Respuesta**:

a) Tenemos tres tipos de tareas
- Tareas con periocidad fija y **deadlines estrictos**
- *Logs* en segundo plano. 
- Tareas disparadas remotamente por el operador que pueden tener diferente prioridad.

Lo importante acá es que la prioridad NÚMERO 1 son las tareas que tienen un deadline. ¿Por qué? porque el deadline es estricto. Si no lo cumplimos pueden pasar cosas graves.

Para esto podemos usar un algoritmo con desalojo que ejecute el que tiene el deadline más próximo. Si estamos ejecutando una que tiene el deadline en 5s y cae una que lo tiene en 1s, tenemos que priorizar la de 1s, por eso el desalojo.

Las tareas disparadas remotamente por el operador no se especifica si tienen deadline, así que podemos usar una prioridad NÚMERO 2, y desempatar en esta misma cola por PRIORIDAD. Es decir, si el operador envía BAJA y URGENTE necesitamos sí o sí desalojar la BAJA y ejecutar la URGENTE así que va a ser con desalojo. Acá podemos usar un Round Robin con prioridades. **Notar que asumo que las de deadline son la prioridad máxima. El operador deberá esperar si hay otras cosas con deadline más prioritarias**  

¿Podría pasar que estas queden en starvation? podría suceder si tenemos constantemente eventos con deadlines, pero como mencionamos anteriormente, no podemos permitir que no se ejecuten en ese deadline.

Las tareas de logs pueden tranquilamente estar en una queue de la menor prioridad posible, y usar un sistema de aging para que no queden en starvation infinitamente. Podemos usar un Round Robin sin ningún problema.

Lo que sí debemos garantizar siempre es que: tareas con deadline > tareas por operador (se organizan por urgencia y hay desalojo si llega una con mayor urgencia) > sistema de logs. 