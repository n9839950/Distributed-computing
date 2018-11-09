#ifndef SERVER_H_
#define SERVER_H_

#include "board.h"


#define PASSWORD_FILE "Authentication.txt"
#define VERBOSE 1

int authenticate(int sock, char *user, char *pass);
void * listener(void *data);
int * get_coords(int);
void update_board(long sock, struct Board *board);

#endif /* SERVER_H_ */
