#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chunk.h"
#include "scanner.h"
#include "int_array.h"
#include "compiler.h"

#ifdef DEBUG_PRINT_TOKENS
#include "debug.h"
#endif

#define UINT8_COUNT	(UINT8_MAX + 1)

#define SET_ERROR	parser.flags |= 1
#define IS_ERROR	(parser.flags & 1)
#define SET_PANIC	parser.flags |= 2
#define UNSET_PANIC	parser.flags &= ~2
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
	PREC_TERNARY,

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

typedef void (*ParseFn)(int);

typedef struct
{
	ParseFn prefix;
	ParseFn infix;
	Precedence precedence;
} ParseRule;

typedef struct
{
	Token name;
	int depth;
} Local;

typedef struct
{
	int loop;
	int depth;
} Loop;

typedef enum
{
	TYPE_FUNCTION,
	TYPE_SCRIPT
} FunctionType;

typedef struct Compiler
{
	struct Compiler *enclosing;

	ObjFunction *function;
	FunctionType type;

	Local locals[UINT8_COUNT];
	int localCount;
	Loop loops[32];
	int loopCount;
	int scopeDepth;
} Compiler;

static void grouping(int);
static void unary(int);
static void binary(int);
static void and_(int);
static void or_(int);
static void trnary(int);
static void number(int);
static void string(int);
static void variable(int);
static void literal(int);

