#ifndef OBJECT_H
#define OBJECT_H

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
	int length;
	char str[];
} ObjString;

ObjString *copyString(const char *str, int length, Obj **head);
ObjString *concat(ObjString *a, ObjString *b, Obj **head);

int objsEqual(Obj *a, Obj *b);

void freeObject(Obj *obj);
void printObject(Obj *obj);

#endif
