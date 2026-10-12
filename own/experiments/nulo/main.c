// Cliente de ejemplo para /dev/nulo
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char **argv)
{
	const char *msg = argc > 1 ? argv[1] : "hola driver";
	char buf[64];

	int fd = open("/dev/nulo", O_RDWR);
	if (fd < 0) {
		perror("open /dev/nulo (¿está cargado el driver?)");
		return 1;
	}

	ssize_t w = write(fd, msg, strlen(msg));
	printf("write(\"%s\") -> %zd (esperado: %zu)\n", msg, w, strlen(msg));

	ssize_t r = read(fd, buf, sizeof(buf));
	printf("read() -> %zd (esperado: 0, EOF)\n", r);

	close(fd);
	return 0;
}