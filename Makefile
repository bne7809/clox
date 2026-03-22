NAME = clox

SRCS = ./src/main.c ./src/chunk.c ./src/memory.c ./src/debug.c ./src/value.c

OBJS = $(SRCS:./src/%.c=./bin/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	cc -o $(NAME) $(OBJS)

./bin/%.o: ./src/%.c | bin
	cc -c -Wall -Werror -Wextra -I ./include $< -o $@

bin:
	mkdir -p bin

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
