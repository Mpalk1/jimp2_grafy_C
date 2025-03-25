COMPILER = clang
CFLAGS = -Wall -Wextra -Werror -g

NAME = gsplit

SRCS = srcs/main.c srcs/options.c srcs/print.c srcs/parsing.c srcs/memory.c srcs/debug.c srcs/split.c

OBJECTS = ${SRCS:.c=.o}

INCLUDE_FOLDER = headers
INCLUDES = ${INCLUDE_FOLDER}/gsplit.h

all: ${NAME}

${NAME}: ${OBJECTS}
	${COMPILER} ${CFLAGS} -I${INCLUDE_FOLDER} $^ -o $@

%.o: %.c ${INCLUDES}
	${COMPILER} ${CFLAGS} -I${INCLUDE_FOLDER} -c $< -o $@

clean:
	rm -f ${OBJECTS}

fclean: clean
	rm -f ${NAME}

re: fclean all

.PHONY: all clean fclean re
