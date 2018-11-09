/*
 * protocol.h
 *
 *  Created on: Oct 17, 2018
 *      Author: Manan Patel
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#define START_GAME 1
#define BYE 2

#define REVEAL_TILE 20
#define FLAG_TILE 30

#define MINE_TILE 40
#define EMPTY_TILE 50

#define AUTH_USER 10

#define WINNER 500
#define UPDATE_BOARD 200
#define LEADERS 1000

#define OK 100
#define ERROR 101


void write_int(long sock, int16_t status);

#endif /* PROTOCOL_H_ */
