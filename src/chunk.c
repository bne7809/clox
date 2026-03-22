#include "chunk.h"
#include "memory.h"

void initChunk(Chunk *chunk)
{
	chunk->capacity = 0;
	chunk->count = 0;
	chunk->code = NULL;
	chunk->lines = NULL;
	initValueArray(&chunk->constants);
}

void writeChunk(Chunk *chunk, uint8_t byte, int line)
{
	if (chunk->capacity == chunk->count)
	{
		chunk->capacity = GROW_CAPACITY(chunk->capacity);
		chunk->code = GROW_ARRAY(uint8_t, chunk->code, chunk->capacity);
		chunk->lines = GROW_ARRAY(int, chunk->lines, chunk->capacity);
	}
	chunk->code[chunk->count] = byte;
	chunk->lines[chunk->count] = line;
	++chunk->count;
}

void freeChunk(Chunk *chunk)
{
	FREE_ARRAY(chunk->code);
	FREE_ARRAY(chunk->lines);
	freeValueArray(&chunk->constants);
	initChunk(chunk);
}

int addConstant(Chunk *chunk, Value value)
{
	writeValueArray(&chunk->constants, value);
	return chunk->constants.count - 1;
}
