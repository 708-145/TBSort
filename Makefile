CC = gcc
CFLAGS = -Wall -g -std=c99 -O2
LDFLAGS = -lm # For linking math library used in tbsort_int64.c

TARGET = tbsort_bench
OBJS = tbsort_bench.o tbsort_int64.o

# Default rule
all: $(TARGET)

# Rule to link the target executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Rule to compile tbsort_bench.c into tbsort_bench.o
tbsort_bench.o: tbsort_bench.c tbsort_int64.h
	$(CC) $(CFLAGS) -c tbsort_bench.c -o tbsort_bench.o

# Rule to compile tbsort_int64.c into tbsort_int64.o
tbsort_int64.o: tbsort_int64.c tbsort_int64.h
	$(CC) $(CFLAGS) -c tbsort_int64.c -o tbsort_int64.o

# Clean rule
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean
