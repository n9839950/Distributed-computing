#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leaderboard.h"

/**
 * Creates a leader board
 *
 * It's in the form of an array, that makes it possible for us to use the
 * system sort.
 */
struct  Record* create_leaderboard() {
	struct Record *leaders = malloc(sizeof(struct Record) * (LEADERBOARD_SIZE +1));
	int i;
	for(i = 0 ; i < LEADERBOARD_SIZE ; i++) {
		leaders[i].name[0] = '\0';
		leaders[i].seconds = 2147483647;
	}

	return leaders;
}

/**
 * Sort and clear out the last item.
 * That way a new item gets added into the array only if it is of higher value
 * at least one of the existing records.
 */
void sort_records(struct Record *records) {
	qsort(records, LEADERBOARD_SIZE + 1, sizeof(struct Record), compare_records);
	records[LEADERBOARD_SIZE].name[LEADERBOARD_SIZE] = '\0';
}

/**
 * Compare two records.
 */
int compare_records(const void *record1, const void *record2) {
	struct Record *r1 = (struct Record *) record1;
	struct Record *r2 = (struct Record *) record2;
	int cmp;

	cmp = r1->seconds - r2->seconds;
	if(cmp == 0) {
		return r2->played - r1->played  ;
	}
	return cmp;

}

void insert(struct Record *records, char *name, int seconds, int won) {
	int i = LEADERBOARD_SIZE;
//	for(i = 0 ; i < LEADERBOARD_SIZE ; i ++) {
//		if(strcmp(name, records[i].name) == 0) {
//			if(records[i].seconds > seconds && won) {
//				records[i].seconds = seconds;
//				records[i].played++;
//				records[i].won++;
//			}
//			else {
//				records[i].played++;
//				if(won) {
//					records[i].won++;
//				}
//			}
//			sort_records(records);
//			return;
//		}
//	}

	strncpy(records[i].name, name, LEADERBOARD_NAME_LENGTH);
	records[i].seconds = seconds;
	records[i].played = 1;
	if(won) {
		records[i].won = 1;
	}
	else {
		records[i].won = 0;
	}

	sort_records(records);
}
