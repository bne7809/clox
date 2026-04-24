#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "vm.h"
#include "memory.h"
#include "object.h"

ObjFunction *newFunction()
{
	ObjFunction *ret = ALLOCATE(ObjFunction, 1);
	ret->obj.type = OBJ_FUNCTION;
	ret->obj.next = vm.objects;
	vm.objects = (Obj *)ret;
	ret->arity = 0;
	initChunk(&ret->chunk);
	ret->name = NULL;
	return ret;
}

static uint32_t hashString(const char *str, int length)
{
	uint32_t hash = 2166136261u;
	for (int i = 0; i < length; ++i)
	{
		hash ^= (uint8_t)str[i];
		hash *= 16777619;
	}
	return hash;
}

static ObjString *tableFindString(Table *table, const char *str, int length, uint32_t hash)
{
	if (table->count == 0) return NULL;
	uint32_t index = hash % table->capacity;
	while (1)
	{
		Entry *entry = &table->entries[index];
		if (entry->key == NULL)
		{
			if (entry->value.type == VAL_NIL) return NULL;
		} else if (entry->key->length == length && entry->key->hash == hash && !memcmp(entry->key->str, str, length)) {
			return entry->key;
		}
		index = (index + 1) % table->capacity;
	}
}

ObjString *copyString(const char *str, int length)
{
	uint32_t hash = hashString(str, length);
	ObjString *interned = tableFindString(&vm.strings, str, length, hash);
	if (interned != NULL) return interned;
	ObjString *ret = malloc(sizeof(ObjString) + length + 1);
	ret->obj.type = OBJ_STRING;
	ret->obj.next = vm.objects;
	ret->hash = hash;
	ret->length = length;
	memcpy(ret->str, str, length);
	ret->str[length] = '\0';
	vm.objects = (Obj *)ret;
	tableSet(&vm.strings, ret, (Value){ VAL_NIL, {.num = 0} });
	return ret;
}

ObjString *concat(ObjString *a, ObjString *b)
{
	ObjString *ret = malloc(sizeof(ObjString) + a->length + b->length + 1);
	ret->length = a->length + b->length;
	memcpy(ret->str, a->str, a->length);
	memcpy(ret->str + a->length, b->str, b->length);
	ret->str[ret->length] = '\0';
	uint32_t hash = hashString(ret->str, ret->length);
	ObjString *interned = tableFindString(&vm.strings, ret->str, ret->length, hash);
	if (interned != NULL)
	{
		free(ret);
		return interned;
	}
	ret->obj.type = OBJ_STRING;
	ret->obj.next = vm.objects;
	ret->hash = hash;
	vm.objects = (Obj *)ret;
	return ret;
}

int objsEqual(Obj *a, Obj *b)
{
	if (a->type != b->type) return 0;
	switch (a->type)
	{
	case OBJ_STRING: return a == b;
	default: return 0;
	}
}

void freeObject(Obj *obj)
{
	switch (obj->type)
	{
	case OBJ_FUNCTION:
	{
		ObjFunction *func = (ObjFunction *)obj;
		freeChunk(&func->chunk);
		free(func->name);
		free(func);
		break;
	}
	case OBJ_STRING: free(obj); break;
	default: return;
	}
}

void printObject(Obj *obj)
{
	switch (obj->type)
	{
	case OBJ_FUNCTION:
	{
		ObjFunction *func = (ObjFunction *)obj;
		if (func->name == NULL)
		{
			printf("<script>");
			break;
		}
		printf("<fn %s>", func->name->str);
		break;
	}
	case OBJ_STRING: printf("%s", ((ObjString *)obj)->str); break;
	default: return;
	}
}
