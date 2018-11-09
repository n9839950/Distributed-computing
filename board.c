#include <stdio.h>
#include <stdlib.h>
#include "board.h"
#include "protocol.h"

/**
 * Creates a new instance of the board.
 */
struct Board *create_board() {
	int i, j;
	struct Board *board = malloc(sizeof(struct Board));

	for (i = 0; i < NUM_TILES_Y; i++) {
		for (j = 0; j < NUM_TILES_X; j++) {
			board->tiles[i][j].neighbours = 0;
			board->tiles[i][j].revealed = 0;
			board->tiles[i][j].mined = 0;
		}
	}

	board->remaining = NUM_MINES;
	board->running = 1;

	return board;

}

/**
 * Reveals the entire board when the game is over.
 */
void reveal_all(struct Board *board) {
	int i, j;

	for (i = 0; i < NUM_TILES_Y; i++) {
		for (j = 0; j < NUM_TILES_X; j++) {
			if (board->tiles[i][j].mined)
				board->tiles[i][j].revealed = 1;
		}
	}
}
/**
 * Reveals the tile at the given coordinates
 *
 */
int reveal_tile(struct Board *board, int *coords) {
	int row = coords[0];
	int col = coords[1];
	int next[2];
	struct Cell cell = board->tiles[row][col];
	int c = -1;

	if (cell.mined) {
		return MINE_TILE;
	}
	if (cell.revealed) {
		return EMPTY_TILE;
	}

	board->tiles[row][col].revealed = 1;
	c = cell.neighbours;

	if (c == 0) {
		if (row > 0) {
			if (col > 0) {
				next[0] = row - 1;
				next[1] = col - 1;
				reveal_tile(board, next);
			}
			if (col < NUM_TILES_X - 1) {
				next[0] = row - 1;
				next[1] = col + 1;
				reveal_tile(board, next);
			}
			next[0] = row - 1;
			next[1] = col;
			reveal_tile(board, next);
		}
		if (row != NUM_TILES_Y - 1) {
			if (col > 0) {
				next[0] = row + 1;
				next[1] = col - 1;
				reveal_tile(board, next);
			}
			if (col < NUM_TILES_X - 1) {
				next[0] = row + 1;
				next[1] = col + 1;
				reveal_tile(board, next);
			}
			next[0] = row + 1;
			next[1] = col;
			reveal_tile(board, next);
		}

		if (col > 0) {
			next[0] = row;
			next[1] = col - 1;
			reveal_tile(board, next);
		}
		if (col < NUM_TILES_X - 1) {
			next[0] = row;
			next[1] = col + 1;
			reveal_tile(board, next);
		}
	}

	return EMPTY_TILE;
}

/**
 * Flag this tile as being mined.
 */
int flag_tile(struct Board *board, int *coords) {
	int col = coords[1];
	int row = coords[0];
	if(board->tiles[row][col].mined) {
		board->remaining--;
		board->tiles[row][col].revealed = 1;
		return MINE_TILE;
	}
	else {
		return EMPTY_TILE;
	}
}
/**
 * Prints the given board.
 */
void print_board(struct Board *board) {
	int i = 0;
	printf("    ");
	for (i = 0; i < NUM_TILES_X; i++) {
		printf(" %d", i + 1);
	}
	printf("\n");
	printf("----");
	for (i = 0; i < NUM_TILES_X; i++) {
		printf("--");
	}
	printf("\n");
	for (i = 0; i < NUM_TILES_Y; i++) {
		printf("%c |  ", (char) 'A' + i);
		for (int j = 0; j < NUM_TILES_X; j++) {
			if (board->tiles[i][j].revealed) {
				if(board->tiles[i][j].mined) {
					if(board->remaining == 0 || board->running) {
						printf("+ ");
					}
					else {
						printf("* ");
					}
				}
				else {
					printf("%d ", board->tiles[i][j].neighbours);
				}
			}
			else {
				printf("  ");
			}
		}
		printf("\n");
	}
}

/**
 * Counts the number of mines around a cell
 */
int count_neighbours(struct Board *board, int row, int col) {
	int count = 0;
	if (row != 0) {
		if (col != 0) {
			if (board->tiles[row - 1][col - 1].mined)
				count++;
		}
		if (col != NUM_TILES_X - 1) {
			if (board->tiles[row - 1][col + 1].mined)
				count++;
		}
		if (board->tiles[row - 1][col].mined)
			count++;
	}
	if (row != NUM_TILES_Y - 1) {
		if (col != 0) {
			if (board->tiles[row + 1][col - 1].mined)
				count++;
		}
		if (col != NUM_TILES_X - 1) {
			if (board->tiles[row + 1][col + 1].mined)
				count++;
		}
		if (board->tiles[row + 1][col].mined)
			count++;
	}
	if (col != 0) {
		if (board->tiles[row][col - 1].mined)
			count++;
	}
	if (col != NUM_TILES_X - 1) {
		if (board->tiles[row][col + 1].mined)
			count++;
	}
	board->tiles[row][col].neighbours = count;
	return count;
}

/**
 * Initializes the board
 */
void init_board(struct Board *board) {
	int i, x, y;
	for (i = 0; i < NUM_MINES; i++) {
		do {

			x = rand() % NUM_TILES_X;
			y = rand() % NUM_TILES_Y;

			//printf("%d %d     ", y, x);
		} while (board->tiles[y][x].mined);

		board->tiles[y][x].mined = 1;
	}

	for (i = 0; i < NUM_TILES_Y; i++) {
		for (int j = 0; j < NUM_TILES_X; j++) {
			count_neighbours(board, i, j);
		}
	}
}
