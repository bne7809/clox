#ifndef COMPILER_H
#define COMPILER_H

#include "object.h"

ObjFunction *compile(const char *source);

void resetParser();

#endif
