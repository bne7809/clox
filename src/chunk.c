#include "memory.h"
#include "chunk.h"

void initChunk(Chunk *chunk)
{
	chunk->capacity = 0;
	chunk->count = 0;
	chunk->code = NULL;
	initLineEncoding(&chunk->lineData);
	initValueArray(&chunk->constants);
}

void writeChunk(Chunk *chunk, uint8_t byte, int line)
{
	if (chunk->capacity == chunk->count)
	{
		chunk->capacity = GROW_CAPACITY(chunk->capacity);
		chunk->code = GROW_ARRAY(uint8_t, chunk->code, chunk->capacity);
	}
	chunk->code[chunk->count++] = byte;
	writeLineEncoding(&chunk->lineData, line);
}

void freeChunk(Chunk *chunk)
{
	FREE_ARRAY(chunk->code);
	freeLineEncoding(&chunk->lineData);
	freeValueArray(&chunk->constants);
	initChunk(chunk);
}

void writeGlobal(Chunk *chunk, OpCode type, int global, int line)
{
	if (global > 255)
	{
		writeChunk(chunk, type + 1, line);
		writeChunk(chunk, global & 255, line);
		writeChunk(chunk, (global >> 8) & 255, line);
		writeChunk(chunk, (global >> 16) & 255, line);
	} else {
		writeChunk(chunk, type, line);
		writeChunk(chunk, global, line);
	}
}

int makeConstant(Chunk *chunk, Value value)
{
	writeValueArray(&chunk->constants, value);
	return chunk->constants.count - 1;
}

void writeConstant(Chunk *chunk, Value value, int line)
{
	int const_index = makeConstant(chunk, value);
	if (const_index > 255)
	{
		writeChunk(chunk, OP_CONSTANT_LONG, line);
		writeChunk(chunk, const_index & 255, line);
		writeChunk(chunk, (const_index >> 8) & 255, line);
		writeChunk(chunk, (const_index >> 16) & 255, line);
	} else {
		writeChunk(chunk, OP_CONSTANT, line);
		writeChunk(chunk, const_index, line);
	}
}
