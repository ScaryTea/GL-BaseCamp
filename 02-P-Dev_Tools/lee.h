#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifndef LEE_H 
#define LEE_H

struct Cell {
	int value;
	bool marked;
	int x;
	int y;
}; 

struct CellsData{
	struct Cell **cells;
	int w;
	int h;
	struct Cell *start;
	struct Cell *finish;
};

extern enum { WALL = -2, CELL = 0 };

extern int lee(struct CellsData *pCellsData);

#endif

