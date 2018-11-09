#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>

#include "protocol.h"
#include "board.h"

void play(long sock);

int authenticate(int sock) {
	char user[128];
	char pass[128];
	int16_t c = htons(AUTH_USER);

	printf("===========================================================\n");
	printf("Welcome to the online Minesweeper gaming system\n");
	printf("===========================================================\n\n");
	printf(
			"You are required to log on with your registered user name and password\n\n");

	printf("Username: ");
	scanf("%120s", user);

	printf("Password: ");
	scanf("%120s", pass);

	/* tell that we want to authenticate */
	send(sock, &c, sizeof(int16_t), 0);
	/* sent the length of the username and then the username */
	c = htons(strlen(user));
	send(sock, &c, sizeof(int16_t), 0);
	send(sock, user, strlen(user), 0);

	/* send the lenght of the password and then the password */
	c = htons(strlen(pass));
	send(sock, &c, sizeof(int16_t), 0);
	send(sock, pass, strlen(pass), 0);

	recv(sock, &c, sizeof(int16_t), 0);
	c = ntohs(c);
	if (c == OK) {
		return 1;
	} else {
		return -1;
	}
}

int choose_option() {
	char c = '\0';
	printf("Choose an option: \n");
	printf("<R> Reveal tile\n");
	printf("<P> Place flag\n");
	printf("<Q> Quit game\n");

	while (1) {
		scanf("%c", &c);
		if (c == 'r' || c == 'R')
			return 1;
		if (c == 'P' || c == 'p')
			return 2;
		if (c == 'q' || c == 'Q')
			return 3;
	}
}

/**
 * Get user input for coordinates.
 */
char *get_coordinates() {
	char *c = malloc(10);
	printf("Enter tile coordinates: ");
	while (1) {
		c[0] = '\0';
		scanf("%5s", c);
		if (strlen(c) != 2) {
			printf("Please enter the coordinates in RowCol format eg: B2\n");
		}
		c[0] = toupper(c[0]);
		return c;
	}
}

/**
 * Did the server send an ok response
 */
int is_ok(long sock) {
	int response;
	recv(sock, &response, sizeof(int16_t), 0);
	response = ntohs(response);
	return response == OK;
}

/**
 * Fetch and display the leaderboard
 */
void get_leaderboard(long sock) {
	int16_t size;
	char buf[128];
	write_int(sock, LEADERS);
	if (is_ok(sock)) {
		printf(
				"=============================================================================\n");
		while (1) {
			/* the server will send the leader board as formatted strings.
			 * Loop through them and print them. A zero length string marks the end of the
			 * leaderboard.
			 */
			recv(sock, &size, sizeof(int16_t), 0);
			size = ntohs(size);
			if (size == 0) {
				break;
			}
			else {
				buf[0] = '\0';
				recv(sock, buf, size, 0);
				printf("%s\n", buf);
			}
		}
		printf(
				"=============================================================================\n");
	} else {
		printf(
				"=============================================================================\n");
		printf(
				"There is no information currently stored in the leaderboard. Try again later.\n");
		printf(
				"==========================================================================\n\n");
	}
}
/**
 * Update the board by fetching latest status from the server.
 */
void update_board(long sock, struct Board *board) {
	int i, j;
	int16_t c;
	write_int(sock, UPDATE_BOARD);
	recv(sock, &c, sizeof(int16_t), 0);
	board->remaining = ntohs(c);

	for (i = 0; i < NUM_TILES_Y; i++) {
		for (j = 0; j < NUM_TILES_X; j++) {
			recv(sock, &c, sizeof(int16_t), 0);
			c = ntohs(c);
			if (c == -1) {
				// we are still in the dark about this one.
				board->tiles[i][j].revealed = 0;
				board->tiles[i][j].mined = 0;
			} else if (c == -2) {
				// this is a mine that has been revealed/flagged
				board->tiles[i][j].revealed = 1;
				board->tiles[i][j].mined = 1;
			} else {
				board->tiles[i][j].revealed = 1;
				board->tiles[i][j].neighbours = c;
			}
		}
	}
}

/**
 * The main client menu.
 */
void main_menu(long sock) {
	char choice = '\0';

	while (1) {
		if(choice != ' ' || choice != '\n') {
			printf("Welcome to the Minesweeper gaming system.\n");
			printf("Please enter a selection:\n");
			printf("<1> Play Minesweeper\n");
			printf("<2> Show leaderboard\n");
			printf("<3> Quit\n");
		}

		if (scanf("%c", &choice)) {
			if (choice == '1') {
				play(sock);
			}
			if (choice == '2') {
				get_leaderboard(sock);
			}
			if (choice == '3') {
				return;
			}
		}
	}
}

/**
 * Play the game
 */
void play(long sock) {
	struct Board *board = create_board();
	char *coords;
	int choice = 0;
	int16_t response;

	write_int(sock, START_GAME);
	print_board(board);

	while (1) {
		choice = choose_option();
		if (choice == 3) {
			break;
		}
		coords = get_coordinates();

		if (choice == 1) {
			write_int(sock, REVEAL_TILE);
			send(sock, coords, 2, 0);

			recv(sock, &response, sizeof(int16_t), 0);
			response = ntohs(response);
			// printf("SERVER RESPONSE for %d, %d --> %d\n", coords[0], coords[1], response);
			update_board(sock, board);
			// printf("BOARD UPDATED\n");
			if (response == MINE_TILE) {
				board->running = 0;
				update_board(sock, board);
				print_board(board);
				printf("\nGame over! You hit a mine.\n");
				write_int(sock, OK); // acknowledge that the game is over
				return;
			}
		}
		if (choice == 2) {
			write_int(sock, FLAG_TILE);
			send(sock, coords, 2, 0);
			recv(sock, &response, sizeof(int16_t), 0);
			response = ntohs(response);
			if (response == MINE_TILE) {
				board->remaining--;
				update_board(sock, board);
				printf("%d mines remaining\n", board->remaining);
			} else if (response == WINNER) {
				printf("DRUM ROLL\n");
				board->running = 0;
				update_board(sock, board);
				print_board(board);
				printf("\nCongratulations! You have located all the mines!\n");

				recv(sock, &response, sizeof(int16_t), 0);
				response = ntohs(response);

				printf("You won in %d seconds!\n", response);
				write_int(sock, OK); // acknowledge that the game is over
				return;
			} else {
				printf("There's no mine at that location\n");
			}
		}
		print_board(board);
		free(coords);
	}
}

int main(int argc, char **argv) {
	char *server = NULL;
	int port = 12345;
	char *p;
	int sock = 0;
	struct sockaddr_in serv_addr;

	if (argc >= 2) {
		server = argv[1];
	}
	if (argc == 3) {
		port = strtol(argv[2], &p, 10);
		if (port == 0) {
			printf("The second parameter should be the port number\n");
			return -1;
		}
	} else if (argc > 3 || argc < 2) {
		printf("Invalid number of arguments\n");
		return -1;
	}

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Could not create client socket \n");
		return -1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, server, &serv_addr.sin_addr) <= 0) {
		printf("Invalid server address \n");
		return -1;
	}

	if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("Could not connect to minesweeper host \n");
		return -1;
	}
	//send(sock, hello, strlen(hello), 0);
	printf("Hello message sent\n");
	if (authenticate(sock) == 1) {
		main_menu(sock);
	} else {
		printf(
				"You entered an incorrect username or password. Disconnecting\n");
	}

	return 0;
}
