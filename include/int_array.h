#ifndef INT_ARRAY_H
#define INT_ARRAY_H

typedef struct
{
	int capacity;
	int count;
	int *values;
} IntArray;

void initIntArray(IntArray *array);
void freeIntArray(IntArray *array);
void addIntArray(IntArray *array, int i);

#endif
