#ifndef COMPILER_H
#define COMPILER_H

#include "chunk.h"

int compile(const char *source, Chunk *chunk, Obj **head);

void resetParser();

#endif
