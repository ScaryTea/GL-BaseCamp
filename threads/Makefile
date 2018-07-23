TARGET=thread_matrix

CC=gcc
CFLAGS=-O2 -Wall -Wextra -Wpedantic
LIBFLAGS=-lpthread

.PHONY: all clean

all: clean | $(TARGET)

$(TARGET):
	$(CC) $(CFLAGS) $(addsuffix .c, $(TARGET)) -o $@  $(LIBFLAGS)

clean:
	-rm -f $(TARGET)
