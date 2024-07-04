CC = gcc
LANG = c
STD = c99
OBJ_DIR = objs/
SRC_DIR = src/
OBJS = $(OBJ_DIR)brute-ttt.o

CFLAGS= -x $(LANG) --std=$(STD) -Wall -Wextra -O3
PROG_NAME = brute-ttt

$(PROG_NAME) : $(OBJS)
	@$(CC) -o $(PROG_NAME) $(OBJS) $(CFLAGS)

$(OBJ_DIR)brute-ttt.o : $(SRC_DIR)brute-ttt.c
	@$(CC) $(CFLAGS) -c $(SRC_DIR)brute-ttt.c -o $(OBJ_DIR)brute-ttt.o

run:
	./$(PROG_NAME)

.PHONY : clean
clean :
	rm $(PROG_NAME) $(OBJS)
