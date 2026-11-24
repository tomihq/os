# Touch Screen Driver
Se desea implementar el driver para una pantalla táctil. 

Un programa de usuario podrá comunicarse con este driver para conocer los eventos de toque que ocurran en la pantalla, y así identificar distintos gestos. 

Un gesto comienza cuando se toca la pantalla táctil, dura todo el tiempo que se mantiene el contacto, y termina al dejar de tocar.

El dispositivo genera una interrupción `IRQ_TOUCH` cuando detecta un toque en la pantalla (al iniciar un gesto) y actualiza automáticamente los registros `TCH_COORD` con un entero que representa las coordenadas (x,y) del contacto, `TCH_STATUS` con el valor `TCH_PRESS` y `TCH_TIME` con el valor de 0. Cuando el gesto termina, se actualizan *muy rápidamente* los registros `TCH_TIME` con un valor numérico que representa la cantidad de milisegundos que duró el gesto y `TCH_STATUS` con el valor `TCH_UP`. Mientras dura el gesto, el registro `TCH_COORD` es actualizado con los cambios de ubicación del toque y `TCH_TIME` es actualizado con la cantidad de tiempo transcurrido desde que se inició el gesto.

Cuando una aplicación haga uso del input de la pantalla táctil, deberá poder identificar si se trata de un click, un deslizamiento, o una pulsación prolongada. El driver deberá suministrar la información necesaria para ello.

Además, la pantalla permite colorear los píxeles utilizando los registros `PRINT_COORD` para la coordenada del píxel, `PRINT_COLOR` para el color a utilizar y `PRINT_OK` para efectuar el coloreo al escribir el valor `PRINT`.

a) Escribir las funciones necesarias del driver para lograr el funcionamiento pedido. Tener en cuenta que la lectura debe ser bloqueante mientras no esté ocurriendo ningún gesto.
b) Escribir el pseudocódigo de un programa de usuario que se comunique con el driver para detectar los siguientes patrones de gestos:
- Click: se detecta un único punto de contacto sin movimiento y con una duración muy corta (menos de 500ms)
- Deslizamiento: se detecta un punto de contacto inicial y un movimiento continuo (sin soltar)
- Pulsación prolongada: se detecta un único punto de contacto sin movimiento y con una duración mayor al click.

Al detectar un click, se debe colorear el pixel tocado en color AZUL. Si se trata de una pulsación prolongada, será de VERDE. En caso de que sea un desplazamiento, cada punto detectado debe ser coloreado de ROJO.

Se cuenta con las funciones `int getX(int coord)` y `int getY(int coord)` que permiten obtener las coordenadas $x$ e $y$ respectivamente a partir del valor que se encuentra en `TCH_COORD`. A su vez, la función `int genCoord(int x, int y)` toma dos coordenadas y genera el valor necesario para el registro `PRINT_COORD`.

## Solución
Empecemos organizando la información que tenemos que es mucha.
Registros:
    - TCH_COORD: entero que representa las coordenadas (x,y) del contacto.
        - Cuando inicia el gesto, pone las coordenadas del contacto. 
        - Mientras dura el gesto, es actualizado con los cambios de ubicación del toque. 
        - Cuando el gesto termina no dice nada. 
    - TCH_STATUS: es el estado del touch screen
        - Cuando inicia el gesto se coloca el valor `TCH_PRESS`
        - Cuando termina el gesto se coloca el valor `TCH_UP`
    - TCH_TIME: representa la cantidad de milisegundos que duró el gesto.
        - Cuando empieza el contacto, se setea con el valor de 0.
        - Durante el contacto, se setea el valor con el tiempo transcurrido desde que se inició el gesto. 
        - Cuando termina el contacto, se setea el valor con el tiempo total que duró el gesto. 

Interrupciones: IRQ_TOUCH, tenemos que rehandlearla con un handler propio que configuraremos en driver_init y lo eliminaremos en driver_remove. ¿Por qué? porque queremos que cuando se cargue el módulo del driver en el Kernel, podamos interpretar esas interrupciones y ejecutar nuestro handler. 

¿Cuando arranca la responsabilidad del driver?
- Write: tenemos que permitir colorear los píxeles.
    - Leemos TCH_COORD. Almacenamos el valor (es un entero).
        - Utilizamos int x = getX(int coord) y int y = getY(int coord). Cuando tenemos ambas, usamos int coord = genCoord(x, y) para que podamos tener el valor que mandamos a PRINT_COORD.
    - PRINT_COORD: le envíamos la coordenada anteriormente calculada (coord)
    - PRINT_COLOR: le envíamos el color a utilizar (AZUL|ROJO|VERDE)
    - PRINT_OK: le escribimos el valor PRINT para que se efectúe el cambio
    De acá lo que manda el usuario es solamente el color según la acción que sucedió.
- Read: acá hay algo interesante, sabemos que cuando empieza el gesto nos llega `IRQ_TOUCH`. 

¿Qué haría el programa de usuario en b)? 
Hacer READ del driver, y en base a la respuesta que nos da, ejecutar el WRITE según los patrones de gestos con sus reglas de dominio.
Notar que entonces, el READ debería dar la información necesaria para que el programa usuario sepa exactamente qué gesto fue realizado.

**Ambigüedad del enunciado**
El READ debe ser bloqueante. Esto significa que, si no hay información disponible, el proceso usuario debe quedar esperando.

El problema es que el hardware genera una única interrupción (IRQ_TOUCH) cuando comienza un gesto, pero no especifica que genere otra cuando termina (TCH_UP).

Supongamos que utilizamos un semáforo para bloquear al proceso cuando ejecuta read() y no hay información disponible. Cuando llega la interrupción, el proceso se despierta. Sin embargo, ¿qué información debería devolverle el driver?
Un gesto puede durar muchísimo tiempo y durante ese período se actualizan constantemente los registros TCH_COORD y TCH_TIME.

Se presentan dos alternativas:

1. Devolver información parcial durante el gesto.
En este caso, el driver devuelve las coordenadas y el tiempo transcurrido cada vez que el usuario ejecuta read().
Pero aparece una dificultad: si el usuario vuelve a ejecutar read(), ¿cómo sabe el driver que debe devolver información inmediatamente y no bloquear al proceso esperando una nueva interrupción?
El usuario ya superó la espera inicial porque el gesto comenzó, pero no hay una nueva interrupción que notifique las actualizaciones posteriores.

2. Esperar hasta que termine el gesto (TCH_UP).
En este caso, el driver mantiene bloqueado al proceso hasta que finalice el contacto y recién entonces devuelve la información.
El problema es que se pierden las actualizaciones intermedias de coordenadas, por lo que no sería posible reconstruir correctamente un deslizamiento.

3. ¿Cómo detecta el driver la finalización del gesto?
Como no se especifica una interrupción al producirse TCH_UP, tampoco queda claro cómo puede el driver detectar ese cambio y despertar al proceso usuario.