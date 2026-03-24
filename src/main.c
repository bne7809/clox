#include <stdio.h>
#include <stdlib.h>
#include "vm.h"

static void repl()
{
	char line[1024];
	while (1)
	{
		printf("> ");
		if (!fgets(line, sizeof(line), stdin))
		{
			printf("\n");
			break;
		}
		interpret(line);
	}
}

static char *readFile(const char *path)
{
	FILE *f = fopen(path, "rb");
	if (!f)
	{
		fprintf(stderr, "Could not open file \"%s\".\n", path);
		exit(74);
	}
	fseek(f, 0, SEEK_END);
	size_t fileSize = ftell(f);
	rewind(f);
	char *buff = malloc(fileSize + 1);
	if (!buff)
	{
		fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
		exit(74);
	}
	size_t bytesRead = fread(buff, sizeof(char), fileSize, f);
	if (bytesRead < fileSize)
	{
		fprintf(stderr, "Could not read file \"%s\".\n", path);
		exit(74);
	}
	buff[bytesRead] = '\0';
	fclose(f);
	return buff;
}

static void runFile(const char *path)
{
	char *source = readFile(path);
	InterpretResult result = interpret(source);
	free(source);

	if (result == INTERPRET_COMPILE_ERROR) exit(65);
	if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, char **argv)
{
	initVM();

	if (argc == 1)
	{
		repl();
	} else if (argc == 2) {
		runFile(argv[1]);
	} else {
		fprintf(stderr, "Usage: clox [path]\n");
		return 64;
	}

	freeVM();
}
