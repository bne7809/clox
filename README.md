# clox

A bytecode interpreter for the Lox programming language, written from scratch in C.

This project follows Robert Nystrom's book [*Crafting Interpreters*](https://craftinginterpreters.com/) (the `clox` bytecode-VM half), with several extensions of my own beyond the base material. I built it to understand, end to end, how a real language runtime works: how source text becomes bytecode, and how a virtual machine executes it.

## What it does

The interpreter takes Lox source code and runs it through a full pipeline:

1. **Scanner** — tokenizes the source text.
2. **Compiler** — a single-pass compiler that parses tokens and emits bytecode directly (Pratt / recursive-descent expression parsing with operator precedence).
3. **Virtual machine** — a stack-based VM that executes the bytecode.

Language features supported include dynamic typing (numbers, booleans, nil, strings), arithmetic and logical operators, string concatenation, global and local variables, and control flow (`if`/`else`, `while`, and `for`, compiled to jump and loop instructions).

Under the hood it uses a hash table for global variables and for string interning, and a tagged-union `Value` representation.

## Extensions beyond the book

- **Long constant operands** — `OP_CONSTANT_LONG` and the `*_LONG` global-variable opcodes use a 24-bit operand, so a chunk can hold more than 256 constants.
- **Run-length-encoded line information** — line numbers are stored compactly as run-length data rather than one entry per instruction, including the logic to walk that encoding backwards when a loop jumps back, so the debug tracer still reports the correct source line.
- **Additional language features — added switch statements, continue in loops, and a ternary (?:) operator, each compiled down to the existing jump/loop bytecode.

## Build

Requires a C compiler (`cc`/`gcc`/`clang`) and `make`.

```sh
make
```

This produces the `clox` executable.

## Usage

```sh
./clox            # start the REPL
./clox script.lox # run a Lox source file
```

## Project structure

```
include/   # headers
src/       # scanner, compiler, VM, chunk, value, object, table, memory, line encoding
test/      # test inputs
Makefile
```

## Notes

Debug tracing (bytecode disassembly and a per-instruction stack dump) can be toggled with the `DEBUG_TRACE_EXECUTION` define in `src/vm.c`.
