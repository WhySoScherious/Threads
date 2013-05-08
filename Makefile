#Makefile for Assignment 1

SRC    = threads.c
OBJ    = $(SRC:.c=.o)
MKFILE = Makefile
README = README
DESGN  = DESIGN
JUNK   = $(OBJ)
ALL    = $(SRC) $(DESGN) $(MKFILE)
EXE    = threads

GCC    = gcc
FLAGS  = -Wall -Wextra -std=gnu99

.SECONDARY:


all: $(EXE)

$(EXE): $(OBJ)
	$(GCC) -o $@ $^ 

%.o:%.c
	$(GCC) $(FLAGS) -c $^

tar:
	tar -cf $(EXE).tar $(ALL) 

again: spotless all

clean:
	rm $(JUNK)

spotless: clean
	rm $(EXE)
