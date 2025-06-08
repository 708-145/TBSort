#ifndef TBSORT_INT64_H
#define TBSORT_INT64_H

#include <stdint.h> // For int64_t
#include <stddef.h> // For size_t (though not strictly in TBSort_int64 signature, good for related code)

// Define a new struct for timing information
typedef struct {
    double tree_duration;
    double bin_duration;
    double sort_duration;
} TBSortTimings;

// Forward declaration for the TBSort function adapted for int64_t
void TBSort_int64(int64_t arr[], int l, int r, TBSortTimings* timings, int depth);

#endif // TBSORT_INT64_H
