#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/time.h>
#include "server.h"
#include "leaderboard.h"
#include "protocol.h"
#include "board.h"


/**
 * Multiple threads needs to access the leader board and there isn't a clear
 * solution so reluctantly globals are used.
 */
struct Record *records = NULL;

/**
 * A usefull tool for logging
 */
int logger(const char* format, ...) {

	int charsprinted;
	va_list argvec;
	va_start(argvec, format);
	if (VERBOSE) {
		charsprinted = vprintf(format, argvec);
	}
	va_end(argvec);
	return charsprinted;
}

/**
 * Check the username and password.
 * The validation is done against the contents of the password file
 */
int check_auth(char *user, char *password) {
	char uname[32], pass[32];

	FILE *fp = fopen(PASSWORD_FILE, "r");
	if (fp != NULL) {
		while (!feof(fp)) {
			fscanf(fp, "%s %s", uname, pass);
			if (strcmp(uname, user) == 0 && strcmp(pass, password) == 0) {
				logger("Authenticated %s\n", user);
				return 1;
			}
		}
	}
	return 0;
}

/**
 * Start the server on the given port
 *
 * Spawns a new thread for each incoming connection.
 */
void start_server(int port) {

	int master = 0; // the file descriptor for the main socket
	int size = 0; // the size of the server struct
	long fd = 0; // the file descrptor for each socket connected to a client.

	struct sockaddr_in server, client;

	printf("Starting web server on port %d.\n", port);

	master = socket(AF_INET, SOCK_STREAM, 0);

	if (master != -1) {
		server.sin_addr.s_addr = INADDR_ANY;
		server.sin_port = htons(port);
		server.sin_family = AF_INET;

		size = sizeof(struct sockaddr_in);
		if (bind(master, (struct sockaddr *) &server, sizeof(server))) {
			printf("Could not bind to %d\n", port);
			return;
		}
		listen(master, 3);

		while (1) { // loop indefinitely while waiting for connections
			printf("Server listening\n");
			fd = accept(master, (struct sockaddr *) &client,
					(socklen_t*) &size);
			if (fd == 0) {
				break;
			}

			/* a new connection, pass it to a new thread */
			pthread_t handler;
			pthread_create(&handler, NULL, listener, (void *) fd);
		}

	} else {
		printf("Could not create socket\n");
	}
}

/**
 * Authenticate by reading username and password from socket.
 */
int authenticate(int sock, char *user, char *pass) {

	int16_t len;

	/* get the length of the username */
	size_t size = recv(sock, &len, sizeof(int16_t), 0);
	if (size != sizeof(int16_t)) {
		return -1;
	}
	len = ntohs(len);

	/* read the user name */
	size = recv(sock, user, len, 0);
	if (size != len) {
		return -1;
	}

	/* get the length of the passwword */
	size = recv(sock, &len, sizeof(int16_t), 0);
	len = ntohs(len);
	if (size != sizeof(int16_t)) {
		return -1;
	}

	/* read the password */
	size = recv(sock, pass, len, 0);

	if (size != len) {
		return -1;
	}

	if (check_auth(user, pass) == 1) {
		return 0;
	}
	return -1;
}

/**
 * Reads the coordinates from the socket.
 */
int * get_coords(int sock) {
	int *coords = malloc(sizeof(int) * 2);
	char buf[10] = { 0 };
	recv(sock, buf, 2, 0);

	logger("Recieved coordinates %s\n", buf);
	if (buf[0] >= 'A' && buf[0] <= 'I') {
		coords[0] = buf[0] - 'A';
		if (buf[1] >= '1' && buf[1] <= '9') {
			coords[1] = buf[1] - '1';
			return coords;
		}
	}
	free(coords);
	return NULL;
}

/**
 * Sends the client information about the updated board status.
 *
 * Only information that has been revealed either by the reveal or flag
 * command will be shared.
 */
void update_board(long sock, struct Board *board) {
	int i, j;

	/* write the number of mines that are undiscovered */
	write_int(sock, board->remaining);
	logger("Sending board updates to client %d mines remain\n",
			board->remaining);
	for (i = 0; i < NUM_TILES_Y; i++) {
		for (j = 0; j < NUM_TILES_X; j++) {
			if (board->tiles[i][j].revealed) {
				if (board->tiles[i][j].mined) {
					/* write that we have revealed a mine */
					write_int(sock, -2);
				} else {
					/* write that we have revealed an emptry square with N mines on
					 * adjoining cells
					 */
					write_int(sock, board->tiles[i][j].neighbours);
				}
			} else {
				write_int(sock, -1);
			}
		}
	}

}

/**
 * Sends the leaderboard
 */
