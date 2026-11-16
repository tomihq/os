# Cheatsheet
## Scheduler
- new: proceso creado.
- ready: proceso que compite por CPU. Listo para poder ser tomado por el scheduler.
- running: proceso que está corriendo
- blocked: el proceso está esperando algún evento o I/O.
- terminated: el proceso terminó.
## C
### C funciones libc
Muchas de estas funciones utilizan una o más syscalls.

Las funciones de la libc se ejecutan en modo usuario, dentro del espacio de memoria de usuario. 
```c
pid_t fork(): crea un nuevo proceso hijo idéntico al actual (memoria, configuraciones, etc). El hijo arranca desde el mismo lugar que fue creado. 
Utiliza la syscall clone.

void exit(int status): finaliza el programa con el status proporcionado. status = 0 = OK, status != 0 != OK

void pause(): bloquea el programa (blocked en scheduler) y no te despertás hasta que llega una señal. Esa señal la tenés que tener handleada para que puedas despertarte. No podés elegir con cual de ellas despertarte. Cualquiera hace efecto. 

pid_t waitpid(pid_t pid, int* status): bloquea el programa (blocked en scheduler) hasta que el hijo con pid proporcionado se ponga en el estado proporcionado por parámetro. El valor default es que el hijo termine.
Utiliza la syscall wait4.

pid_t wait(int* status): bloquea el programa (blocked en scheduler) hasta que algún hijo se ponga en el estado proporcionado. El valor default es que algún hijo termine, y lo enviás como NULL. 
Wait utiliza waitpid internamente: waitpid(-1, &status);

void sleep(int time): duerme al proceso por el tiempo especificado con la variable time. El tiempo debe estar en segundos (se puede configurar para otra unidad igual)

pid_t getppid(): obtiene el PID del padre del proceso que lo está invocando. 

pid_t getpid(): obtiene el PID del proceso actual. 

int exec*: la familia de funciones exec permite reemplazar el programa actual por uno nuevo sin cambiar el PID. Si el reemplazo es exitoso, NUNCA se vuelve al programa que ejecutó la instrucción. Si falla, el reemplazo no se hace. 

void kill(pid_t pid, int señal): envía la señal indicada por parámetro al proceso con el pid indicado. 

void signal(int signal, sighandler_t handler): hace el rehandleo de una señal en particular.

ssize_t write(int fd, const void buf[count], size_t count): escribe al archivo referido por el file descriptor, {count} bytes del buffer enviado como parámetro.

ssize_t read(int fd, void buf[count], size_t count): lee al archivo referido por el file descriptor, y almacena en {count} bytes en el buffer.

int open(char* path, int flags, ...): abre un archivo y te devuelve un file descriptor que apunta a ese archivo.
int close(int fd): cierra un file descriptor para el proceso actual. 
int pipe(int pipefd[2]): se crea un pipe unidireccional. Cada elemento refiere a un file descriptor. El 0 es el extremo de lectura mientras que el 1 es el extremo de escritura. 

Tanto el write() y read() del pipe son bloqueantes.
int dup2(int oldfd, int newfd): duplica un file descriptor. En particular, newfd refiere al mismo file descriptor que oldfd.
```
Para ver qué syscalls se hacen cuando usamos una función, podemos usar programas como strace. 

### C funciones libc (Sockets)
```c
int socket(int domain, int type, int protocol): inicializa un nuevo socket. En la materia, domain: AF_UNIX, type: SOCK_STREAM.
int bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen): asigna el nombre al socket sockfd enviado por addr, addrlen es el tamaño de sockdaddr
int listen(int sockfd): marca al socket sockfd como un socket pasivo, es decir, que el socket va a servir para aceptar conexiones usando accept().
int accept(int sockfd, struct sockaddr* addr, socklen_t addrlen): acepta una solicitud de conexión al socket. Devuelve un nuevo file descriptor para el nuevo socket.
ssize_t send(int sockfd, const void buf[.len], size_t len): envía al file descriptor indicado por sockd, {len} bytes del buf. Es igual a la función ssize_t write() excepto que send recibe flags.
ssize_t recv(int sockfd, void buf[.len], size_t len, int flags): recibís mensajes de un socket. Es igual a la función ssize_t read() excepto que recv recibe flags.
```
### Syscalls
Las syscalls son formas de hacer llamadas al sistema desde un proceso. Cuando decimos llamadas al sistema hablamos de que solicitamos algo al Kernel, que involucra cambios de contexto,  cambios de privilegio, etc.

### Señales
Son un mecanismo que incorporan los sistemas operativos POSIX y que permite notificar a un proceso la ocurrencia de un evento. 

