/* tbsort.c - A general O(n * log(log(n)) sort
 * Copyright (C) 2024 Tobias Bergmann
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

// This file implements the TBSort algorithm.
// gcc -o tbsort tbsort.c -lm

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h> // For srand and time

// Function to swap two elements
void swap(int* xp, int* yp) {
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}

// Function to print an array
void printArray(int arr[], int size) {
    int i;
    for (i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

// Function to clamp an integer n to be within lower and upper bounds
int myclamp(int n, int lower, int upper) {
    if (n < lower) return lower;
    if (n > upper) return upper;
    return n;
}

// Function to perform binary search for element e in array a of size n
// Returns the position of the element in the array that is <= e
// Assumes the array is sorted

// Comparison function for qsort
int compare_integers(const void* a, const void* b) {
    int int_a = *((const int*)a);
    int int_b = *((const int*)b);
    if (int_a < int_b) return -1;
    if (int_a > int_b) return 1;
    return 0;
}

int search(int a[], int n, int e) {
    int low = 0, high = n - 1;
    int ans = -1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (a[mid] <= e) {
            ans = mid;
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return ans;
}


// Function to implement insertion sort
void insertionSort(int arr[], int n) {
    if (n <= 1) return; // Added base case for insertion sort
    int i, key, j;
    for (i = 1; i < n; i++) {
        key = arr[i];
        j = i - 1;

        /* Move elements of arr[0..i-1], that are
           greater than key, to one position ahead
           of their current position */
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

#define BLOCK_SIZE 16

typedef struct BlockNode_s { // Use _s suffix to avoid conflict if BlockNode is used elsewhere
    int elements[BLOCK_SIZE];
    int count;
    struct BlockNode_s* next;
    int externally_allocated; // 0 for pool, 1 for malloc
} BlockNode;

typedef struct {
    BlockNode* head;
    BlockNode* tail;
    int total_elements;
} DynamicBin;

typedef struct {
    int* elements;
    int size;
} ConsolidatedBinInfo;

static BlockNode* allocate_new_block(DynamicBin* bin, char** pool_next_free_ptr, char* pool_end_ptr) {
    BlockNode* new_block_node = NULL;
    size_t block_node_size = sizeof(BlockNode);

    if (*pool_next_free_ptr + block_node_size <= pool_end_ptr) {
        new_block_node = (BlockNode*)(*pool_next_free_ptr);
        *pool_next_free_ptr += block_node_size;
        new_block_node->externally_allocated = 0;
    } else {
        new_block_node = (BlockNode*)malloc(block_node_size);
        if (!new_block_node) {
            perror("Failed to allocate new block node externally");
            return NULL; // Critical error or handle more gracefully
        }
        new_block_node->externally_allocated = 1;
    }

    new_block_node->count = 0;
    new_block_node->next = NULL;

    if (bin->head == NULL) {
        bin->head = new_block_node;
        bin->tail = new_block_node;
    } else {
        bin->tail->next = new_block_node;
        bin->tail = new_block_node;
    }
    return new_block_node;
}

// Structure for bins
// typedef struct {
//     int* elements;
//     int size;
//     int capacity;
// } Bin;


