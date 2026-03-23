#include "memory.h"
#include "line_encoding.h"

void initLineEncoding(LineEncoding *encoding)
{
	encoding->capacity = 0;
	encoding->count = 0;
	encoding->data = NULL;
}

void freeLineEncoding(LineEncoding *encoding)
{
	FREE_ARRAY(encoding->data);
	initLineEncoding(encoding);
}

static void addLineData(LineEncoding *encoding, int num, int line)
{
	if (encoding->capacity == encoding->count)
	{
		encoding->capacity = GROW_CAPACITY(encoding->capacity);
		encoding->data = GROW_ARRAY(LineData, encoding->data, encoding->capacity);
	}
	encoding->data[encoding->count++] = (LineData){ num, line };
}

void writeLineEncoding(LineEncoding *encoding, int line)
{
	int count = encoding->count;
	LineData *data = encoding->data;
	if (!data || data[count - 1].line != line)
	{
		addLineData(encoding, 1, line);
	} else {
		++data[count - 1].num;
	}
}
