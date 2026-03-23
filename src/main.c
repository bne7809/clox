#include "debug.h"
#include "vm.h"

int main()
{
	initVM();
	Chunk chunk;
	initChunk(&chunk);
	writeConstant(&chunk, 1.2, 123);
	writeChunk(&chunk, OP_RETURN, 123);
	disassembleChunk(&chunk, "test chunk");
	interpret(&chunk);
	freeVM();
	freeChunk(&chunk);
}
