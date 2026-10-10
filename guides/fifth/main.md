# Guía 5: Entrada/Salida

## Ejercicio 1
Las verdaderas son $a$ y $c$.

El driver es un software que forma parte del sistema operativo y que conoce tanto las particularidades del modelo que corre como la interfaz del SO en el que corre.

Conoce tanto al SO como para el modelo de dispositivo que va a servir debido a que necesita conocer sus registros, comandos y particularidades.

Corre en privilegio Kernel porque accede a registros, memoria del dispositivo, etc. 

¿Por qué las incorrectas son incorrectas?

b) No es hardware. Es software que utiliza el sistema operativo para realizar la comunicación entre un usuario con un dispositivo específico. 

d) Que el usuario pueda cambiarlo no lo convierte en aplicación de usuario. Una aplicación corre en modo usuario, sin acceso directo al hardware; el driver corre en modo kernel y accede a registros, memoria del dispositivo, etc. También podés cambiar o actualizar otros módulos del kernel y no por eso son aplicaciones.

e) El gestor de interrupciones es un componente del SO. El driver sí suele incluir un handler o rutina de servicio para las interrupciones de su dispositivo, pero eso no lo hace “un gestor de interrupciones” en general. Es una de sus funciones, no su definición.

f) Está al revés: el driver tiene que conocer el SO en el que corre, porque usa su API interna (estructuras, llamadas del kernel, modelo de E/S).

g) El driver sí tiene la información del modelo específico de dispositivo que tiene que manejar debido a que necesita conocer sus registros, comandos y particularidades.

## Ejercicio 2
CHRONO_CURRENT_TIME y CHRONO_CTRL son dos registros. Por lo tanto podemos hacer operaciones *IN/OUT* con él. 

Reiniciar cronómetro: mandar CHRONO_RESET a CHRONO_CTRL

Leer tiempo medido: CHRONO_CURRENT_TIME. 

La operación *read()* debe devolver el tiempo actual. 

La operación *write()* debe reiniciarse el cronometro.

No nos piden un código a nivel de usuario, así que podemos desarrollar directamente el driver asumiendo cómo el usuario va a utilizarlo. 

Asumo que cuando dice *debe devolver el tiempo actual* indica explícitamente que el valor del cronómetro es lo que retornamos con el mismo tipo de dato con el que haya sido definido. 

Como no esta explícitado el tipo de dato del cronómetro asumo que es un valor entero. 

A continuación, un pseudocódigo aproximado.


```text
    #define IO_OK 1

    //inicializar el driver. Acá va el típico boilerplate + config init de mutexes.
    int driver_init(){
        return IO_OK;
    }

    int driver_read(int* data){
        int tiempo = IN(CHRONO_CURRENT_TIME);
        return tiempo; 
    }

    int driver_write(int* data){
        OUT(CHRONO_CTRL, CHRONO_RESET);
        return IO_OK;
    }

    //destruir el driver. Acá iría el típico boilerplate.
    int driver_exit(){}

```

**Preguntar**: ¿qué convención vamos a tener del OUT? porque en C usamos *outb(VALOR, PUERTO)* pero acá es *OUT(PUERTO, VALOR)*

## Ejercicio 3
El registro de *E/S* es *BTN_STATUS*. Por lo tanto vamos a poder usar las operaciones de *IN/OUT* con él. 

Solo nos interesan el *bit* menos significativo y el segundo *bit* menos significativo.

El *bit* menos significativo, es decir, BTN_STATUS$_0$ es el valor: 0 si no fue pulsada, 1 si fue pulsada.

El segundo *bit* menos significativo, es decir, BTN_STATUS$_1$ hay que escribirle 0 para limpiar la memoria de la tecla.

Hay que usar máscaras.

Debemos retornar la constante *BTN_PRESSED* cuando se presiona la tecla.

Hay que usar busy waiting

El objetivo, por lo que interpreto, es que hay que hacer busy waiting en el driver_read hasta que alguien realmente pulse la tecla. Si alguien te pide "escuchar si se presionó", no tenés que devolver nada hasta que alguien realmente presione. 

