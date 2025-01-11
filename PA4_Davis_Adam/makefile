CC = gcc
CFLAGS = -Wall -Wextra -g -pedantic

TARGET = mymalloc
SRC = mymalloc.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
   $(CC) $(CFLAGS) -o $@ $^

%.o: %.c
   $(CC) $(CFLAGS) -c $< -o $@

clean:
   rm -f $(OBJ) $(TARGET)

.PHONY: all clean