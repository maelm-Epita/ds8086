CC = gcc
DCFLAGS = -Wall -Wextra -Werror -O3 -Wvla -fsanitize=address -g -D__GNU_SOURCE
CFLAGS = -Wall -Wextra -O3 -Wvla -fsanitize=address -g -D__GNU_SOURCE
SRC = \
			src/main.c \
			src/instructions.c \
			src/hex.c

debug:
	${CC} ${SRC} ${CFLAGS} -o ds $(LFLAGS)
all: 
	${CC} ${SRC} ${CFLAGS} -o ds $(LFLAGS)
.PHONY: clean
clean:
	rm ds