```text
    #define IO_OK 1
    #define IO_ERR 0

    //inicializar el driver. Acá va el típico boilerplate + config init de mutexes.
    int driver_init(){
        return IO_OK;
    }

    int driver_read(int* data){
        while((IN(BTN_STATUS) & 0x01) == 0);
        OUT(0, BTN_STATUS);
        return BTN_PRESSED; 
    }

    int driver_write(int* data){
        return IO_ERR;
    }

    //destruir el driver. Acá iría el típico boilerplate.
    int driver_exit(){}

```

**Preguntar**: ¿cuando sabés si tenés que meter la información de retorno en data o devolver directo en la función? porque a veces los "retornar" no los interpreto tan claramente.

## Ejercicio 4
Ahora nos piden el ejercicio 3 pero con interrupciones.

Para eso vamos a tener que hacer varias cosas:
1. Agregar semáforos. Nos va a solucionar el tema del busy waiting ya que ahora vamos a escuchar una interrupción.
2. Configurar en el *driver_init* para escuchar la interrupción.
3. Configurar al dispositivo para que cuando alguien pulse la tecla dispare la interrupción *(guardar BTN_INT en el registro de la tecla)* 

Aclaración: solo puse el mutex a modo de ejemplo de que si tenés varios *IN/OUT* necesitás sí o sí proteger el acceso por concurrencia. En este ejercicio ese *mutex* no es necesario porque es un único OUT.

```text
    #define IO_OK 1
    #define IO_ERR 0

    //inicializar el driver. Acá va el típico boilerplate + config init de mutexes.
    int driver_init(){
        sem = sem.init(0)
        mutex = mutex.init();
        OUT(BTN_INT, BTN_STATUS);
        request_ir7(BTN_INT, handler)
        return IO_OK;
    }

    //con los mutex protegemos los accesos concurrentes al recurso compartido de E/S. Con el semáforo esperamos la interrupción.
    int driver_read(int* data){
        sem.wait();
        mutex.lock();
        OUT(0, BTN_STATUS);
        mutex.unlock();
        return BTN_PRESSED; 
    }

    int driver_write(int* data){
        return IO_ERR;
    }

    //destruir el driver. Acá iría el típico boilerplate.
    int driver_exit(){}

    void handler(){
        //si efectivamente pulsó, mando la señal.
        if((IN(BTN_STATUS) & 0x01) == 1){
            sem.signal()
        }
    }

```

**Preguntar**: 
1. ¿En algún momento liberamos el handler, tipo, driver_exit? ¿los semáforos, los mutex? ¿qué pasaría si se hiciera? Razonarlo. Pensar en qué contexto viven esas cosas. Cuantas veces se inicializarían por vez que el driver se carga, quién usa ese programa, etc.
2. ¿Está bien que la limpieza esté en *driver_read* y no en el handler?
3. ¿Asumo que está bien hecha la configuración del dispositivo de hardware para emitir la interrupción, no? La hice en el *driver_init()*. ¿Qué pasaría si BTN_STATUS tiene alguna otra información, haciendo lo que hago hoy, no estaría pisando esa configuración previa? 

## Ejercicio 5
El administrador de E/S es un componente del sistema operativo.

Cuando alguien efectúa un *open()* está solicitando el acceso al driver. El *open()* prepara el acceso a un archivo o dispositivo para que después el usuario pueda operar con él.
1. Resuelve qué recurso querés abrir, a partir de la ruta /dev/tecla.
2. Verifica si tenés permiso para acceder a ese recurso.
3. Prepara el estado necesario para esa apertura, incluyendo la información que permitirá identificar el recurso en las operaciones posteriores.
4. Te devuelve un file descriptor (fd), que es el identificador que vas a usar en read(), write() y otras operaciones.

Cuando alguien efectúa un *write()* el proceso le pide al SO que escriba una determinada cantidad de datos en el recurso asociado a un file descriptor.

## Ejercicio 6
El nivel de acceso para las syscalls IN y OUT deben ser de privilegio Kernel.

Porque estas instrucciones te permiten interactuar con registros de E/S del hardware.

En nivel de usuario no podés. 

## Ejercicio 7