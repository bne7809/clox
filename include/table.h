#ifndef TABLE_H
#define TABLE_H

#include "value.h"

typedef struct
{
	ObjString *key;
	Value value;
} Entry;

typedef struct
{
	int count;
	int capacity;
	Entry *entries;
} Table;

void initTable(Table *table);
void freeTable(Table *table);
void tableAddAll(Table *from, Table *to);

int tableSet(Table *table, ObjString *key, Value value);
int tableGet(Table *table, ObjString *key, Value *value);
int tableDelete(Table *table, ObjString *key);

#endif
