#include <stdio.h>
#include "chunk.h"
#include "scanner.h"

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

void disassembleChunk(Chunk *chunk, const char *name)
{
	printf("== BEGIN %s ==\n\n", name);
	int offset = 0;
	int curr_line = 0;
	int total = 0;
	int lineDataIndex = 0;
	while (offset < chunk->count)
	{
		printf("%04d ", offset);
		while (total <= offset)
		{
			total += chunk->lineData.data[lineDataIndex++].num;
		}
		int line = chunk->lineData.data[lineDataIndex - 1].line;
		if (offset > 0 && line == curr_line)
		{
			printf("   | ");
		} else {
			printf("%4d ", line);
			curr_line = line;
		}
		uint8_t inst = chunk->code[offset];
		switch (inst)
		{
		case OP_CONSTANT_LONG:
			offset = constantLongInstruction("OP_CONSTANT_LONG", chunk, offset);
			break;
		case OP_CONSTANT:
			offset = constantInstruction("OP_CONSTANT", chunk, offset);
			break;
		case OP_NIL:
			offset = simpleInstruction("OP_NIL", offset);
			break;
		case OP_TRUE:
			offset = simpleInstruction("OP_TRUE", offset);
			break;
		case OP_FALSE:
			offset = simpleInstruction("OP_FALSE", offset);
			break;
		case OP_ADD:
			offset = simpleInstruction("OP_ADD", offset);
			break;
		case OP_SUBTRACT:
			offset = simpleInstruction("OP_SUBTRACT", offset);
			break;
		case OP_MULTIPLY:
			offset = simpleInstruction("OP_MULTIPLY", offset);
			break;
		case OP_DIVIDE:
			offset = simpleInstruction("OP_DIVIDE", offset);
			break;
		case OP_NEGATE:
			offset = simpleInstruction("OP_NEGATE", offset);
			break;
		case OP_RETURN:
			offset = simpleInstruction("OP_RETURN", offset);
			break;
		default:
			printf("Unknown opcode %d\n", inst);
			++offset;
			break;
		}
	}
	printf("\n== END %s ==\n\n", name);
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

void printTokens(const char *source)
{
	initScanner(source);
	int line = -1;
	while (1)
	{
		Token t = scanToken();
		if (t.line != line)
		{
			line = t.line;
			printf("%4d ", line);
		} else {
			printf("   | ");
		}
		printf("%-18s '%.*s'\n", tokenToString(t.type), t.length, t.start);
		if (t.type == TOKEN_EOF) break;
	}
}
