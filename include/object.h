#ifndef OBJECT_H
#define OBJECT_H

#include <stdint.h>
#include "chunk.h"

typedef enum
{
	OBJ_STRING,
	OBJ_FUNCTION
} ObjType;

typedef struct Obj
{
	ObjType type;
	struct Obj *next;
} Obj;

typedef struct
{
	Obj obj;
	uint32_t hash;
	int length;
	char str[];
} ObjString;

typedef struct
{
	Obj obj;
	int arity;
	Chunk chunk;
	ObjString *name;
} ObjFunction;

ObjFunction *newFunction();

ObjString *copyString(const char *str, int length);
ObjString *concat(ObjString *a, ObjString *b);

int objsEqual(Obj *a, Obj *b);

void freeObject(Obj *obj);
void printObject(Obj *obj);

#endif
