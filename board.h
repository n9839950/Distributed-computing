/*
 * board.h
 *
 *  Created on: Oct 18, 2018
 *      Author: Manan Patel
 */

#ifndef BOARD_H_
#define BOARD_H_


#define RANDOM_NUMBER_SEED 42

#define NUM_MINES 10
#define NUM_TILES_X 9
#define NUM_TILES_Y 9

struct Cell {
	int neighbours;
	int revealed;
	int mined;
};

struct Board {
	struct Cell tiles[NUM_TILES_Y][NUM_TILES_X];
	int remaining;
	int running;
};

struct Board *create_board();
void print_board(struct Board *);
int count_neighbours(struct Board *board, int row, int col);
int reveal_tile(struct Board *board, int *coords);
void reveal_all(struct Board *board);
int flag_tile(struct Board *board, int *coords);

void init_board(struct Board *board);

#endif /* BOARD_H_ */
