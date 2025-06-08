#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <string.h> // For memcpy
#include "tbsort_int64.h"

// Comparison function for qsort with int64_t
int compare_int64_t(const void* a, const void* b) {
    int64_t val_a = *(const int64_t*)a;
    int64_t val_b = *(const int64_t*)b;
    if (val_a < val_b) return -1;
    if (val_a > val_b) return 1;
    return 0;
}

// Function to generate an array of random int64_t values
int64_t* generate_random_int64_array(size_t n) {
    int64_t* arr = (int64_t*)malloc(n * sizeof(int64_t));
    if (!arr) {
        perror("Failed to allocate memory for random array");
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < n; i++) {
        // RAND_MAX is often 2^31-1 or 2^15-1. To get full int64_t range:
        // Combine multiple calls to rand().
        // This creates a pseudo-random 64-bit number.
        uint64_t val = 0;
        val |= (uint64_t)rand();
        val <<= 15; // Shift by 15 bits if RAND_MAX is around 2^15
        val |= (uint64_t)rand();
        val <<= 15;
        val |= (uint64_t)rand();
        val <<= 15;
        val |= (uint64_t)rand();
        val <<= 15; // ensure last bits also from rand()
        val |= (uint64_t)rand();
        arr[i] = (int64_t)val;
    }
    return arr;
}

// Function to copy an int64_t array
// (memcpy can be used directly, but this is a clear helper)
void copy_int64_array(const int64_t* src, int64_t* dst, size_t n) {
    if (src == NULL || dst == NULL) {
        fprintf(stderr, "Error: Null pointer passed to copy_int64_array.\n");
        return;
    }
    memcpy(dst, src, n * sizeof(int64_t));
}

int main() {
    size_t sizes[] = {50, 1000, 1000000, 20000000};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    srand(time(NULL)); // Seed random number generator

    printf("Starting benchmark comparisons between TBSort_int64 and qsort.\n");
    printf("=============================================================\n");

    for (int i = 0; i < num_sizes; i++) {
        size_t current_size = sizes[i];
        printf("\nBenchmarking for N = %zu elements:\n", current_size);

        // Generate original random array
        int64_t* arr_orig = generate_random_int64_array(current_size);
        if (!arr_orig) continue; // Should be handled by exit in generate_random_int64_array

        // Create copies for each sort
        int64_t* arr_tbsort = (int64_t*)malloc(current_size * sizeof(int64_t));
        if (!arr_tbsort) {
            perror("Failed to allocate memory for arr_tbsort");
            free(arr_orig);
            continue;
        }
        copy_int64_array(arr_orig, arr_tbsort, current_size);

        int64_t* arr_qsort = (int64_t*)malloc(current_size * sizeof(int64_t));
        if (!arr_qsort) {
            perror("Failed to allocate memory for arr_qsort");
            free(arr_orig);
            free(arr_tbsort);
            continue;
        }
        copy_int64_array(arr_orig, arr_qsort, current_size);

        // Benchmark TBSort_int64
        clock_t start_time = clock();
        TBSort_int64(arr_tbsort, 0, current_size - 1);
        clock_t end_time = clock();
        double tbsort_duration = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
        printf("  TBSort_int64 time: %f seconds\n", tbsort_duration);

        // Benchmark qsort
        start_time = clock();
        qsort(arr_qsort, current_size, sizeof(int64_t), compare_int64_t);
        end_time = clock();
        double qsort_duration = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
        printf("  qsort time:        %f seconds\n", qsort_duration);

        // Free allocated memory
        free(arr_orig);
        free(arr_tbsort);
        free(arr_qsort);

        printf("-------------------------------------------------------------\n");
    }

    printf("\nBenchmark finished.\n");
    return 0;
}
