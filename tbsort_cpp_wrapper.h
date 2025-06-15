#ifndef TBSORT_CPP_WRAPPER_H
#define TBSORT_CPP_WRAPPER_H

#include <stddef.h> // For size_t
#include <stdint.h> // For int64_t (C-compatible header)

#ifdef __cplusplus
extern "C" {
#endif

void TBSort_cpp_int64(int64_t arr[], int l, int r);
void StdSort_cpp_int64(int64_t arr[], size_t n);
void StdStableSort_cpp_int64(int64_t arr[], size_t n);

#ifdef __cplusplus
}
#endif

#endif // TBSORT_CPP_WRAPPER_H