La ejecución de un proceso se interrumpe cuando se recibe una interrupción, la cual tiene asociado un handler que tiene un número asociado *(si no tiene, la default es signal())*.

Algunas señales NO se pueden rehandlear: por ejemplo, SIGKILL y SIGSTOP no se les puede cambiar el comportamiento.

Las señales no se encolan. Es decir, si a un programa le mandás un millon de señales pero no procesó alguna, entonces hasta que no procese esa, el resto las va a ignorar.

Si necesitás que las señales se encolen, hay un tipo particular pero no nos interesan acá. 

#### Cómo se reaccionan a las señales cuando los procesos duermen
Cuando los procesos están dormidos, por ejemplo, con un `pause()` una excelente pregunta sería ¿y cómo hace el proceso, que está en BLOCKED dentro del scheduler para pasar a READY y competir por CPU nuevamente? 

Cuando el proceso está en BLOCKED, no consume por CPU porque está esperando algún evento como mencioné anteriormente. 

Cuando llega una señal, el Kernel recibe esa señal y se pregunta: "llegó una señal ¿para qué PID es?". 

Notar que el proceso está BLOCKED y le cambia su estado a READY para que el scheduler esté al tanto de que ya está listo para competir por CPU.

Apenas el scheduler lo pone en ritmo, el proceso se pone en RUNNING y ejecuta el handler correspondiente a la señal recibida. 

Recién cuando el handler termina, el código sigue desde el `pause()`.

#### Algunas señales
En C, se pueden utilizar a través de su nombre, pero en el fondo son un número. 
```c
SIGKILL: matar a un proceso inmediatamente. No se le da tiempo de hacer nada. Es un kill inmediato. 
SIGCHLD: cambio de estado de un hijo.
SIGINT: interrupción de un proceso. El handler default en los programas es terminar el proceso.
SIGTERM: es una señal que le indica a un proceso para que termine de forma ordenada y limpia. Se le da tiempo de guardar su estado si lo necesita *(graceful shutdown)*
```
#### Aclaraciones sobre el rehandleo
Los handlers deben utilizar funciones Async-Signal-Safe. ¿Qué significa esto? 

Los handlers PUEDEN SER INTERRUMPIDOS cuando están trabajando. Por lo que si este handler requiere acceso a un recurso y obtiene permiso, cualquier otro que interrumpa y quiera usar ese recurso no va a poder hasta que ese handler lo suelte. 

Pero si lo interrumpiste, no lo va a poder usar. Deadlock.

Un claro ejemplo es usar malloc. Si un proceso A está usando malloc en un handler, obtiene un lock. Si justo en el medio de eso llega una interrupción de un proceso que necesita malloc también, no lo va a poder usar, y ambos se van a colgar. 

#### Funciones Async-Signal-Safe
Son funciones que funcionan correctamente aunque interrumpan a otra funcion cualquiera porque:
- no usan locks
- bloquean las señales que tocan ese estado
- son wrappers de syscalls (el kernel se encarga de sincronizar)

De las anteriores que mencionamos de la libc, son Async-Signal-Safe las siguientes
```c
write, read, open, close, _exit, kill, fork, execve, wait, waitpid, signal, pipe, dup2, sleep
```
 
### Aclaraciones
```c
pid_t fork(): los espacios de memoria son diferentes. Sin embargo, luego del fork, el hijo apunta a las mismas tablas de páginas del padre. Apenas alguno de los dos escribe, lo escribe en su propio espacio de memoria. El resto permanece igual. 
Esta estrategia es conocida como copy-on-write. En vez de copiar toda la memoria entera que podría ser demasiada, se comparten las mismas páginas hasta que alguno hace una modificación. 

pid_t wait*(): recolecta los recursos de los hijos que hayan finalizado o bien con exit() o return, o hayan muerto por una señal. Es indispensable hacer esta recolección para no dejar a los hijos en modo Zombie.

void kill(pid_t pid, int señal): a nivel de procesos, los padres pueden enviar señales a sus hijos, sus hijos a sus padres, y los hijos entre ellos (hermanos). 

Pipes: todos quienes no vayan a usarlo deben cerrarlos (incluso los que terminaron de usarlos deben cerrarlos), de lo contrario nunca se llegará al EOF. 
Tenés que cerrar cada extremo por separado, porque son file descriptors diferentes.

```

## Utilidades Sockets
1. Una vez que aceptás la conexión, hacés fork() y a ese proceso lo hacés responsable de su propio file descriptor. El servidor cierra ese fd porque no lo va a usar.
2. Enviar y recibir información puede ser pesado. Mencionar siempre que vas pasando de a pedacitos (chunks), donde el Content-Length es lo que te permite determinar después: cuanto leíste y cuanto te falta. 