#include "tbsort_cpp_wrapper.h"
#include "tbsort.hpp" // To get the declaration of the C++ TBSort

// Definition of the extern "C" function
void TBSort_cpp_int64(int64_t arr[], int l, int r) {
    TBSort(arr, l, r); // Call the actual C++ TBSort function
}
