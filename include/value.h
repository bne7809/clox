#ifndef VALUE_H
#define VALUE_H

#include <stddef.h>

struct Obj;

typedef enum
{
	VAL_NUMBER,
	VAL_BOOL,
	VAL_OBJ,
	VAL_NIL
} ValueType;

typedef struct
{
	ValueType type;
	union
	{
		double num;
		size_t boolean;
		struct Obj *obj;
	};
} Value;

typedef struct
{
	int capacity;
	int count;
	Value *values;
} ValueArray;

void initValueArray(ValueArray *array);
void freeValueArray(ValueArray *array);
void writeValueArray(ValueArray *array, Value value);
void printValue(Value value);

int valuesEqual(Value a, Value b);

#endif
