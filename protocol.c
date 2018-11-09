#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>
#include "server.h"
#include "protocol.h"

void write_int(long sock, int16_t status) {
	int16_t ok = htons(status);
	send(sock, &ok, sizeof(int16_t), 0);
}
