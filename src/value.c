#include <stdio.h>
#include "memory.h"
#include "value.h"

void initValueArray(ValueArray *array)
{
	array->capacity = 0;
	array->count = 0;
	array->values = NULL;
}

void freeValueArray(ValueArray *array)
{
	FREE_ARRAY(array->values);
	initValueArray(array);
}

void writeValueArray(ValueArray *array, Value value)
{
	if (array->capacity == array->count)
	{
		array->capacity = GROW_CAPACITY(array->capacity);
		array->values = GROW_ARRAY(Value, array->values, array->capacity);
	}
	array->values[array->count++] = value;
}

void printValue(Value value)
{
	switch(value.type)
	{
	case VAL_NUMBER: printf("%g", value.num); break;
	case VAL_BOOL: printf("%s", value.boolean ? "true" : "false"); break;
	case VAL_NIL: printf("nil");
	default: return;
	}
}
