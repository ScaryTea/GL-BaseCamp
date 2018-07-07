#include "lee.h"
#include "queue.h"

int dx[4] = {1, 0, -1, 0};
int dy[4] = {0, 1, 0, -1};

bool markNeighbours(struct CellsData *pCellsData, struct Cell *cell, struct Queue *queue) {

	struct Cell **cells = pCellsData->cells;
	
	int value = cell->value + 1;
	int h = pCellsData->h;
	int w = pCellsData->w;

	for (int k = 0; k < 4; k++) {
		int i = cell->x + dx[k];
		int j = cell->y + dy[k];
		if (i >= 0 && i < h && j >= 0 && j < w && 
			!pCellsData->cells[i][j].marked && cells[i][j].value != WALL) {
			cells[i][j].marked = true;
			cells[i][j].value = value;
			enqueue(queue, &cells[i][j]);
		}
	}
}

struct Cell *nextNeighbour(struct CellsData *pCellsData, struct Cell *cell)
{
	int h = pCellsData->h;
	int w = pCellsData->w;

	for (int k = 0; k < 4; k++) {
		int i = cell->x + dx[k];
		int j = cell->y + dy[k];
		if (i >= 0 && i < h && j >= 0 && j < w && 
			(cell->value - pCellsData->cells[i][j].value == 1))
			return &(pCellsData->cells[i][j]);
	}
}

int lee(struct CellsData *pCellsData)
{
	struct Queue *pQ = constructQueue();
	pCellsData->start->marked = true;
	enqueue(pQ, pCellsData->start);

	do {
		struct Cell *cell;
		struct Node *node; 
		while((node = dequeue(pQ)) != NULL) {
			cell = node->cell;
			markNeighbours(pCellsData, cell, pQ);
		}
	} while (!(pCellsData->finish)->marked && !isEmpty(pQ));

/*
	for (int i = 0; i < pCellsData->h; i++) {
		for (int j = 0; j < pCellsData->w; j++) 
			printf("%4d",pCellsData->cells[i][j].value);
		printf("\n");
	}
*/
	if(pCellsData->finish->marked) {
		struct Cell *cell = pCellsData->finish;
		int len = cell->value;

		//printf("(%d;%d)",cell->x,cell->y);
		for (int i = len - 1; i >= 0; i--) {
			cell = nextNeighbour(pCellsData, cell);
			//printf("-(%d;%d)",cell->x,cell->y);
		}
		//printf("\n");
		destructQueue(pQ);
		return 0;
	} else {
		destructQueue(pQ);
		//printf("No path\n");
		return 1;
	}
}