// TBSort function implementation
void TBSort(int arr[], int l, int r) {
    // 1. Base Case
    if (l >= r) {
        return;
    }

    int numElements = r - l + 1;

    // Handle very small arrays with insertion sort directly for stability
    if (numElements < 4) { // Threshold for direct sort
        if (numElements > 1) { // insertionSort handles n<=1
             // Need to copy to a temp array or adjust insertionSort to work on a sub-array
            int* subArray = (int*)malloc(numElements * sizeof(int));
            if (!subArray) {
                perror("Failed to allocate memory for subArray in TBSort");
                return; // Or handle error appropriately
            }
            memcpy(subArray, &arr[l], numElements * sizeof(int));
            insertionSort(subArray, numElements);
            memcpy(&arr[l], subArray, numElements * sizeof(int));
            free(subArray);
        }
        return;
    }

    // 2. TREE Step
    int treeSize;
    if (numElements < 4) { // e.g. log2(log2(3)) is problematic
        treeSize = 1; // Should be handled by the direct sort above, but as a fallback
    } else {
        double log2_numElements = log2(numElements);
        if (log2_numElements <= 0) log2_numElements = 1; // Avoid log2(0) or log2(negative) for log2(log2_numElements)
        double log2_log2_numElements = log2(log2_numElements);
        if (log2_log2_numElements < 0) log2_log2_numElements = 0; // Ensure power is not negative
        treeSize = (int)pow(2, round(log2_log2_numElements));
    }

    if (treeSize < 2 && numElements > 1) { // Ensure treeSize is at least 2 if numElements > 1
        treeSize = 2;
    }
    if (numElements <= 1) { // if numElements is 1, treeSize could become 0 from pow(2,round(log2(log2(1))))
        treeSize = 1; // or it's handled by l>=r base case.
    }
    // Ensure treeSize does not exceed numElements
    if (treeSize > numElements) {
        treeSize = numElements;
    }


    int* sampleTree = (int*)malloc(treeSize * sizeof(int));
    if (!sampleTree) {
        perror("Failed to allocate memory for sampleTree");
        return; // Or handle error appropriately
    }

    // Populate sampleTree with random elements from arr[l...r]
    for (int i = 0; i < treeSize; i++) {
        if (numElements == 0) continue; // Should not happen if l < r
        sampleTree[i] = arr[l + rand() % numElements];
    }
    insertionSort(sampleTree, treeSize); // Sort the sampleTree

    // 3. BIN Step
    int binCount;
    double logVal = log2(numElements);
    if (logVal <= 0 || numElements < 2) { // Avoid division by zero or log of <=1
        binCount = treeSize + 2; // Default or minimum binCount
    } else {
        binCount = (int)(numElements / logVal);
    }

    if (binCount < treeSize + 2) {
        binCount = treeSize + 2;
    }
    if (binCount == 0) binCount = 1; // Ensure at least one bin


    // Bin* bins = (Bin*)calloc(binCount, sizeof(Bin)); // Use calloc to zero-initialize
    // if (!bins) {
    //     perror("Failed to allocate memory for bins");
    //     free(sampleTree);
    //     return;
    // }

    // for (int i = 0; i < binCount; i++) {
    //     bins[i].capacity = 4; // Initial capacity
    //     bins[i].elements = (int*)malloc(bins[i].capacity * sizeof(int));
    //     if (!bins[i].elements) {
    //         perror("Failed to allocate memory for bin elements");
    //         // Free previously allocated memory
    //         for (int j = 0; j < i; j++) free(bins[j].elements);
    //         free(bins);
    //         free(sampleTree);
    //         return;
    //     }
    //     bins[i].size = 0;
    // }

    // Calculate targetbin, slope, and offset
    int targetbinSize = treeSize + 2;
    int* targetbin = (int*)malloc(targetbinSize * sizeof(int));
    float* slope = (float*)malloc((targetbinSize -1) * sizeof(float)); // size treeSize + 1
    float* offset = (float*)malloc((targetbinSize -1) * sizeof(float)); // size treeSize + 1

    if (!targetbin || !slope || !offset) {
         perror("Failed to allocate memory for targetbin/slope/offset");
         // for (int i = 0; i < binCount; i++) free(bins[i].elements); // Removed bins access
         // free(bins); // Removed bins access
         free(sampleTree);
         if(targetbin) free(targetbin);
         if(slope) free(slope);
         if(offset) free(offset);
         return;
    }

    targetbin[0] = 0;
    targetbin[targetbinSize - 1] = binCount -1; // Corrected to binCount - 1 as max index
    for (int i = 1; i < targetbinSize - 1; i++) { // Iterates treeSize times
        targetbin[i] = myclamp((int)roundf(numElements * (i) / (float)(treeSize +1) / logVal), 0, binCount -1);
    }
    // Ensure targetbin is sorted (it should be by construction, but good for robustness)
    // insertionSort(targetbin, targetbinSize); // Not strictly necessary if calculation is correct

    for (int i = 0; i < targetbinSize - 1; i++) { // Iterates treeSize + 1 times
        float x1 = (i == 0) ? (float)sampleTree[0] -1 : (float)sampleTree[i-1]; //Approximate M_i-1
        float x2 = (i == treeSize ) ? (float)sampleTree[treeSize-1] +1 : (float)sampleTree[i]; //Approximate M_i
        // Handle x1 == x2 case to avoid division by zero
        if (x1 >= x2) { // If x1 is not strictly less than x2, use a default slope/offset
            slope[i] = 0; // Or some other sensible default
            offset[i] = (float)targetbin[i];
        } else {
            slope[i] = (targetbin[i+1] - targetbin[i]) / (x2 - x1);
            offset[i] = targetbin[i] - slope[i] * x1;
        }
    }

    // main_pool will store BlockNode structures.
    // For now, we allocate it as char* to manage bytes for the bump allocator.
    char* main_pool = (char*)malloc(numElements * sizeof(int)); // Placeholder size
    if (!main_pool) {
        perror("Failed to allocate memory for main_pool");
        // Free sampleTree, targetbin, slope, offset before returning
        if (sampleTree) free(sampleTree);
        if (targetbin) free(targetbin);
        if (slope) free(slope);
        if (offset) free(offset);
        return;
    }
    char* pool_next_free_ptr = main_pool;
    char* pool_end_ptr = main_pool + numElements * sizeof(int);

    DynamicBin* dynamic_bins = (DynamicBin*)calloc(binCount, sizeof(DynamicBin));
    if (!dynamic_bins) {
        perror("Failed to allocate memory for dynamic_bins");
        // Free previously allocated memory: sampleTree, targetbin, slope, offset, main_pool
        if (sampleTree) free(sampleTree);
        if (targetbin) free(targetbin);
        if (slope) free(slope);
        if (offset) free(offset);
        if (main_pool) free(main_pool);
        return;
    }

    // int* final_bin_sizes = (int*)calloc(binCount, sizeof(int)); // No longer needed
    // if (!final_bin_sizes) {
    //     perror("Failed to allocate memory for final_bin_sizes");
    //     // Free other allocated memory before returning
    //     if (sampleTree) free(sampleTree);
    //     if (targetbin) free(targetbin);
    //     if (slope) free(slope);
    //     if (offset) free(offset);
    //     if (main_pool) free(main_pool);
    //     if (dynamic_bins) free(dynamic_bins);
    //     return;
    // }

    // Loop to distribute elements into bins
    for (int i = 0; i < numElements; i++) {
        int element_val = arr[l + i];

        // --- Begin: Calculate mybin_idx (copied from original/previous logic) ---
        int mypos = search(sampleTree, treeSize, element_val);
        int slope_offset_idx;
        if (mypos == -1) {
            slope_offset_idx = 0;
        } else if (mypos == treeSize - 1 && element_val >= sampleTree[treeSize - 1]) {
            slope_offset_idx = treeSize;
        } else {
            slope_offset_idx = mypos + 1;
        }
        // Ensure slope_offset_idx is within bounds for slope/offset arrays [0, treeSize]
        // treeSize for slope/offset arrays means treeSize+1 elements.
        // Max index is treeSize. Min index is 0.
        if (slope_offset_idx > treeSize) slope_offset_idx = treeSize;
        if (slope_offset_idx < 0) slope_offset_idx = 0; // Should not happen with logic above

        int mybin_idx = myclamp((int)roundf(element_val * slope[slope_offset_idx] + offset[slope_offset_idx]), 0, binCount - 1);
        // --- End: Calculate mybin_idx ---

        DynamicBin* current_bin = &dynamic_bins[mybin_idx];
        BlockNode* tail_block = current_bin->tail;

        if (tail_block == NULL || tail_block->count == BLOCK_SIZE) {
            tail_block = allocate_new_block(current_bin, &pool_next_free_ptr, pool_end_ptr);
            if (tail_block == NULL) {
                fprintf(stderr, "TBSort: Critical block allocation failure during distribution.\n");
                // Perform cleanup of all allocated resources similar to other error exits
                if (sampleTree) free(sampleTree);
                if (targetbin) free(targetbin);
                if (slope) free(slope);
                if (offset) free(offset);
                if (main_pool) free(main_pool);
                // Need to free BlockNodes allocated so far before freeing dynamic_bins array
                for (int b_idx = 0; b_idx < binCount; ++b_idx) {
                    BlockNode* bn = dynamic_bins[b_idx].head;
                    while (bn) {
                        BlockNode* next_bn = bn->next;
                        if (bn->externally_allocated) free(bn); // Only free if malloc'd
                        bn = next_bn;
                    }
                }
                if (dynamic_bins) free(dynamic_bins);
                // if (final_bin_sizes) free(final_bin_sizes); // final_bin_sizes is removed
                return; // Indicate failure
            }
        }

        tail_block->elements[tail_block->count++] = element_val;
        current_bin->total_elements++;
        // final_bin_sizes[mybin_idx]++; // No longer needed, use current_bin->total_elements
    }

    // final_bin_sizes is no longer needed and should have been removed.
    // If its free call is still present later, it should be removed.

    ConsolidatedBinInfo* consolidated_bins_info = (ConsolidatedBinInfo*)calloc(binCount, sizeof(ConsolidatedBinInfo));
    if (!consolidated_bins_info) {
        perror("Failed to allocate memory for consolidated_bins_info");
        // Full cleanup
        for (int b_idx = 0; b_idx < binCount; ++b_idx) {
            BlockNode* bn = dynamic_bins[b_idx].head;
            while (bn) {
                BlockNode* next_bn = bn->next;
                if (bn->externally_allocated) free(bn);
                bn = next_bn;
            }
        }
        if (dynamic_bins) free(dynamic_bins);
        if (sampleTree) free(sampleTree);
        if (targetbin) free(targetbin);
        if (slope) free(slope);
        if (offset) free(offset);
        if (main_pool) free(main_pool);
        return;
    }

    for (int i = 0; i < binCount; i++) {
        DynamicBin* current_bin = &dynamic_bins[i];
        consolidated_bins_info[i].elements = NULL;
        consolidated_bins_info[i].size = 0;

        if (current_bin->total_elements > 0) {
            consolidated_bins_info[i].size = current_bin->total_elements;
            consolidated_bins_info[i].elements = (int*)malloc(current_bin->total_elements * sizeof(int));

            if (!consolidated_bins_info[i].elements) {
                perror("Failed to allocate memory for a consolidated bin");
                consolidated_bins_info[i].size = 0;
                // Critical error: attempt to free already allocated parts and then the main array
                for (int k = 0; k < i; ++k) { // Free successfully allocated elements arrays before this one
                    if (consolidated_bins_info[k].elements) {
                        free(consolidated_bins_info[k].elements);
                        consolidated_bins_info[k].elements = NULL;
                    }
                }
                // Free the main consolidated_bins_info array itself as the process is incomplete
                if (consolidated_bins_info) free(consolidated_bins_info);
                consolidated_bins_info = NULL; // Mark as freed to avoid double free in main cleanup

                // Now, perform the rest of the cleanup like other major allocation failures
                for (int b_idx = 0; b_idx < binCount; ++b_idx) { // Cleanup dynamic_bins' BlockNodes
                    BlockNode* bn = dynamic_bins[b_idx].head;
                    while (bn) {
                        BlockNode* next_bn = bn->next;
                        if (bn->externally_allocated) free(bn);
                        bn = next_bn;
                    }
                }
                if (dynamic_bins) free(dynamic_bins);
                if (sampleTree) free(sampleTree);
                if (targetbin) free(targetbin);
                if (slope) free(slope);
                if (offset) free(offset);
                if (main_pool) free(main_pool);
                return; // Exit TBSort due to critical memory failure
            }

            if (consolidated_bins_info[i].elements) {
                int* writer_ptr = consolidated_bins_info[i].elements;
                BlockNode* bn = current_bin->head;
                while (bn) {
                    memcpy(writer_ptr, bn->elements, bn->count * sizeof(int));
                    writer_ptr += bn->count;
                    bn = bn->next;
                }
            }

            BlockNode* bn_to_free = current_bin->head;
            while (bn_to_free) {
                BlockNode* next_bn = bn_to_free->next;
                if (bn_to_free->externally_allocated) {
                    free(bn_to_free);
                }
                bn_to_free = next_bn;
            }
            current_bin->head = NULL;
            current_bin->tail = NULL;
            current_bin->total_elements = 0;
        }
    }

    // 4. SORT Step - Perform sorting on consolidated bins
    int binThreshold = 0;
    if (binCount > 0) {
        binThreshold = (int)(5 * (double)numElements / binCount);
    } else if (numElements > 0) {
        binThreshold = numElements + 1;
    }

    for (int i = 0; i < binCount; i++) {
        if (consolidated_bins_info[i].elements != NULL && consolidated_bins_info[i].size > 0) {
            if (consolidated_bins_info[i].size < binThreshold) {
                insertionSort(consolidated_bins_info[i].elements, consolidated_bins_info[i].size);
            } else {
                // Recursive call to TBSort
                TBSort(consolidated_bins_info[i].elements, 0, consolidated_bins_info[i].size - 1);
            }
        }
    }
    // The old memcpy from temp_arr to arr[l] will be replaced by a merge step later.

    int current_pos_in_arr = l;
    for (int i = 0; i < binCount; i++) {
        if (consolidated_bins_info[i].elements != NULL && consolidated_bins_info[i].size > 0) {
            if (current_pos_in_arr + consolidated_bins_info[i].size > r + 1) {
                fprintf(stderr, "TBSort: Error! Overrun while copying sorted bins back to original array. Data corruption likely.\n");
                // Free remaining elements to avoid memory leaks on error
                for (int j = i; j < binCount; ++j) { // Start from current bin
                    if (consolidated_bins_info[j].elements != NULL) {
                        free(consolidated_bins_info[j].elements);
                        consolidated_bins_info[j].elements = NULL;
                    }
                }
                break;
            }
            memcpy(&arr[current_pos_in_arr], consolidated_bins_info[i].elements, consolidated_bins_info[i].size * sizeof(int));
            current_pos_in_arr += consolidated_bins_info[i].size;

            free(consolidated_bins_info[i].elements);
            consolidated_bins_info[i].elements = NULL;
            consolidated_bins_info[i].size = 0;
        }
    }

    // free(bins); // Already handled
    if (sampleTree) free(sampleTree);
    if (targetbin) free(targetbin);
    if (slope) free(slope);
    if (offset) free(offset);
    if (dynamic_bins) free(dynamic_bins);
    if (consolidated_bins_info) free(consolidated_bins_info);
    // if (final_bin_sizes) free(final_bin_sizes); // Should be removed by now
    // if (temp_arr) free(temp_arr); // Removed
    if (main_pool) free(main_pool);
}

