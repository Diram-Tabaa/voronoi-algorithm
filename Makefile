CC = gcc
CFLAGS = -g -Wall -O3 -std=c99 
LDFLAGS = -lm

SOURCES = uarray.c bst.c geometry.c priority_queue.c voronoi.c voronoi_main.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = voronoi

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	@rm -f $(TARGET) $(OBJECTS) core