ParseRule rules[] = {
	[TOKEN_LEFT_PAREN]    = {grouping, NULL,   PREC_NONE},
	[TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
	[TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}, 
	[TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
	[TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_DOT]           = {NULL,     NULL,   PREC_NONE},
	[TOKEN_QUESTION]      = {NULL,     trnary, PREC_TERNARY},
	[TOKEN_COLON]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
	[TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},

	[TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
	[TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
	[TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
	[TOKEN_BANG]          = {unary,    NULL,   PREC_NONE},
	[TOKEN_BANG_EQUAL]    = {NULL,     binary, PREC_EQUALITY},
	[TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_EQUAL_EQUAL]   = {NULL,     binary, PREC_EQUALITY},
	[TOKEN_GREATER]       = {NULL,     binary, PREC_COMPARISON},
	[TOKEN_GREATER_EQUAL] = {NULL,     binary, PREC_COMPARISON},
	[TOKEN_LESS]          = {NULL,     binary, PREC_COMPARISON},
	[TOKEN_LESS_EQUAL]    = {NULL,     binary, PREC_COMPARISON},

	[TOKEN_IDENTIFIER]    = {variable, NULL,   PREC_NONE},
	[TOKEN_STRING]        = {string,   NULL,   PREC_NONE},
	[TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
	[TOKEN_AND]           = {NULL,     and_,   PREC_AND},
	[TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
	[TOKEN_FALSE]         = {literal,  NULL,   PREC_NONE},
	[TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
	[TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
	[TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
	[TOKEN_SWITCH]        = {NULL,     NULL,   PREC_NONE},
	[TOKEN_CASE]          = {NULL,     NULL,   PREC_NONE},
	[TOKEN_DEFAULT]       = {NULL,     NULL,   PREC_NONE},
	[TOKEN_NIL]           = {literal,  NULL,   PREC_NONE},

	[TOKEN_OR]            = {NULL,     or_,    PREC_OR},
	[TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
	[TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
	[TOKEN_TRUE]          = {literal,  NULL,   PREC_NONE},
	[TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
	[TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};

static Parser parser;
Compiler *current;

static void initCompiler(Compiler *compiler, FunctionType type)
{
	compiler->enclosing = current;
	compiler->function = newFunction();
	compiler->type = type;
	compiler->localCount = 0;
	compiler->loopCount = 0;
	compiler->scopeDepth = 0;
	current = compiler;
	current->function->name = NULL;
	if (type != TYPE_SCRIPT)
	{
		current->function->name = copyString(parser.prev.start, parser.prev.length);
	}

	Local *local = &current->locals[current->localCount++];
	local->depth = 0;
	local->name.start = "";
	local->name.length = 0;
};

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
	return &current->function->chunk;
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

static void synchronize()
{
	UNSET_PANIC;
	while (parser.curr.type != TOKEN_EOF)
	{
		if (parser.prev.type == TOKEN_SEMICOLON) return;
		switch (parser.curr.type)
		{
		case TOKEN_CLASS:
		case TOKEN_FUN:
		case TOKEN_VAR:
		case TOKEN_FOR:
		case TOKEN_IF:
		case TOKEN_WHILE:
		case TOKEN_PRINT:
		case TOKEN_RETURN:
			return;
		default:
			break;
		}
		advance();
	}
}

static int check(TokenType type)
{
	return parser.curr.type == type;
}

static int match(TokenType type)
{
	if (check(type))
	{
		advance();
		return 1;
	}
	return 0;
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
	int canAssign = precedence <= PREC_ASSIGNMENT;
	prefixRule(canAssign);
	while (precedence <= getRule(parser.curr.type)->precedence)
	{
		advance();
		ParseFn infixRule = getRule(parser.prev.type)->infix;
		infixRule(canAssign);
	}
	if (canAssign && match(TOKEN_EQUAL)) error("Invalid assignment target.");
}

static int identifierConstant(Token *token)
{
	return makeConstant(currentChunk(), (Value){ VAL_OBJ, {.obj = (Obj *)copyString(token->start, token->length)} });
}

static void expression()
{
	parsePrecedence(PREC_ASSIGNMENT);
}


static int identifiersEqual(Token *t1, Token *t2)
{
	if (t1->length != t2->length) return 0;
	return !memcmp(t1->start, t2->start, t1->length);
}

static int resolveLocal(Compiler *compiler, Token *name)
{
	for (int i = compiler->localCount - 1; i >= 0; --i)
	{
		Local *l = &compiler->locals[i];
		if (identifiersEqual(name, &l->name))
		{
			if (l->depth == -1)
			{
				error("Can't read local variable in its own initializer.");
			}
			return i;
		}
	}
	return -1;
}

static void variable(int canAssign)
{
	OpCode getOp;
	OpCode setOp;
	int arg = resolveLocal(current, &parser.prev);
	if (arg != -1)
	{
		getOp = OP_GET_LOCAL;
		setOp = OP_SET_LOCAL;
	} else {
		arg = identifierConstant(&parser.prev);
		getOp = OP_GET_GLOBAL;
		setOp = OP_SET_GLOBAL;
	}
	if (canAssign && match(TOKEN_EQUAL))
	{
		expression();
		writeVariable(currentChunk(), setOp, arg, parser.prev.line);
	} else {
		writeVariable(currentChunk(), getOp, arg, parser.prev.line);
	}
}

static void literal(int canAssign)
{
	(void)canAssign;
	switch (parser.prev.type)
	{
	case TOKEN_FALSE: emitByte(OP_FALSE); break;
	case TOKEN_TRUE: emitByte(OP_TRUE); break;
	case TOKEN_NIL: emitByte(OP_NIL); break;
	default: return;
	}
}

static void string(int canAssign)
{
	(void)canAssign;
	return emitConstant((Value) { VAL_OBJ, { .obj = (Obj *)copyString(parser.prev.start + 1, parser.prev.length - 2) } });
}

static void number(int canAssign)
{
	(void)canAssign;
	Value v;
	v.type = VAL_NUMBER;
	v.num = strtod(parser.prev.start, NULL);
	emitConstant(v);
}

static int emitJump(uint8_t inst)
{
	emitByte(inst);
	emitByte(0xff);
	emitByte(0xff);
	return currentChunk()->count - 2;
}

static void patchJump(int offset)
{
	size_t count = currentChunk()->count;
	uint8_t *code = currentChunk()->code;

	int jump = count - offset - 2;
	if (jump > UINT16_MAX) error("Too much code to jump over.");
	code[offset] = (jump >> 8) & 0xff;
	code[offset + 1] = jump & 0xff;
}

static void trnary(int canAssign)
{
	(void)canAssign;
	int j1 = emitJump(OP_JUMP_IF_FALSE);
	emitByte(OP_POP);
	parsePrecedence(PREC_TERNARY);
	int j2 = emitJump(OP_JUMP);
	patchJump(j1);
	emitByte(OP_POP);
	consume(TOKEN_COLON, "Expect ':' in ternary.");
	parsePrecedence(PREC_TERNARY);
	patchJump(j2);
}

static void binary(int canAssign)
{
	(void)canAssign;
	TokenType op = parser.prev.type;
	ParseRule *rule = getRule(op);
	parsePrecedence(rule->precedence + 1);
	switch (op)
	{
	case TOKEN_PLUS: emitByte(OP_ADD); break;
	case TOKEN_MINUS: emitByte(OP_SUBTRACT); break;
	case TOKEN_STAR: emitByte(OP_MULTIPLY); break;
	case TOKEN_SLASH: emitByte(OP_DIVIDE); break;
	case TOKEN_BANG_EQUAL: emitByte(OP_EQUAL); emitByte(OP_NOT); break;
	case TOKEN_EQUAL_EQUAL: emitByte(OP_EQUAL); break;
	case TOKEN_GREATER: emitByte(OP_GREATER); break;
	case TOKEN_GREATER_EQUAL: emitByte(OP_LESS); emitByte(OP_NOT); break;
	case TOKEN_LESS: emitByte(OP_LESS); break;
	case TOKEN_LESS_EQUAL: emitByte(OP_GREATER); emitByte(OP_NOT); break;
	default: return;
	}
}

static void unary(int canAssign)
{
	(void)canAssign;
	TokenType op = parser.prev.type;
	parsePrecedence(PREC_UNARY);
	switch (op)
	{
	case TOKEN_MINUS: emitByte(OP_NEGATE); break;
	case TOKEN_BANG: emitByte(OP_NOT); break;
	default: return;
	}
}

static void grouping(int canAssign)
{
	(void)canAssign;
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void declaration();
static void statement();

static void printStatement()
{
	expression();
	consume(TOKEN_SEMICOLON, "Expect ';' after value.");
	emitByte(OP_PRINT);
}

static void expressionStatement()
{
	expression();
	consume(TOKEN_SEMICOLON, "Expect ';' after value.");
	emitByte(OP_POP);
}

static void block()
{
	while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF))
	{
		declaration();
	}
	consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

static void beginScope()
{
	++current->scopeDepth;
}

static void endScope()
{
	--current->scopeDepth;
	while (current->localCount > 0 && current->locals[current->localCount - 1].depth > current->scopeDepth)
	{
		emitByte(OP_POP);
		--current->localCount;
	}
}

static void and_(int canAssign)
{
	(void)canAssign;
	int jump = emitJump(OP_JUMP_IF_FALSE);
	emitByte(OP_POP);
	parsePrecedence(PREC_AND);
	patchJump(jump);
}

static void or_(int canAssign)
{
	(void)canAssign;
	int jump = emitJump(OP_JUMP_IF_FALSE);
	int end = emitJump(OP_JUMP);
	patchJump(jump);
	emitByte(OP_POP);
	parsePrecedence(PREC_OR);
	patchJump(end);
}

static void ifStatement()
{
	int elseJump;

	consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");
	int thenJump = emitJump(OP_JUMP_IF_FALSE);
	emitByte(OP_POP);
	statement();
	elseJump = emitJump(OP_JUMP);
	patchJump(thenJump);
	emitByte(OP_POP);
	if (match(TOKEN_ELSE)) statement();
	patchJump(elseJump);
}

static void emitLoop(int start)
{
	emitByte(OP_LOOP);
	int offset = currentChunk()->count - start + 2;
	if (offset > UINT16_MAX) error("Loop body too large.");
	emitByte((offset >> 8) & 0xff);
	emitByte(offset & 0xff);
}

static void whileStatement()
{
	int loop = currentChunk()->count;
	Loop *l = &current->loops[current->loopCount++];
	l->loop = loop;
	l->depth = current->scopeDepth;
	consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");
	int end = emitJump(OP_JUMP_IF_FALSE);
	emitByte(OP_POP);
	statement();
	emitLoop(loop);
	patchJump(end);
	emitByte(OP_POP);
	--current->loopCount;
}

static void addLocal(Token *name)
{
	if (current->localCount == UINT8_COUNT)
	{
		error("Too many local variables in function.");
		return;
	}
	for (int i = current->localCount - 1; i >= 0; --i)
	{
		Local *l = &current->locals[i];
		if (l->depth != -1 && l->depth < current->scopeDepth)
			break;
		if (identifiersEqual(name, &l->name))
		{
			error("Already a variable with this name in this scope.");
		}
	}
	Local *local = &current->locals[current->localCount++];
	local->name = *name;
	local->depth = -1;
}

static int parseVariable(const char *str)
{
	consume(TOKEN_IDENTIFIER, str);
	if (current->scopeDepth > 0)
	{
		addLocal(&parser.prev);
		return 0;
	}
	return identifierConstant(&parser.prev);
}

static void varDeclaration()
{
	int global = parseVariable("Expect variable name.");
	int line = parser.prev.line;
	if (match(TOKEN_EQUAL)) expression();
	else emitByte(OP_NIL);
	consume(TOKEN_SEMICOLON, "Expect ';' after value.");
	if (current->scopeDepth > 0)
	{
		current->locals[current->localCount - 1].depth = current->scopeDepth;
		return;
	}
	writeVariable(currentChunk(), OP_DEFINE_GLOBAL, global, line);
}

static void forStatement()
{
	beginScope();
	consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");
	if (match(TOKEN_SEMICOLON))
	{
	} else if (match(TOKEN_VAR)) {
		varDeclaration();
	} else {
		expressionStatement();
	}
	Loop *l = &current->loops[current->loopCount++];
	l->loop = currentChunk()->count;
	l->depth = current->scopeDepth;
	int exit = -1;
	if (!match(TOKEN_SEMICOLON))
	{
		expression();
		consume(TOKEN_SEMICOLON, "Expect ';'.");
		exit = emitJump(OP_JUMP_IF_FALSE);
		emitByte(OP_POP);
	}
	if (!match(TOKEN_RIGHT_PAREN))
	{
		int bodyJump = emitJump(OP_JUMP);
		int incStart = currentChunk()->count;
		expression();
		emitByte(OP_POP);
		consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");
		emitLoop(l->loop);
		l->loop = incStart;
		patchJump(bodyJump);
	}
	statement();
	emitLoop(l->loop);
	if (exit != -1)
	{
		patchJump(exit);
		emitByte(OP_POP);
	}
	--current->loopCount;
	endScope();
}

static void switchStatement()
{
	IntArray jumps;
	initIntArray(&jumps);
	int numdefs = 0;
	consume(TOKEN_LEFT_PAREN, "Expect '(' after 'switch'.");
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after switch expression.");
	consume(TOKEN_LEFT_BRACE, "Expect '{'.");
	while (1)
	{
		if (match(TOKEN_CASE))
		{
			if (numdefs > 0) error("Default case should be last.");
			emitByte(OP_PUSH);
			expression();
			emitByte(OP_EQUAL);
			int jCase = emitJump(OP_JUMP_IF_FALSE);
			emitByte(OP_POP);
			consume(TOKEN_COLON, "Expect ':' after 'case'.");
			while (!check(TOKEN_EOF) && !check(TOKEN_RIGHT_BRACE) && !check(TOKEN_CASE) && !check(TOKEN_DEFAULT)) statement();
			addIntArray(&jumps, emitJump(OP_JUMP));
			patchJump(jCase);
			emitByte(OP_POP);
		} else if (match(TOKEN_DEFAULT)) {
			if (numdefs > 0) error("More than one default case.");
			++numdefs;
			consume(TOKEN_COLON, "Expect ':' after 'default'.");
			while (!check(TOKEN_EOF) && !check(TOKEN_RIGHT_BRACE)) statement();
			addIntArray(&jumps, emitJump(OP_JUMP));
		} else {
			break;
		}
	}
	consume(TOKEN_RIGHT_BRACE, "Expect '}'.");
	for (int i = 0; i < jumps.count; ++i)
	{
		patchJump(jumps.values[i]);
	}
	freeIntArray(&jumps);
	emitByte(OP_POP);
}

static void continueStatement()
{
	if (current->loopCount == 0)
	{
		error("Continue not in loop.");
		return;
	}
	Loop *l = &current->loops[current->loopCount - 1];
	int scopeDepth = current->scopeDepth;
	int localCount = current->localCount;
	while (scopeDepth > l->depth)
	{
		--scopeDepth;
		while (localCount > 0 && current->locals[localCount - 1].depth > scopeDepth)
		{
			emitByte(OP_POP);
			--localCount;
		}
	}
	consume(TOKEN_SEMICOLON, "Expect ';' after continue.");
	emitLoop(l->loop);
}

static void statement()
{
	if (match(TOKEN_PRINT))
	{
		printStatement();
	} else if (match(TOKEN_IF)) {
		ifStatement();
	} else if (match(TOKEN_SWITCH)) {
		switchStatement();
	} else if (match(TOKEN_WHILE)) {
		whileStatement();
	} else if (match(TOKEN_FOR)) {
		forStatement();
	} else if (match(TOKEN_CONTINUE)) {
		continueStatement();
	}else if (match(TOKEN_LEFT_BRACE)) {
		beginScope();
		block();
		endScope();
	} else {
		expressionStatement();
	}
}

static ObjFunction *endCompiler()
{
	emitByte(OP_RETURN);
	ObjFunction *f = current->function;
	current = current->enclosing;
	return f;
}

static void function(FunctionType type)
{
	Compiler compiler;
	initCompiler(&compiler, type);
	beginScope();
	consume(TOKEN_LEFT_PAREN, "Expect '(' after function name.");
	if (!check(TOKEN_RIGHT_PAREN))
	{
		do {
			if (++current->function->arity > 255)
			{
				errorAtCurrent("Can't have more than 255 parameters.");
				uint8_t constant = parseVariable("Expect parameter name.");
				if (current->scopeDepth == 0)
				{
					writeVariable(currentChunk(), OP_DEFINE_GLOBAL, constant, parser.prev.line);
				}
			}
		} while (match(TOKEN_COMMA));
	}
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
	consume(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
	block();
	ObjFunction *f = endCompiler();
	emitConstant((Value){.type = VAL_OBJ, .obj = (Obj *)f});
}

static void funDeclaration()
{
	uint8_t global = parseVariable("Expect function name.");
	if (current->scopeDepth != 0)
	{
		current->locals[current->localCount - 1].depth = current->scopeDepth;
	}
	function(TYPE_FUNCTION);
	writeVariable(currentChunk(), OP_DEFINE_GLOBAL, global, parser.prev.line);
}

static void declaration()
{
	if (match(TOKEN_FUN)) funDeclaration();
	else if (match(TOKEN_VAR)) varDeclaration();
	else statement();
	if (IS_PANIC) synchronize();
}

ObjFunction *compile(const char *source)
{
#ifdef DEBUG_PRINT_TOKENS
	printTokens(source);
#endif
	initScanner(source);
	Compiler compiler;
	initCompiler(&compiler, TYPE_SCRIPT);
	advance();
	while (!match(TOKEN_EOF)) declaration();
	ObjFunction *f = endCompiler();
	return IS_ERROR ? NULL : f;
}
