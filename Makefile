COMPILER = cc
CFLAGS = -Wall -Wextra -Werror -g

NAME = 

SRCS = 

OBJECTS = ${SRCS:.c=.o}

INCLUDE_FOLDER = 
INCLUDES = 

all: ${NAME}

${NAME}: ${OBJECTS}
	${COMPILER} ${CFLAGS} $^ -o $@ -I${INCLUDE_FOLDER}

%.o: %.c ${INCLUDES}
	${COMPILER} ${CFLAGS} -c $< -o $@ -I${INCLUDE_FOLDER}

clean:
	rm -f ${OBJECTS}

fclean: clean
	rm -f ${NAME}

re: fclean all

.PHONY: all clean fclean re
