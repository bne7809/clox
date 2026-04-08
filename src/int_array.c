#include <stddef.h>
#include "memory.h"
#include "int_array.h"

void initIntArray(IntArray *array)
{
	array->capacity = 0;
	array->count = 0;
	array->values = NULL;
}

void freeIntArray(IntArray *array)
{
	FREE_ARRAY(array->values);
	initIntArray(array);
}

void addIntArray(IntArray *array, int value)
{
	if (array->capacity == array->count)
	{
		array->capacity = GROW_CAPACITY(array->capacity);
		array->values = GROW_ARRAY(int, array->values, array->capacity);
	}
	array->values[array->count++] = value;
}

