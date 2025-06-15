#include "tbsort_cpp_wrapper.h"
#include "tbsort.hpp" // To get the declaration of the C++ TBSort
#include <algorithm> // For std::sort and std::stable_sort
// No need for <vector> if we are just sorting raw arrays

// Definition of the extern "C" function
void TBSort_cpp_int64(int64_t arr[], int l, int r) {
    TBSort(arr, l, r); // Call the actual C++ TBSort function
}

void StdSort_cpp_int64(int64_t arr[], size_t n) {
    std::sort(arr, arr + n);
}

void StdStableSort_cpp_int64(int64_t arr[], size_t n) {
    std::stable_sort(arr, arr + n);
}
