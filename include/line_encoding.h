#ifndef LINE_ENCODING_H
#define LINE_ENCODING_H

typedef struct
{
	int num;
	int line;
} LineData;

typedef struct
{
	int capacity;
	int count;
	LineData *data;
} LineEncoding;

void initLineEncoding(LineEncoding *encoding);
void freeLineEncoding(LineEncoding *encoding);
void writeLineEncoding(LineEncoding *encoding, int line);


#endif
