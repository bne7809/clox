#include <stdio.h>
#include "chunk.h"

typedef struct
{
	int curr_line;
	int total;
	int index;
} DisasmState;

static int simpleInstruction(const char *name, int offset)
{
	printf("%s\n", name);
	return offset + 1;
}

static int constantInstruction(const char *name, Chunk *chunk, int offset)
{
	uint8_t const_index = chunk->code[offset + 1];
	printf("%-16s %4d '", name, const_index);
	printValue(chunk->constants.values[const_index]);
	printf("'\n");
	return offset + 2;
}

static int constantLongInstruction(const char *name, Chunk *chunk, int offset)
{
	uint8_t c1 = chunk->code[offset + 1];
	uint8_t c2 = chunk->code[offset + 2];
	uint8_t c3 = chunk->code[offset + 3];
	int const_index = (c3 << 16) | (c2 << 8) | c1;
	printf("%-16s %4d '", name, const_index);
	printValue(chunk->constants.values[const_index]);
	printf("'\n");
	return offset + 4;
}

static int disassembleInstruction(Chunk *chunk, int offset, DisasmState *state)
{
	printf("%04d ", offset);
	while (state->index < chunk->lineData.count)
	{
		if (state->total > offset) break;
		state->total += chunk->lineData.data[state->index++].num;
	}
	int line = chunk->lineData.data[state->index - 1].line;
	if (offset > 0 && line == state->curr_line)
	{
		printf("   | ");
	} else {
		printf("%4d ", line);
		state->curr_line = line;
	}
	uint8_t inst = chunk->code[offset];
	switch (inst)
	{
	case OP_CONSTANT_LONG:
		return constantLongInstruction("OP_CONSTANT_LONG", chunk, offset);
	case OP_CONSTANT:
		return constantInstruction("OP_CONSTANT", chunk, offset);
	case OP_RETURN:
		return simpleInstruction("OP_RETURN", offset);
	default:
		printf("Unknown opcode %d\n", inst);
		return offset + 1;
	}
}

void disassembleChunk(Chunk *chunk, const char *name)
{
	printf("== %s ==\n", name);
	int offset = 0;
	DisasmState state = {};
	while (offset < chunk->count)
	{
		offset = disassembleInstruction(chunk, offset, &state);
	}
}

int getLine(Chunk *chunk, int index)
{
	int n = 0;
	int i;
	for (i = 0; i < chunk->lineData.count; ++i)
	{
		n += chunk->lineData.data[i].num;
		if (n > index) break;
	}
	return chunk->lineData.data[i].line;
}
