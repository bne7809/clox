#ifndef DEBUG_H
#define DEBUG_H

#include "chunk.h"

void disassembleChunk(Chunk *chunk, const char *name);
void printTokens(const char *source);

int getLine(Chunk *chunk, int index);

#endif
