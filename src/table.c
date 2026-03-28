#include "memory.h"
#include "table.h"

#define TABLE_MAX_LOAD	0.75

void initTable(Table *table)
{
	table->count = 0;
	table->capacity = 0;
	table->entries = NULL;
}

void freeTable(Table *table)
{
	FREE_ARRAY(table->entries);
	initTable(table);
}

void tableAddAll(Table *from, Table *to)
{
	for (int i = 0; i < from->capacity; ++i)
	{
		Entry *entry = &from->entries[i];
		if (entry->key != NULL)
		{
			tableSet(to, entry->key, entry->value);
		}
	}
}

static Entry *findEntry(Entry *entries, int capacity, ObjString *key)
{
	uint32_t index = key->hash % capacity;
	Entry *tombstone = NULL;
	while (1)
	{
		Entry *entry = &entries[index];
		if (entry->key == NULL)
		{
			if (entry->value.type == VAL_NIL) return tombstone != NULL ? tombstone : entry;
			else if (tombstone == NULL) tombstone = entry;
		} else if (entry->key == key) {
			return entry;
		}
		index = (index + 1) % capacity;
	}
}

static void adjustCapacity(Table *table, int capacity)
{
	Entry *entries = ALLOCATE(Entry, capacity);
	for (int i = 0; i < capacity; ++i)
	{
		entries[i].key = NULL;
		entries[i].value = (Value){ VAL_NIL, {.num = 0} };
	}
	table->count = 0;
	for (int i = 0; i < table->capacity; ++i)
	{
		Entry *entry = &table->entries[i];
		if (entry->key == NULL) continue;
		Entry *dest = findEntry(entries, capacity, entry->key);
		dest->key = entry->key;
		dest->value = entry->value;
		++table->count;
	}
	FREE_ARRAY(table->entries);
	table->entries = entries;
	table->capacity = capacity;
}

int tableSet(Table *table, ObjString *key, Value value)
{
	if (table->count + 1 > table->capacity * TABLE_MAX_LOAD)
	{
		int capacity = GROW_CAPACITY(table->capacity);
		adjustCapacity(table, capacity);
	}
	Entry *entry = findEntry(table->entries, table->capacity, key);
	int isNewKey = !entry->key;
	if (isNewKey && entry->value.type == VAL_NIL) ++table->count;
	entry->key = key;
	entry->value = value;
	return isNewKey;
}

int tableGet(Table *table, ObjString *key, Value *value)
{
	if (table->count == 0) return 0;
	Entry *entry = findEntry(table->entries, table->capacity, key);
	if (entry->key == NULL) return 0;
	*value = entry->value;
	return 1;
}

int tableDelete(Table *table, ObjString *key)
{
	if (table->count == 0) return 0;
	Entry *entry = findEntry(table->entries, table->capacity, key);
	if (entry->key == NULL) return 0;
	entry->key = NULL;
	entry->value = (Value){ VAL_BOOL, {.boolean = 1} };
	return 1;
}
