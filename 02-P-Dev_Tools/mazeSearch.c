#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lee.h"

unsigned int getSeed()
{
	FILE *fp_rand;
	if ((fp_rand = fopen("/dev/random", "rb")) == NULL) 
		goto exc_fopen;
	unsigned int seed;
	fread(&seed, sizeof(seed), 1, fp_rand);
	if (ferror(fp_rand)) 
		goto exc_fread;
	fclose(fp_rand);
	return 0;	

	exc_fopen:
	printf("Error: could not open /dev/random\n");
	return 1;

	exc_fread:
	printf("Error: read I/O mismatch\n");
	fclose(fp_rand);
	return 1;
}

double timespec_diff(struct timespec *stop, struct timespec *start)
{
	double diff = difftime(stop->tv_sec, start->tv_sec);
	diff *= 1000;
	diff += (stop->tv_nsec - start->tv_nsec) / 1e6;
	return diff;
}

int main(int argc, char* argv[])
{
	int h = 10;
	int w = 10;
	
	if (argc == 1) 
		goto exc_noargs;
	if (argc == 3) {
		h = atol(argv[1]);
		w = atol(argv[2]);
		if (h <= 0 || w <= 0) 
			goto exc_wrongargs;
	}
	if (argc > 3) 
		goto exc_argtoomuch;

	struct Cell **cells = malloc(h * sizeof(*cells));
	if (cells == NULL)
		goto exc_malloc;
	
	srand(getSeed());

	for (int i = 0; i < h; i++) {
		cells[i] = malloc(w * sizeof(struct Cell));
		if (cells[i] == NULL)
			goto exc_malloc;
		for (int j = 0; j < w; j++) {
			int val = rand();
			cells[i][j].value = (val > RAND_MAX/4) ? CELL : WALL;
			cells[i][j].marked = false;
			cells[i][j].x = i;
			cells[i][j].y = j;
		}
	}

	struct CellsData cellsData;
	cells[0][0].value = 0;
	cells[h-1][w-1].value = 0;
	cellsData.start = &cells[0][0];
	cellsData.finish = &cells[h-1][w-1];
	cellsData.cells = cells;
	cellsData.h = h;
	cellsData.w = w;
	
	struct timespec time_now, time_after;

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_now);
	int ret = lee(&cellsData);
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_after);
	if (!ret)
		printf("%d-%g\n", h*w, timespec_diff(&time_after, &time_now));

	for (int i = 0; i < h; i++)
		free(cells[i]);
	free(cells);

	return 0;

	/* exceptions handling */
	exc_noargs:
	printf("Error: no arguments\n");
	goto exc_usage;

	exc_wrongargs:
	printf("Error: incorrect arguments\n");
	goto exc_usage;

	exc_argtoomuch:
	printf("Error: too much arguments\n");
	goto exc_usage;

	exc_usage:
	printf("Usage:\n%s <height> <width> \n", argv[0]);
	return 1;

	exc_malloc:
	printf("Error: could not allocate memory\n");
	return 1;
}







