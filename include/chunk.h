#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>
#include "value.h"
#include "line_encoding.h"

typedef enum
{
	OP_CONSTANT,
	OP_CONSTANT_LONG,
	OP_NEGATE,
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_RETURN
} OpCode;

typedef struct
{
	int capacity;
	int count;
	uint8_t *code;
	LineEncoding lineData;
	ValueArray constants;
} Chunk;

void initChunk(Chunk *chunk);
void freeChunk(Chunk *chunk);
void writeChunk(Chunk *chunk, uint8_t byte, int line);
void writeConstant(Chunk *chunk, Value value, int line);

#endif
