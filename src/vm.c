#include "compiler.h"
#include "vm.h"

#define DEBUG_TRACE_EXECUTION

static VM vm;

void initVM()
{
	vm.stackTop = vm.stack;
}

void freeVM()
{
}

#if 0
static void push(Value val)
{
	*vm.stackTop++ = val;
}

static Value pop()
{
	return *--vm.stackTop;
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
		printf("          ");
		for (Value *v = vm.stack; v < vm.stackTop; ++v)
		{
			printf("[ ");
			printValue(*v);
			printf(" ]");
		}
		printf("\n");
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
			push(val);
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
			push(val);
			break;
		}
		case OP_ADD:
		{
#ifdef DEBUG_TRACE_EXECUTION
			printf("OP_ADD\n");
#endif
			Value a = pop();
			Value b = pop();
			push(b + a);
			break;
		}
		case OP_SUBTRACT:
		{
#ifdef DEBUG_TRACE_EXECUTION
			printf("OP_SUBTRACT\n");
#endif
			Value a = pop();
			Value b = pop();
			push(b - a);
			break;
		}
		case OP_MULTIPLY:
		{
#ifdef DEBUG_TRACE_EXECUTION
			printf("OP_MULTIPLY\n");
#endif
			Value a = pop();
			Value b = pop();
			push(b * a);
			break;
		}
		case OP_DIVIDE:
		{
#ifdef DEBUG_TRACE_EXECUTION
			printf("OP_DIVIDE\n");
#endif
			Value a = pop();
			Value b = pop();
			push(b / a);
			break;
		}
		case OP_NEGATE:
#ifdef DEBUG_TRACE_EXECUTION
			printf("OP_NEGATE\n");
#endif
			vm.stackTop[-1] = -vm.stackTop[-1];
			break;
		case OP_RETURN:
#ifdef DEBUG_TRACE_EXECUTION
			printf("OP_RETURN\n");
#endif
			printValue(pop());
			printf("\n");
			return INTERPRET_OK;
		}
	}
}
#endif

InterpretResult interpret(const char *source)
{
	compile(source);
	return INTERPRET_OK;
}
