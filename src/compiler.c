#include "scanner.h"

#define DEBUG_PRINT_TOKENS

#ifdef DEBUG_PRINT_TOKENS
#include "debug.h"
#endif

void compile(const char *source)
{
#ifdef DEBUG_PRINT_TOKENS
	printTokens(source);
#endif
	initScanner(source);
}
