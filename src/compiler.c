#include <stdio.h>
#include <stdlib.h>
#include "chunk.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_TOKENS
#include "debug.h"
#endif

#define SET_ERROR	parser.flags |= 1
#define IS_ERROR	(parser.flags & 1)
#define SET_PANIC	parser.flags |= 2
#define IS_PANIC	(parser.flags & 2)

typedef struct
{
	Token prev;
	Token curr;
	int flags;
} Parser;

typedef enum {
	PREC_NONE,
	PREC_ASSIGNMENT,
	PREC_OR,

	PREC_AND,
	PREC_EQUALITY,
	PREC_COMPARISON,

	PREC_TERM,
	PREC_FACTOR,

	PREC_UNARY,
	PREC_CALL,
	PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)();

typedef struct
{
	ParseFn prefix;
	ParseFn infix;
	Precedence precedence;
} ParseRule;

static void grouping();
static void unary();
static void binary();
static void number();

ParseRule rules[] = {
	[TOKEN_LEFT_PAREN]    = {grouping, NULL,   PREC_NONE},
	[TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
	[TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}, 
	[TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
	[TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_DOT]           = {NULL,     NULL,   PREC_NONE},
	[TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
	[TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},

	[TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
	[TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
	[TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
	[TOKEN_BANG]          = {NULL,     NULL,   PREC_NONE},
	[TOKEN_BANG_EQUAL]    = {NULL,     NULL,   PREC_NONE},
	[TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_EQUAL_EQUAL]   = {NULL,     NULL,   PREC_NONE},
	[TOKEN_GREATER]       = {NULL,     NULL,   PREC_NONE},
	[TOKEN_GREATER_EQUAL] = {NULL,     NULL,   PREC_NONE},

	[TOKEN_LESS]          = {NULL,     NULL,   PREC_NONE},

	[TOKEN_LESS_EQUAL]    = {NULL,     NULL,   PREC_NONE},
	[TOKEN_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
	[TOKEN_STRING]        = {NULL,     NULL,   PREC_NONE},
	[TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
	[TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
	[TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
	[TOKEN_FALSE]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
	[TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
	[TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
	[TOKEN_NIL]           = {NULL,     NULL,   PREC_NONE},

	[TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
	[TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
	[TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
	[TOKEN_TRUE]          = {NULL,     NULL,   PREC_NONE},
	[TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
	[TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};

static Parser parser;
static Chunk *compilingChunk;

void resetParser()
{
	parser.flags = 0;
}

static ParseRule *getRule(TokenType type)
{
	return &rules[type];
}

static Chunk *currentChunk()
{
	return compilingChunk;
}

static void errorAt(Token *token, const char *str)
{
	if (IS_PANIC) return;
	SET_PANIC;
	fprintf(stderr, "[line %d] Error", token->line);
	if (token->type == TOKEN_EOF)
	{
		fprintf(stderr, " at end");
	} else if (token->type == TOKEN_ERROR) {
	} else {
		fprintf(stderr, " at %.*s", token->length, token->start);
	}
	fprintf(stderr, ": %s\n", str);
	SET_ERROR;
}

static void errorAtCurrent(const char *str)
{
	errorAt(&parser.curr, str);
}

static void error(const char *str)
{
	errorAt(&parser.prev, str);
}

static void advance()
{
	parser.prev = parser.curr;
	while (1)
	{
		parser.curr = scanToken();
		if (parser.curr.type != TOKEN_ERROR) break;
		errorAtCurrent(parser.curr.start);
	}
}

static void consume(TokenType type, const char *str)
{
	if (parser.curr.type == type)
	{
		advance();
		return;
	}
	errorAtCurrent(str);
}

static void emitByte(uint8_t byte)
{
	writeChunk(currentChunk(), byte, parser.prev.line);
}

static void emitConstant(Value val)
{
	writeConstant(currentChunk(), val, parser.prev.line);
}

static void parsePrecedence(Precedence precedence)
{
	advance();
	ParseFn prefixRule = getRule(parser.prev.type)->prefix;
	if (!prefixRule)
	{
		error("Expect expression.");
		return;
	}
	prefixRule();
	while (precedence <= getRule(parser.curr.type)->precedence)
	{
		advance();
		ParseFn infixRule = getRule(parser.prev.type)->infix;
		infixRule();
	}
}

static void number()
{
	Value v = strtod(parser.prev.start, NULL);
	emitConstant(v);
}

static void binary()
{
	TokenType op = parser.prev.type;
	ParseRule *rule = getRule(op);
	parsePrecedence(rule->precedence + 1);
	switch (op)
	{
	case TOKEN_PLUS: emitByte(OP_ADD); return;
	case TOKEN_MINUS: emitByte(OP_SUBTRACT); return;
	case TOKEN_STAR: emitByte(OP_MULTIPLY); return;
	case TOKEN_SLASH: emitByte(OP_DIVIDE); return;
	default: return;
	}
}

static void expression()
{
	parsePrecedence(PREC_ASSIGNMENT);
}

static void unary()
{
	TokenType op = parser.prev.type;
	parsePrecedence(PREC_UNARY);
	switch (op)
	{
	case TOKEN_MINUS: emitByte(OP_NEGATE); return;
	default: return;
	}
}

static void grouping()
{
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void endCompiler()
{
	emitByte(OP_RETURN);
}

int compile(const char *source, Chunk *chunk)
{
#ifdef DEBUG_PRINT_TOKENS
	printTokens(source);
#endif
	initScanner(source);
	compilingChunk = chunk;
	advance();
	expression();
	consume(TOKEN_EOF, "Expect end of expression.");
	endCompiler();
	return !IS_ERROR;
}
