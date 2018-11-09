#ifndef LEADERBOARD_H_
#define LEADERBOARD_H_

#define LEADERBOARD_SIZE 20
#define LEADERBOARD_NAME_LENGTH 40
struct Record {
	char name[LEADERBOARD_NAME_LENGTH];
	int seconds;
	int played;
	int won;
};

void sort_records(struct Record *);
struct  Record* create_leaderboard();
int compare_records(const void *record1, const void *record2);
void insert(struct Record *records, char *name, int seconds, int won);

#endif /* LEADERBOARD_H_ */
