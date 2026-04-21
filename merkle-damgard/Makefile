CC = gcc
CFLAGS = -O3 -march=native -std=c11 -Isrc
LDFLAGS = -lm

SRCDIR = src
TARGET = second_preim
OBJS = second_preim_48.o tests.o helpers.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

second_preim_48.o: $(SRCDIR)/second_preim_48.c $(SRCDIR)/second_preim.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/second_preim_48.c

tests.o: $(SRCDIR)/tests.c $(SRCDIR)/second_preim.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/tests.c

helpers.o: $(SRCDIR)/helpers.c $(SRCDIR)/xoshiro.h $(SRCDIR)/second_preim.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/helpers.c

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run