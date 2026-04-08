#ifndef SCANNER_H
#define SCANNER_H

#define TOKEN_LIST \
	X(TOKEN_LEFT_PAREN) \
	X(TOKEN_RIGHT_PAREN) \
	X(TOKEN_LEFT_BRACE) \
	X(TOKEN_RIGHT_BRACE) \
	X(TOKEN_COMMA) \
	X(TOKEN_DOT) \
	X(TOKEN_QUESTION) \
	X(TOKEN_COLON) \
	X(TOKEN_MINUS) \
	X(TOKEN_PLUS) \
	X(TOKEN_SEMICOLON) \
	X(TOKEN_SLASH) \
	X(TOKEN_STAR) \
	X(TOKEN_BANG) \
	X(TOKEN_BANG_EQUAL) \
	X(TOKEN_EQUAL) \
	X(TOKEN_EQUAL_EQUAL) \
	X(TOKEN_GREATER) \
	X(TOKEN_GREATER_EQUAL) \
	X(TOKEN_LESS) \
	X(TOKEN_LESS_EQUAL) \
	X(TOKEN_IDENTIFIER) \
	X(TOKEN_STRING) \
	X(TOKEN_NUMBER) \
	X(TOKEN_AND) \
	X(TOKEN_CLASS) \
	X(TOKEN_ELSE) \
	X(TOKEN_FALSE) \
	X(TOKEN_FOR) \
	X(TOKEN_FUN) \
	X(TOKEN_IF) \
	X(TOKEN_SWITCH) \
	X(TOKEN_CASE) \
	X(TOKEN_DEFAULT) \
	X(TOKEN_NIL) \
	X(TOKEN_OR) \
	X(TOKEN_PRINT) \
	X(TOKEN_RETURN) \
	X(TOKEN_SUPER) \
	X(TOKEN_THIS) \
	X(TOKEN_TRUE) \
	X(TOKEN_VAR) \
	X(TOKEN_WHILE) \
	X(TOKEN_ERROR) \
	X(TOKEN_EOF)

typedef enum
{
#define X(t)	t,
	TOKEN_LIST
#undef X
} TokenType;

typedef struct
{
	TokenType type;
	const char *start;
	int length;
	int line;
} Token;

void initScanner(const char *source);

Token scanToken();

const char *tokenToString(TokenType type);

#endif
