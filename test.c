#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>

#include "leaderboard.h"
#include "server.h"
#include "protocol.h"
#include "board.h"

void print_records(struct Record *records);

void leaders() {
	char *names[] = { "Jack", "Jill", "Henry", "Brown", "Sally", "Douglas",
			"William", "Greg", "Thomas", "Anny", "Lucy", "Mary", "Jane",
			"Violet", "Liz", "Ginger", "Algy", "Bertie", "Nelson", "Nick",
			"Len", "Alex", "Ben", "Hilary", "Shirley" };
	int i, j;
	struct Record *records = create_leaderboard();
	for (i = 0; i < 25; i++) {
		j = rand() % 20;
		insert(records, names[j], rand() % 2500, rand() % 2);
	//print_records(records);
	//	printf("\n\n");
	}
	print_records(records);
}

void print_records(struct Record *records) {
	int i ;
	for (i = LEADERBOARD_SIZE; i >= 0; i--) {
		if (records[i].name[0]) {
			printf("%d: %20s %4d seconds    %2d games won, %2d games played\n",
					i, records[i].name, records[i].seconds, records[i].won,
					records[i].played);
		}
	}
}
int main(int argc, char **argv) {
	int coords[2];
	srand(RANDOM_NUMBER_SEED);
	struct Board *board = create_board();
	init_board(board);
	print_board(board);

	reveal_all(board);
//	coords[0] = 0;
//	coords[1] = 1;
//	reveal_tile(board, coords);
//
//	coords[0] = 4;
//	coords[1] = 4;
//	reveal_tile(board, coords);

	print_board(board);
	leaders();
}

