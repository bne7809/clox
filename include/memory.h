#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

#define GROW_CAPACITY(x)	((x) < 8 ? 8 : (2 * (x)))
#define GROW_ARRAY(t, d, n)	(t*)reallocate(d, sizeof(t) * (n))
#define ALLOCATE(t, n)		(t*)reallocate(NULL, sizeof(t) * (n))
#define FREE_ARRAY(d)		reallocate(d, 0)

void *reallocate(void *pointer, size_t newSize);

#endif