void send_leaders(long sock) {
	int i, len, empty = 1 ;
	char buf[128];
	/* first lets check that the board is not empty */
	for(i = 0 ; i < LEADERBOARD_SIZE ; i ++) {
		if(records[i].name[0]) {
			empty = 0;
			break;
		}
	}
	if(!empty) {
		write_int(sock, OK);
		for (i = LEADERBOARD_SIZE; i >= 0; i--) {
			if (records[i].name[0]) {
				buf[0] = '\0';
				sprintf(buf, "%20s %4d seconds    %2d games won, %2d games played",
						records[i].name, records[i].seconds, records[i].won,
						records[i].played);
				len = strlen(buf);
				write_int(sock, len);
				send(sock, buf, len, 0);
				logger(buf);
				logger("\n");
			}
		}
		write_int(sock, 0);
	}
	else {
		write_int(sock, ERROR);
		logger("Leader board is currently empty\n");
	}
}

/**
 * The workshorse, the method called by pthread create.
 */
void * listener(void *data) {
	/* holds the coordinates of a tile being flagged or revealed */
	int *coords = NULL;
	long sock = (long) data;
	int16_t command;
	int ok = 1;
	char user[128] = { 0 };
	char pass[128] = { 0 };

	/* the user should start by sending an auth command */
	recv(sock, &command, sizeof(int16_t), 0);
	command = ntohs(command);
	if (command == AUTH_USER) {
		if (authenticate(sock, user, pass) != 0) {
			/* if the authentication has failed, we just disconnect */
			write_int(sock, ERROR);
			ok = -1;
		} else {
			write_int(sock, OK);
			logger("Client authenticated sending OK response %d \n", htons(OK));
		}
	} else {
		ok = -1;
	}

	while (ok == 1) {
		/* loop until the client disconnects */
		size_t size = recv(sock, &command, sizeof(int16_t), 0);
		command = ntohs(command);
		int live = 1;

		if(size != sizeof(int16_t)) {
			logger("client disconnected");
			break;
		}
		if (command == START_GAME) {
			struct Board *board = create_board();
			init_board(board);
			struct timeval t1, t2;
			gettimeofday(&t1, NULL);

			while (live) {
				size_t size = recv(sock, &command, sizeof(int16_t), 0);
				command = ntohs(command);

				logger("command = %d\n", command);
				if (size != sizeof(int16_t)) {
					printf("Client disconnected? %ld %ld\n", size,
							sizeof(int16_t));
					ok = -1;
					break;
				}
				switch (command) {
				case UPDATE_BOARD:
					update_board(sock, board);
					break;

				case REVEAL_TILE:
					coords = get_coords(sock);
					if (coords != NULL) {
						int c = reveal_tile(board, coords);
						write_int(sock, c);
						if (c == MINE_TILE) {
							/* game lost */
							board->running = 0;
							gettimeofday(&t2, NULL);
							int16_t seconds = t2.tv_sec - t1.tv_sec;
							logger("\nGAME OVER: Time taken %d\n", seconds);

							reveal_all(board);
							board->running = 0;
						}
						logger("Reveal tile at %d, %d -> %d\n", coords[0],
								coords[1], c);
					}
					free(coords);
					break;

				case FLAG_TILE:
					coords = get_coords(sock);
					if (coords != NULL) {
						int c = flag_tile(board, coords);
						if (c == MINE_TILE) {
							if (board->remaining == 0) {
								/* puzzle solved, game over. */
								board->running = 0;
								gettimeofday(&t2, NULL);
								int16_t seconds = t2.tv_sec - t1.tv_sec;
								write_int(sock, WINNER);
								write_int(sock, seconds);

								logger("GAME OVER: Time taken %d", seconds);

								/* create an entry in the leaderboard */
								insert(records, user, seconds, 1);
								logger("Create leader board entry for %s\n", user);


							} else {
								write_int(sock, MINE_TILE);
							}
						} else {
							write_int(sock, EMPTY_TILE);
						}
						logger("Flag tile at %d, %d -> %d\n", coords[0],
								coords[1], c);
					}
					free(coords);
					break;

				case OK:
					if(board->running == 0) {
						/* acknowledgement by the client that the game is over */
						live = 0;
					}
					break;
				}
			}
			logger("Free up memory\n");
			free(board);
		} else if (command == LEADERS) {
			send_leaders(sock);
		} else if (command == BYE) {
			break;
		}
	}
	close(sock);
	pthread_exit(NULL);
	return 0;
}

int main(int argc, char *argv[]) {
	int port = 12345;
	char *p;
	if (argc == 2) {
		port = strtol(argv[1], &p, 10);
		if (port == 0) {
			printf("The first parameter should be the port number\n");
			return 1;
		}
	} else if (argc > 2) {
		printf("Invalid number of arguments\n");
		return 1;
	}
	srand(RANDOM_NUMBER_SEED);
	records = create_leaderboard();

	start_server(port);
}

