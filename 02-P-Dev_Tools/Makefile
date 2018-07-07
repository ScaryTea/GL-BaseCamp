TARGET=mazeSearch
DEPS=lee queue
DEPS:=$(addsuffix .o, $(DEPS))

CC=gcc
CFLAGS?=-O0

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(DEPS)
	$(CC) $(CFLAGS) $(addsuffix .c, $(TARGET)) $(DEPS) -o $(addsuffix $(CFLAGS), $@)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o $(TARGET)
