#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "object.h"

ObjString *copyString(const char *str, int length, Obj **head)
{
	ObjString *ret = malloc(sizeof(ObjString) + length + 1);
	ret->obj.type = OBJ_STRING;
	ret->obj.next = *head;
	ret->length = length;
	memcpy(ret->str, str, length);
	ret->str[length] = '\0';
	*head = (Obj *)ret;
	return ret;
}

ObjString *concat(ObjString *a, ObjString *b, Obj **head)
{
	ObjString *ret = malloc(sizeof(ObjString) + a->length + b->length + 1);
	ret->obj.type = OBJ_STRING;
	ret->obj.next = *head;
	ret->length = a->length + b->length;
	memcpy(ret->str, a->str, a->length);
	memcpy(ret->str + a->length, b->str, b->length);
	ret->str[ret->length] = '\0';
	*head = (Obj *)ret;
	return ret;
}

int objsEqual(Obj *a, Obj *b)
{
	if (a->type != b->type) return 0;
	switch (a->type)
	{
	case OBJ_STRING:
	{
		ObjString *astr = (ObjString *)a;
		ObjString *bstr = (ObjString *)b;
		return astr->length == bstr->length && !memcmp(astr->str, bstr->str, astr->length);
	}
	default: return 0;
	}
}

void freeObject(Obj *obj)
{
	switch (obj->type)
	{
	case OBJ_STRING: free(obj); break;
	default: return;
	}
}

void printObject(Obj *obj)
{
	switch (obj->type)
	{
	case OBJ_STRING: printf("%s", ((ObjString *)obj)->str); break;
	default: return;
	}
}
