#ifndef OBJECT_H
#define OBJECT_H

#include <stdint.h>

typedef enum
{
	OBJ_STRING
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

ObjString *copyString(const char *str, int length);
ObjString *concat(ObjString *a, ObjString *b);

int objsEqual(Obj *a, Obj *b);

void freeObject(Obj *obj);
void printObject(Obj *obj);

#endif