int main(int argc, char *argv[]) {
  srand(time(NULL)); // Seed random number generator

  // Test qsort
  int arr_bubble[] = {64, 34, 25, 12, 22, 11, 90};
  int n_bubble = sizeof(arr_bubble) / sizeof(arr_bubble[0]);
  printf("Array before qsort: \n");
  printArray(arr_bubble, n_bubble);
  qsort(arr_bubble, n_bubble, sizeof(int), compare_integers);
  printf("\nArray after qsort: \n");
  printArray(arr_bubble, n_bubble);
  printf("\n");

  // Test myclamp
  printf("Testing myclamp:\n");
  printf("myclamp(5, 0, 10) = %d\n", myclamp(5, 0, 10));   // Expected: 5
  printf("myclamp(-5, 0, 10) = %d\n", myclamp(-5, 0, 10));  // Expected: 0
  printf("myclamp(15, 0, 10) = %d\n", myclamp(15, 0, 10)); // Expected: 10
  printf("\n");

  // Test search (binary search)
  // Note: binary search requires a sorted array. Using the already sorted arr_bubble
  printf("Testing search (binary search on sorted array: ");
  printArray(arr_bubble, n_bubble);
  printf("search(arr_bubble, n_bubble, 22) = index %d (value %d)\n", search(arr_bubble, n_bubble, 22), arr_bubble[search(arr_bubble, n_bubble, 22)]); // Expected: index 2 (value 22)
  printf("search(arr_bubble, n_bubble, 23) = index %d (value %d)\n", search(arr_bubble, n_bubble, 23), arr_bubble[search(arr_bubble, n_bubble, 23)]); // Expected: index 2 (value 22)
  printf("search(arr_bubble, n_bubble, 10) = index %d\n", search(arr_bubble, n_bubble, 10)); // Expected: -1 (or behavior for element not found/less than smallest)
  printf("search(arr_bubble, n_bubble, 95) = index %d (value %d)\n", search(arr_bubble, n_bubble, 95), arr_bubble[search(arr_bubble, n_bubble, 95)]); // Expected: index 6 (value 90)
   printf("search(arr_bubble, n_bubble, 11) = index %d (value %d)\n", search(arr_bubble, n_bubble, 11), arr_bubble[search(arr_bubble, n_bubble, 11)]); // Expected: index 0 (value 11)
  printf("\n");

  // Test insertionSort
  int arr_insertion[] = {5, 1, 4, 2, 8};
  int n_insertion = sizeof(arr_insertion) / sizeof(arr_insertion[0]);
  printf("Array before insertion sort: \n");
  printArray(arr_insertion, n_insertion);
  insertionSort(arr_insertion, n_insertion);
  printf("\nArray after insertion sort: \n");
  printArray(arr_insertion, n_insertion);
  printf("\n");

  // Test TBSort
  int arr_tbsort[] = {12, 11, 13, 5, 6, 7, 1, 4, 17, 3, 5, 20, 22, 2, 100, 0, -5, 12, 5, 5, 22, -10, 30, 15};
  int n_tbsort = sizeof(arr_tbsort) / sizeof(arr_tbsort[0]);
  printf("Array before TBSort (new test case): \n");
  printArray(arr_tbsort, n_tbsort);
  TBSort(arr_tbsort, 0, n_tbsort - 1);
  printf("\nArray after TBSort (new test case): \n");
  printArray(arr_tbsort, n_tbsort);
  printf("\n");

  // Test TBSort with empty array
  int arr_empty[] = {};
  int n_empty = 0;
  printf("Array before TBSort (empty array): \n");
  printArray(arr_empty, n_empty);
  TBSort(arr_empty, 0, n_empty - 1);
  printf("\nArray after TBSort (empty array): \n");
  printArray(arr_empty, n_empty);
  printf("\n");

  // Test TBSort with single element array
  int arr_single[] = {42};
  int n_single = 1;
  printf("Array before TBSort (single element array): \n");
  printArray(arr_single, n_single);
  TBSort(arr_single, 0, n_single - 1);
  printf("\nArray after TBSort (single element array): \n");
  printArray(arr_single, n_single);
  printf("\n");

  // Test TBSort with array of all duplicates
  int arr_dups[] = {5, 5, 5, 5, 5};
  int n_dups = sizeof(arr_dups) / sizeof(arr_dups[0]);
  printf("Array before TBSort (all duplicates array): \n");
  printArray(arr_dups, n_dups);
  TBSort(arr_dups, 0, n_dups - 1);
  printf("\nArray after TBSort (all duplicates array): \n");
  printArray(arr_dups, n_dups);
  printf("\n");

  return 0;
}
