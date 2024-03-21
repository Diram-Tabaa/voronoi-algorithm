CC = gcc
CFLAGS = -g -Wall -O3 -std=c99 -I/usr/include/python3.10 
LDFLAGS = -lm 

SOURCES = uarray.c bst.c geometry.c priority_queue.c voronoi.c voronoi_main.c 
PY_SOURCES = uarray.c bst.c geometry.c priority_queue.c voronoi.c voronoipy.c
OBJECTS = $(SOURCES:.c=.o)
PY_OBJECTS = $(PY_SOURCES:.c=.o)
TARGET = voronoi

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	@rm -f $(TARGET) $(OBJECTS) core

python: 
	python3 setup.py build_ext --inplace