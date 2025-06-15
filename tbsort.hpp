#ifndef TBSORT_HPP
#define TBSORT_HPP

#include <vector>
#include <cstdint>  // For int64_t
#include <algorithm> // For std::stable_sort, std::sort
#include <cmath>     // For log2, round, roundf
#include <cstdio>    // For printf
#include <cstring>   // For memcpy
#include <cstdlib>   // For rand()

// Tunable parameters
const int LEAF_BUFFER_INITIAL_CAPACITY = 32; // For std::vector::reserve
const int INSERTION_SORT_THRESHOLD = 128;
const int SMALL_LEAF_BUFFER_THRESHOLD = 96;
const int LOCAL_BIN_INITIAL_CAPACITY = 32;
// const int LOCAL_BIN_GROWTH_FACTOR = 2; // std::vector handles growth

// Structure for LeafBuffers
struct LeafBuffer {
    std::vector<int64_t> elements;
    // constructor to reserve initial capacity if desired
    LeafBuffer() {
        elements.reserve(LEAF_BUFFER_INITIAL_CAPACITY);
    }
};

// UTILITY FUNCTIONS
template <typename T>
T myclamp(const T& n, const T& lower, const T& upper) {
  return n <= lower ? lower : n >= upper ? upper : n;
}

// Function to print an array
void printArray(int64_t A[], int size);

// Function to perform binary search for element e in array a of size n
// Returns the index of the rightmost element in the array that is <= e
int search_le_element(const int64_t* arr, int n, int64_t e);

// l is for left index and r is
// right index of the sub-array
// of arr to be sorted
void TBSort(int64_t arr[], int l, int r);

#endif // TBSORT_HPP
