#include "debug.h"

int main()
{
	Chunk chunk;
	initChunk(&chunk);
	writeConstant(&chunk, 1.2, 123);
	writeChunk(&chunk, OP_RETURN, 123);
	disassembleChunk(&chunk, "test chunk");
	freeChunk(&chunk);
}
