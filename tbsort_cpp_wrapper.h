#ifndef TBSORT_CPP_WRAPPER_H
#define TBSORT_CPP_WRAPPER_H

#include <stdint.h> // For int64_t (C-compatible header)

#ifdef __cplusplus
extern "C" {
#endif

void TBSort_cpp_int64(int64_t arr[], int l, int r);

#ifdef __cplusplus
}
#endif

#endif // TBSORT_CPP_WRAPPER_H
