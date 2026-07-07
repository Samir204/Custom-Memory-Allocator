

CC = clang
CFLAGS = -Wall -Wextra -g

SRC = main.c allocator.c buddy.c 
OBJ = $(SRC:.c=.o)

TARGET = allocator

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

%.o: %.c allocator.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)












