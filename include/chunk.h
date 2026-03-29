#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>
#include "value.h"
#include "line_encoding.h"

typedef enum
{
	OP_CONSTANT,
	OP_CONSTANT_LONG,
	OP_DEFINE_GLOBAL,
	OP_DEFINE_GLOBAL_LONG,
	OP_GET_GLOBAL,
	OP_GET_GLOBAL_LONG,
	OP_SET_GLOBAL,
	OP_SET_GLOBAL_LONG,
	OP_NIL,
	OP_TRUE,
	OP_FALSE,
	OP_EQUAL,
	OP_GREATER,
	OP_LESS,
	OP_NOT,
	OP_NEGATE,
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_POP,
	OP_PRINT,
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
void writeGlobal(Chunk *chunk, OpCode type, int global, int line);
void writeConstant(Chunk *chunk, Value value, int line);

int makeConstant(Chunk *chunk, Value value);

#endif
