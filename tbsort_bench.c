#include <stdio.h>
#include <stdlib.h> // For strtoull, exit, EXIT_FAILURE
#include <time.h>
#include <stdint.h>
#include <string.h> // For memcpy, strcmp
#include <limits.h> // For ULLONG_MAX (though not directly used, good for context of strtoull)
#include "tbsort_int64.h" // C version of TBSort
#include "tbsort_cpp_wrapper.h" // C++ version of TBSort (via wrapper)

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
        uint64_t val = 0;
        val |= (uint64_t)rand(); val <<= 15;
        val |= (uint64_t)rand(); val <<= 15;
        val |= (uint64_t)rand(); val <<= 15;
        val |= (uint64_t)rand(); val <<= 15;
        val |= (uint64_t)rand();
        arr[i] = (int64_t)val;
    }
    return arr;
}

// Function to copy an int64_t array
void copy_int64_array(const int64_t* src, int64_t* dst, size_t n) {
    if (src == NULL || dst == NULL) {
        fprintf(stderr, "Error: Null pointer passed to copy_int64_array.\n");
        return;
    }
    memcpy(dst, src, n * sizeof(int64_t));
}

int main(int argc, char *argv[]) {
    static size_t default_sizes[] = {30, 100, 300, 1000, 3000, 10000, 30000, 100000, 300000, 1000000, 3000000, 10000000, 30000000, 100000000};
    size_t single_n_from_arg_array[1]; // For holding the N if provided via arg

    size_t* sizes_to_run_ptr;
    int num_sizes_to_run;

    if (argc == 1) {
        // No arguments, use default sizes
        sizes_to_run_ptr = default_sizes;
        num_sizes_to_run = sizeof(default_sizes) / sizeof(default_sizes[0]);
    } else if (argc == 2) {
        char *endptr;
        unsigned long long n_ull = strtoull(argv[1], &endptr, 10);

        if (endptr == argv[1] || *endptr != '\0') {
            fprintf(stderr, "Error: Invalid character in N value '%s'. N must be a positive integer.\n", argv[1]);
            return EXIT_FAILURE;
        }
        // Note: strtoull returns 0 for "0" and ULLONG_MAX for overflow.
        // We want N > 0. SIZE_MAX is a good upper bound from stddef.h (implicitly via stdlib.h).
        if (n_ull == 0) { // Catches "0" and non-numeric strings that parse as 0 before endptr check.
            fprintf(stderr, "Error: Invalid N value '%s'. N must be a positive integer > 0.\n", argv[1]);
            return EXIT_FAILURE;
        }
        if (n_ull > SIZE_MAX) { // Check if it exceeds what size_t can hold
             fprintf(stderr, "Error: N value %llu is too large (must be <= %zu).\n", n_ull, SIZE_MAX);
             return EXIT_FAILURE;
        }

        single_n_from_arg_array[0] = (size_t)n_ull;
        sizes_to_run_ptr = single_n_from_arg_array;
        num_sizes_to_run = 1;
    } else {
        fprintf(stderr, "Usage: %s [N]\n", argv[0]);
        fprintf(stderr, "  N (optional): A specific array size to benchmark.\n");
        fprintf(stderr, "  If N is not provided, a default set of array sizes will be benchmarked.\n");
        return EXIT_FAILURE;
    }

    srand(time(NULL));

    printf("Starting benchmark comparisons between TBSort_int64 (C), TBSort_cpp_int64 (C++), qsort, std::sort (C++), and std::stable_sort (C++).\n");
    printf("===========================================================================================\n");

    for (int i = 0; i < num_sizes_to_run; i++) {
        size_t current_size = sizes_to_run_ptr[i];
        printf("\nBenchmarking for N = %zu elements:\n", current_size);

        int64_t* arr_orig = generate_random_int64_array(current_size);
        if (!arr_orig) continue;

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

        int64_t* arr_std_sort = (int64_t*)malloc(current_size * sizeof(int64_t));
        if (!arr_std_sort) {
            perror("Failed to allocate memory for arr_std_sort");
            free(arr_orig);
            free(arr_tbsort);
            free(arr_qsort);
            continue;
        }
        copy_int64_array(arr_orig, arr_std_sort, current_size);

        int64_t* arr_std_stable_sort = (int64_t*)malloc(current_size * sizeof(int64_t));
        if (!arr_std_stable_sort) {
            perror("Failed to allocate memory for arr_std_stable_sort");
            free(arr_orig);
            free(arr_tbsort);
            free(arr_qsort);
            free(arr_std_sort);
            continue;
        }
        copy_int64_array(arr_orig, arr_std_stable_sort, current_size);

        TBSortTimings tbs_timings;
        tbs_timings.tree_duration = 0.0;
        tbs_timings.bin_duration = 0.0;
        tbs_timings.sort_duration = 0.0;

        clock_t tbs_start_time = clock();
        TBSort_int64(arr_tbsort, 0, current_size - 1, &tbs_timings, 0);
        clock_t tbs_end_time = clock();
        double tbsort_duration = ((double)(tbs_end_time - tbs_start_time)) / CLOCKS_PER_SEC;

        printf("  TBSort_int64 time: %f seconds (Tree: %f, Bin: %f, Sort: %f)\n",
               tbsort_duration, tbs_timings.tree_duration,
               tbs_timings.bin_duration, tbs_timings.sort_duration);

        clock_t qsort_start_time = clock();
        qsort(arr_qsort, current_size, sizeof(int64_t), compare_int64_t);
        clock_t qsort_end_time = clock();
        double qsort_duration = ((double)(qsort_end_time - qsort_start_time)) / CLOCKS_PER_SEC;
        printf("  qsort time:        %f seconds\n", qsort_duration);

        // Free arrays used by C TBSort and qsort
        free(arr_tbsort);
        free(arr_qsort);

        // C++ TBSort
        int64_t* arr_tbsort_cpp = (int64_t*)malloc(current_size * sizeof(int64_t));
        if (!arr_tbsort_cpp) {
            perror("Failed to allocate memory for arr_tbsort_cpp");
            free(arr_orig); // arr_orig is the only one remaining from the common set
            continue;
        }
        copy_int64_array(arr_orig, arr_tbsort_cpp, current_size); // arr_orig is still valid here

        clock_t tbs_cpp_start_time = clock();
        TBSort_cpp_int64(arr_tbsort_cpp, 0, current_size - 1);
        clock_t tbs_cpp_end_time = clock();
        double tbsort_cpp_duration = ((double)(tbs_cpp_end_time - tbs_cpp_start_time)) / CLOCKS_PER_SEC;
        printf("  TBSort_cpp_int64 time: %f seconds\n", tbsort_cpp_duration);
        free(arr_tbsort_cpp);

        // std::sort
        clock_t std_sort_start_time = clock();
        StdSort_cpp_int64(arr_std_sort, current_size);
        clock_t std_sort_end_time = clock();
        double std_sort_duration = ((double)(std_sort_end_time - std_sort_start_time)) / CLOCKS_PER_SEC;
        printf("  std::sort time:    %f seconds\n", std_sort_duration);
        free(arr_std_sort);

        // std::stable_sort
        clock_t std_stable_sort_start_time = clock();
        StdStableSort_cpp_int64(arr_std_stable_sort, current_size);
        clock_t std_stable_sort_end_time = clock();
        double std_stable_sort_duration = ((double)(std_stable_sort_end_time - std_stable_sort_start_time)) / CLOCKS_PER_SEC;
        printf("  std::stable_sort time: %f seconds\n", std_stable_sort_duration);
        free(arr_std_stable_sort);

        // Now free the original array for this iteration
        free(arr_orig);

        printf("-------------------------------------------------------------\n");
    }

    printf("\nBenchmark finished.\n");
    return 0;
}
