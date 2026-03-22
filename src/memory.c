#include <stdlib.h>

void *reallocate(void *pointer, size_t newSize)
{
	if (newSize == 0)
	{
		free(pointer);
		return NULL;
	}
	void *ret = realloc(pointer, newSize);
	if (!ret)
		exit(1);
	return ret;
}
