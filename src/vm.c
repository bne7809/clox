#include <stdio.h>
#include "vm.h"

#define DEBUG_TRACE_EXECUTION

static VM vm;

void initVM()
{
}

void freeVM()
{
}

static InterpretResult run()
{
#ifdef DEBUG_TRACE_EXECUTION
	int curr_line = 0;
	int total = 0;
	int lineDataIndex = 0;
#endif
	while (1)
	{
#ifdef DEBUG_TRACE_EXECUTION
		int offset = vm.ip - vm.chunk->code;
		printf("%04d ", offset);
		while (total <= offset)
		{
			total += vm.chunk->lineData.data[lineDataIndex++].num;
		}
		int line = vm.chunk->lineData.data[lineDataIndex - 1].line;
		if (offset > 0 && line == curr_line)
		{
			printf("   | ");
		} else {
			printf("%4d ", line);
			curr_line = line;
		}
#endif
		uint8_t inst = *vm.ip++;
		switch (inst)
		{
		case OP_CONSTANT:
		{
			int const_index = *vm.ip++;
			Value val = vm.chunk->constants.values[const_index];
#ifdef DEBUG_TRACE_EXECUTION
			printf("%-16s %4d '", "OP_CONSTANT", const_index);
			printValue(val);
			printf("'\n");
#endif
			printValue(val);
			printf("\n");
			break;
		}
		case OP_CONSTANT_LONG:
		{
			uint8_t c1 = *vm.ip++;
			uint8_t c2 = *vm.ip++;
			uint8_t c3 = *vm.ip++;
			int const_index = (c3 << 16) | (c2 << 8) | c1;
			Value val = vm.chunk->constants.values[const_index];
#ifdef DEBUG_TRACE_EXECUTION
			printf("%-16s %4d '", "OP_CONSTANT_LONG", const_index);
			printValue(val);
			printf("'\n");
#endif
			printValue(val);
			printf("\n");
			break;
		}
		case OP_RETURN:
#ifdef DEBUG_TRACE_EXECUTION
			printf("OP_RETURN\n");
#endif
			return INTERPRET_OK;
		}
	}
}

InterpretResult interpret(Chunk *chunk)
{
	vm.chunk = chunk;
	vm.ip = chunk->code;
	return run();
}
