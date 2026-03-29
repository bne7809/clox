#include <stdio.h>
#include "compiler.h"
#include "vm.h"

#define DEBUG_TRACE_EXECUTION

#ifdef DEBUG_TRACE_EXECUTION
#include "debug.h"
#endif

VM vm;

void initVM()
{
	vm.stackTop = vm.stack;
	vm.objects = NULL;
	initTable(&vm.globals);
	initTable(&vm.strings);
}

void freeVM()
{
	freeTable(&vm.globals);
	freeTable(&vm.strings);
	Obj *o = vm.objects;
	while (o)
	{
		Obj *tmp = o->next;
		freeObject(o);
		o = tmp;
	}
}

static void push(Value val)
{
	*vm.stackTop++ = val;
}

static Value pop()
{
	return *--vm.stackTop;
}

static void runtimeError(const char *str, int line)
{
	if (str) fprintf(stderr, "%s\n[line %d] in script\n", str, line);
	else fprintf(stderr, "\n[line %d] in script\n", line);
	vm.stackTop = vm.stack;
}

static int isFalsey(Value val)
{
	return (val.type == VAL_NIL) || (val.type == VAL_BOOL && !val.boolean);
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
		case OP_DEFINE_GLOBAL:
		{
			int const_index = *vm.ip++;
			ObjString *name = (ObjString *)vm.chunk->constants.values[const_index].obj;
#ifdef DEBUG_TRACE_EXECUTION
			printf("%-16s %4d '", "OP_DEFINE_GLOBAL", const_index);
			printObject((Obj *)name);
			printf("'\n");
#endif
			tableSet(&vm.globals, name, pop());
			break;
		}
		case OP_DEFINE_GLOBAL_LONG:
		{
			uint8_t c1 = *vm.ip++;
			uint8_t c2 = *vm.ip++;
			uint8_t c3 = *vm.ip++;
			int const_index = (c3 << 16) | (c2 << 8) | c1;
			ObjString *name = (ObjString *)vm.chunk->constants.values[const_index].obj;
#ifdef DEBUG_TRACE_EXECUTION
			printf("%-16s %4d '", "OP_DEFINE_GLOBAL_LONG", const_index);
			printObject((Obj *)name);
			printf("'\n");
#endif
			tableSet(&vm.globals, name, pop());
			break;
		}
		case OP_GET_GLOBAL:
		{
			int const_index = *vm.ip++;
			ObjString *name = (ObjString *)vm.chunk->constants.values[const_index].obj;
#ifdef DEBUG_TRACE_EXECUTION
			printf("%-16s %4d '", "OP_GET_GLOBAL", const_index);
			printObject((Obj *)name);
			printf("'\n");
#endif
			Value val;
			if (!tableGet(&vm.globals, name, &val))
			{
				fprintf(stderr, "Undefined variable '%s'.", name->str);
				runtimeError(NULL, curr_line);
				return INTERPRET_RUNTIME_ERROR;
			}
			push(val);
			break;
		}
		case OP_GET_GLOBAL_LONG:
		{
			uint8_t c1 = *vm.ip++;
			uint8_t c2 = *vm.ip++;
			uint8_t c3 = *vm.ip++;
			int const_index = (c3 << 16) | (c2 << 8) | c1;
			ObjString *name = (ObjString *)vm.chunk->constants.values[const_index].obj;
#ifdef DEBUG_TRACE_EXECUTION
			printf("%-16s %4d '", "OP_GET_GLOBAL_LONG", const_index);
			printObject((Obj *)name);
			printf("'\n");
#endif
			Value val;
			if (!tableGet(&vm.globals, name, &val))
			{
				fprintf(stderr, "Undefined variable '%s'.", name->str);
				runtimeError(NULL, curr_line);
				return INTERPRET_RUNTIME_ERROR;
			}
			push(val);
			break;
		}
		case OP_SET_GLOBAL:
		{
			int const_index = *vm.ip++;
			ObjString *name = (ObjString *)vm.chunk->constants.values[const_index].obj;
#ifdef DEBUG_TRACE_EXECUTION
			printf("%-16s %4d '", "OP_SET_GLOBAL", const_index);
			printObject((Obj *)name);
			printf("'\n");
#endif
			if (tableSet(&vm.globals, name, vm.stackTop[-1]))
			{
				fprintf(stderr, "Undefined variable '%s'.", name->str);
				runtimeError(NULL, curr_line);
				return INTERPRET_RUNTIME_ERROR;
			}
			break;
		}
		case OP_SET_GLOBAL_LONG:
		{
			uint8_t c1 = *vm.ip++;
			uint8_t c2 = *vm.ip++;
			uint8_t c3 = *vm.ip++;
			int const_index = (c3 << 16) | (c2 << 8) | c1;
			ObjString *name = (ObjString *)vm.chunk->constants.values[const_index].obj;
#ifdef DEBUG_TRACE_EXECUTION
			printf("%-16s %4d '", "OP_SET_GLOBAL_LONG", const_index);
			printObject((Obj *)name);
			printf("'\n");
#endif
			if (tableSet(&vm.globals, name, vm.stackTop[-1]))
			{
				fprintf(stderr, "Undefined variable '%s'.", name->str);
				runtimeError(NULL, curr_line);
				return INTERPRET_RUNTIME_ERROR;
			}
			break;
		}
		case OP_NIL:
		{
#ifdef DEBUG_TRACE_EXECUTION
			printf("OP_NIL\n");
#endif
			push((Value){ VAL_NIL, { .num = 0 } });
			break;
		}
		case OP_TRUE:
		{
#ifdef DEBUG_TRACE_EXECUTION
			printf("OP_TRUE\n");
#endif
			push((Value){ VAL_BOOL, { .boolean = 1 } });
			break;
		}
		case OP_FALSE:
		{
#ifdef DEBUG_TRACE_EXECUTION
			printf("OP_FALSE\n");
#endif
			push((Value){ VAL_BOOL, { .boolean = 0 } });
			break;
		}
		case OP_EQUAL:
		{
#ifdef DEBUG_TRACE_EXECUTION
			printf("OP_EQUAL\n");
#endif
			Value a = pop();
			Value b = pop();
			push((Value) { VAL_BOOL, {.boolean = valuesEqual(a, b)} });
			break;
		}
		case OP_GREATER:
		{
#ifdef DEBUG_TRACE_EXECUTION
			printf("OP_GREATER\n");
#endif
			Value a = pop();
			Value b = pop();
			if (a.type != VAL_NUMBER || b.type != VAL_NUMBER)
			{
				runtimeError("Operands must be numbers.", curr_line);
				return INTERPRET_RUNTIME_ERROR;
			}
			push((Value) { VAL_BOOL, {.boolean = (b.num > a.num)} });
			break;
		}
		case OP_LESS:
		{
#ifdef DEBUG_TRACE_EXECUTION
			printf("OP_LESS\n");
#endif
			Value a = pop();
			Value b = pop();
			if (a.type != VAL_NUMBER || b.type != VAL_NUMBER)
			{
				runtimeError("Operands must be numbers.", curr_line);
				return INTERPRET_RUNTIME_ERROR;
			}
			push((Value) { VAL_BOOL, {.boolean = (b.num < a.num)} });
			break;
		}
		case OP_ADD:
		{
#ifdef DEBUG_TRACE_EXECUTION
			printf("OP_ADD\n");
#endif
			Value a = pop();
			Value b = pop();
			if (a.type == VAL_NUMBER && b.type == VAL_NUMBER)
			{
				b.num += a.num;
				push(b);
				break;
			} else if (a.type == VAL_OBJ && b.type == VAL_OBJ) {
				if (a.obj->type == OBJ_STRING && b.obj->type == OBJ_STRING)
				{
					push((Value) { VAL_OBJ, {.obj = (Obj *)concat((ObjString *)b.obj, (ObjString *)a.obj)} });
					break;
				}
			}
			runtimeError("Operands must be numbers or strings.", curr_line);
			return INTERPRET_RUNTIME_ERROR;
		}
		case OP_SUBTRACT:
		{
#ifdef DEBUG_TRACE_EXECUTION
			printf("OP_SUBTRACT\n");
#endif
			Value a = pop();
			Value b = pop();
			if (a.type != VAL_NUMBER || b.type != VAL_NUMBER)
			{
				runtimeError("Operands must be numbers.", curr_line);
				return INTERPRET_RUNTIME_ERROR;
			}
			b.num -= a.num;
			push(b);
			break;
		}
		case OP_MULTIPLY:
		{
#ifdef DEBUG_TRACE_EXECUTION
			printf("OP_MULTIPLY\n");
#endif
			Value a = pop();
			Value b = pop();
			if (a.type != VAL_NUMBER || b.type != VAL_NUMBER)
			{
				runtimeError("Operands must be numbers.", curr_line);
				return INTERPRET_RUNTIME_ERROR;
			}
			b.num *= a.num;
			push(b);
			break;
		}
		case OP_DIVIDE:
		{
#ifdef DEBUG_TRACE_EXECUTION
			printf("OP_DIVIDE\n");
#endif
			Value a = pop();
			Value b = pop();
			if (a.type != VAL_NUMBER || b.type != VAL_NUMBER)
			{
				runtimeError("Operands must be numbers.", curr_line);
				return INTERPRET_RUNTIME_ERROR;
			}
			b.num /= a.num;
			push(b);
			break;
		}
		case OP_NEGATE:
#ifdef DEBUG_TRACE_EXECUTION
			printf("OP_NEGATE\n");
#endif
			if (vm.stackTop[-1].type != VAL_NUMBER)
			{
				runtimeError("Operand must be a number.", curr_line);
				return INTERPRET_RUNTIME_ERROR;
			}
			vm.stackTop[-1].num *= -1;
			break;
		case OP_NOT:
#ifdef DEBUG_TRACE_EXECUTION
			printf("OP_NOT\n");
#endif
			vm.stackTop[-1].type = VAL_BOOL;
			vm.stackTop[-1].boolean = isFalsey(vm.stackTop[-1]);
			break;
		case OP_PRINT:
#ifdef DEBUG_TRACE_EXECUTION
			printf("OP_PRINT\n");
#endif
			printValue(pop());
			printf("\n");
			break;
		case OP_POP:
#ifdef DEBUG_TRACE_EXECUTION
			printf("OP_POP\n");
#endif
			pop();
			break;
		case OP_RETURN:
#ifdef DEBUG_TRACE_EXECUTION
			printf("OP_RETURN\n");
#endif
			return INTERPRET_OK;
		}
	}
}

InterpretResult interpret(const char *source, int repl)
{
	Chunk chunk;
	initChunk(&chunk);
	if (!compile(source, &chunk))
	{
		freeChunk(&chunk);
		if (repl) resetParser();
		return INTERPRET_COMPILE_ERROR;
	}
#ifdef DEBUG_TRACE_EXECUTION
	disassembleChunk(&chunk, "code");
#endif
	vm.chunk = &chunk;
	vm.ip = chunk.code;
	InterpretResult result = run();
	freeChunk(&chunk);
	return result;
}
