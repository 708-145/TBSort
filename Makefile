# Compilers
CC = gcc
CXX = g++

# Compiler flags
CFLAGS = -Wall -Wextra -g -std=c99 -O2
CXXFLAGS = -Wall -Wextra -g -std=c++11 -O2

# Linker flags
LDFLAGS = -lm # For math library
LDFLAGS_CPP = -lstdc++ -lm # For C++ standard library and math

# Target names
BENCH_TARGET = tbsort_bench_executable
STANDALONE_CPP_TARGET = tbsort_cpp_test

# Object files for the benchmark
BENCH_OBJS = tbsort_bench.o tbsort_int64.o tbsort.o tbsort_cpp_wrapper.o

# Object files for the standalone C++ test
STANDALONE_CPP_OBJS = tbsort.o # Will be compiled with MAIN_TBSORT_CPP

# Default rule
all: $(BENCH_TARGET)

# Rule to link the benchmark executable
$(BENCH_TARGET): $(BENCH_OBJS)
	$(CXX) $(BENCH_OBJS) -o $(BENCH_TARGET) $(LDFLAGS_CPP)

# --- Compilation rules for object files ---

# C object file for benchmark main
tbsort_bench.o: tbsort_bench.c tbsort_int64.h tbsort_cpp_wrapper.h
	$(CC) $(CFLAGS) -c tbsort_bench.c -o tbsort_bench.o

# C object file for C TBSort implementation
tbsort_int64.o: tbsort_int64.c tbsort_int64.h
	$(CC) $(CFLAGS) -c tbsort_int64.c -o tbsort_int64.o

# C++ object file for C++ TBSort implementation (for benchmark, no main)
tbsort.o: tbsort.cpp tbsort.hpp
	$(CXX) $(CXXFLAGS) -c tbsort.cpp -o tbsort.o

# C++ object file for the C wrapper
tbsort_cpp_wrapper.o: tbsort_cpp_wrapper.cpp tbsort_cpp_wrapper.h tbsort.hpp
	$(CXX) $(CXXFLAGS) -c tbsort_cpp_wrapper.cpp -o tbsort_cpp_wrapper.o

# --- Standalone C++ target ---
# To compile tbsort.cpp with its own main for testing
# Need a different object file name or compile and link in one step,
# as tbsort.o is already defined for the benchmark without main.
# Let's make it simpler: compile tbsort.cpp with main directly to the target.
$(STANDALONE_CPP_TARGET): tbsort.cpp tbsort.hpp
	$(CXX) $(CXXFLAGS) -DMAIN_TBSORT_CPP tbsort.cpp -o $(STANDALONE_CPP_TARGET) $(LDFLAGS_CPP)

# Clean rule
clean:
	rm -f $(BENCH_OBJS) tbsort_bench.o tbsort_int64.o tbsort.o tbsort_cpp_wrapper.o # Explicitly list all possible .o files
	rm -f $(BENCH_TARGET) $(STANDALONE_CPP_TARGET)

# Phony targets
.PHONY: all clean $(STANDALONE_CPP_TARGET)
