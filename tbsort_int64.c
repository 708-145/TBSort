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
// gcc -o tbsort_int64 tbsort_int64.c -lm

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h> // For srand and time, and clock()
#include <stdint.h> // For int64_t
#include "tbsort_int64.h" // For TBSortTimings struct

// Tunable parameters
#define INSERTION_SORT_THRESHOLD 128
#define LEAF_BUFFER_INITIAL_CAPACITY 32
#define LEAF_BUFFER_GROWTH_FACTOR 2
#define SMALL_LEAF_BUFFER_THRESHOLD 96
#define LOCAL_BIN_INITIAL_CAPACITY 32
#define LOCAL_BIN_GROWTH_FACTOR 2

// Function to swap two elements
void swap(int64_t* xp, int64_t* yp) {
    int64_t temp = *xp;
    *xp = *yp;
    *yp = temp;
}

// Function to print an array (renamed and modified for int64_t)
void printArray_int64(int64_t arr[], int size) {
    int i;
    for (i = 0; i < size; i++) {
        printf("%ld ", arr[i]); // Use %ld for int64_t on this system
    }
    printf("\n");
}

// Function to clamp an int64_t n to be within lower and upper bounds
int64_t myclamp(int64_t n, int64_t lower, int64_t upper) {
    if (n < lower) return lower;
    if (n > upper) return upper;
    return n;
}

// Function to perform binary search for element e in array a of size n
// Returns the position of the element in the array that is <= e
// Assumes the array is sorted

// Comparison function for qsort is defined in tbsort_bench.c
/*
int compare_int64_t(const void* a, const void* b) {
    int64_t int_a = *((const int64_t*)a);
    int64_t int_b = *((const int64_t*)b);
    if (int_a < int_b) return -1;
    if (int_a > int_b) return 1;
    return 0;
}
*/

int search(int64_t a[], int n, int64_t e) {
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
void insertionSort(int64_t arr[], int n) {
    if (n <= 1) return; // Added base case for insertion sort
    int i;
    int64_t key;
    int j;
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

// Structure for bins
typedef struct {
    int64_t* elements;
    int size;
    int capacity;
} Bin;

// Structure for leaf node buffers (similar to Bin)
typedef struct {
    int64_t* elements;
    int size;
    int capacity;
} LeafBuffer;


// TBSort function implementation
void TBSort_int64(int64_t arr[], int l, int r, TBSortTimings* timings, int depth) {
    // Initialize timings at the top-level call
    if (depth == 0 && timings != NULL) {
        timings->tree_duration = 0.0;
        timings->bin_duration = 0.0;
        timings->sort_duration = 0.0;
    }

    // 1. Base Case
    if (l >= r) {
        return;
    }

    int numElements = r - l + 1;

    // Handle very small arrays with insertion sort directly for stability
    if (numElements < INSERTION_SORT_THRESHOLD) { // Threshold for direct sort
        if (numElements > 1) { // insertionSort handles n<=1
             // Need to copy to a temp array or adjust insertionSort to work on a sub-array
            int64_t* subArray = (int64_t*)malloc(numElements * sizeof(int64_t));
            if (!subArray) {
                perror("Failed to allocate memory for subArray in TBSort_int64");
                return; // Or handle error appropriately
            }
            memcpy(subArray, &arr[l], numElements * sizeof(int64_t));
            insertionSort(subArray, numElements);
            memcpy(&arr[l], subArray, numElements * sizeof(int64_t));
            free(subArray);
        }
        return;
    }

    // 2. TREE Step
    clock_t tree_start_time = (clock_t)0, tree_end_time = (clock_t)0;
    if (depth == 0 && timings != NULL) {
        tree_start_time = clock();
    }

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


    int64_t* sampleTree = (int64_t*)malloc(treeSize * sizeof(int64_t));
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

    // Initialize LeafBuffers
    LeafBuffer* leafBuffers = NULL;
    int numLeafBuffers = treeSize + 1;
    leafBuffers = (LeafBuffer*)malloc(numLeafBuffers * sizeof(LeafBuffer));
    if (!leafBuffers) {
        perror("Failed to allocate memory for leafBuffers");
        free(sampleTree);
        return; // Or handle error appropriately
    }

    for (int i = 0; i < numLeafBuffers; i++) {
        leafBuffers[i].capacity = LEAF_BUFFER_INITIAL_CAPACITY; // Initial capacity
        leafBuffers[i].elements = (int64_t*)malloc(leafBuffers[i].capacity * sizeof(int64_t));
        if (!leafBuffers[i].elements) {
            perror("Failed to allocate memory for leafBuffer elements");
            // Free previously allocated memory
            for (int j = 0; j < i; j++) {
                free(leafBuffers[j].elements);
            }
            free(leafBuffers);
            free(sampleTree);
            return; // Or handle error appropriately
        }
        leafBuffers[i].size = 0;
    }

    if (depth == 0 && timings != NULL) {
        tree_end_time = clock();
        timings->tree_duration += ((double)(tree_end_time - tree_start_time)) / CLOCKS_PER_SEC;
    }

    // 3. BIN Step (Phase 1: Preparation/Buffering)
    clock_t bin_start_time = (clock_t)0, bin_end_time = (clock_t)0;
    if (depth == 0 && timings != NULL) {
        bin_start_time = clock();
    }

    // Old binning logic (previously commented out) has been removed.
    // All related variables like binCount, logVal, bins, targetbin, slope, offset
    // and their memory management code (allocations, calculations, freeing)
    // have been deleted from this section and from the end of the function.

    // Phase 1: Distribute elements from arr[l...r] into leafBuffers
    // Note: The erroneous code block that was here previously, related to
    // `binCount`, `bins`, `targetbin`, `slope`, `offset` has been removed.
    for (int i = 0; i < numElements; i++) {
        int64_t element_val = arr[l + i];
        int mypos = search(sampleTree, treeSize, element_val);
        int leafBufferIndex;

        if (mypos == -1) { // Element is smaller than sampleTree[0]
            leafBufferIndex = 0;
        } else if (mypos == treeSize - 1 && element_val >= sampleTree[treeSize - 1]) { // Element is larger than or equal to the largest sample
            leafBufferIndex = treeSize;
        } else {
            leafBufferIndex = mypos + 1;
        }

        LeafBuffer* currentLeafBuffer = &leafBuffers[leafBufferIndex];

        if (currentLeafBuffer->size >= currentLeafBuffer->capacity) {
            currentLeafBuffer->capacity = (currentLeafBuffer->capacity == 0) ? 1 : currentLeafBuffer->capacity * LEAF_BUFFER_GROWTH_FACTOR;
            int64_t* new_elements = (int64_t*)realloc(currentLeafBuffer->elements, currentLeafBuffer->capacity * sizeof(int64_t));
            if (!new_elements) {
                perror("Failed to reallocate memory for leafBuffer elements in Phase 1");
                // Critical error: cleanup all allocated memory
                for (int k = 0; k < numLeafBuffers; k++) { // Iterate up to numLeafBuffers
                    if (leafBuffers[k].elements != NULL) { // Check individual elements
                        free(leafBuffers[k].elements);
                    }
                }
                free(leafBuffers); // Free the array of structs
                free(sampleTree);
                return; // Exit due to critical memory failure
            }
            currentLeafBuffer->elements = new_elements;
        }
        currentLeafBuffer->elements[currentLeafBuffer->size++] = element_val;
    }

    if (depth == 0 && timings != NULL) {
        bin_end_time = clock();
        timings->bin_duration += ((double)(bin_end_time - bin_start_time)) / CLOCKS_PER_SEC;
    }

    // Phase 2: Interpolation and Final Binning (replaces old SORT step)
    clock_t sort_start_time = (clock_t)0, sort_end_time = (clock_t)0;
    if (depth == 0 && timings != NULL) {
        sort_start_time = clock();
    }

    int curpos = l;
    int smallBufferThreshold = SMALL_LEAF_BUFFER_THRESHOLD; // Tunable threshold

    for (int i = 0; i < numLeafBuffers; i++) {
        LeafBuffer* currentLeafBuffer = &leafBuffers[i];

        if (currentLeafBuffer->size == 0) {
            continue;
        }

        if (currentLeafBuffer->size < smallBufferThreshold) {
            insertionSort(currentLeafBuffer->elements, currentLeafBuffer->size);
            if (curpos + currentLeafBuffer->size > r + 1) { /* Error guard */ return; }
            memcpy(&arr[curpos], currentLeafBuffer->elements, currentLeafBuffer->size * sizeof(int64_t));
            curpos += currentLeafBuffer->size;
        } else {
            // Large Buffer Handling
            int numElementsInLeaf = currentLeafBuffer->size;
            int64_t* currentElements = currentLeafBuffer->elements;

            int sub_binCount;
            double logValLeaf = log2(numElementsInLeaf);
            if (logValLeaf <= 0 || numElementsInLeaf < 2) {
                sub_binCount = 2;
            } else {
                sub_binCount = (int)(numElementsInLeaf / logValLeaf);
            }
            if (sub_binCount < 2) sub_binCount = 2;
            if (sub_binCount > numElementsInLeaf) sub_binCount = numElementsInLeaf;

            Bin* localBins = (Bin*)calloc(sub_binCount, sizeof(Bin));
            if (!localBins) {
                perror("Failed to allocate memory for localBins");
                // Comprehensive cleanup
                for (int k_lb = 0; k_lb < numLeafBuffers; k_lb++) if (leafBuffers[k_lb].elements) free(leafBuffers[k_lb].elements);
                free(leafBuffers);
                free(sampleTree);
                return;
            }

            for (int j = 0; j < sub_binCount; j++) {
                localBins[j].capacity = LOCAL_BIN_INITIAL_CAPACITY; // Initial capacity for local bins
                localBins[j].elements = (int64_t*)malloc(localBins[j].capacity * sizeof(int64_t));
                if (!localBins[j].elements) {
                    perror("Failed to allocate memory for localBin elements");
                    for (int k_lbe = 0; k_lbe < j; k_lbe++) free(localBins[k_lbe].elements);
                    free(localBins);
                    for (int k_lb = 0; k_lb < numLeafBuffers; k_lb++) if (leafBuffers[k_lb].elements) free(leafBuffers[k_lb].elements);
                    free(leafBuffers);
                    free(sampleTree);
                    return;
                }
                localBins[j].size = 0;
            }

            // Interpolation (Option A: min/max of currentElements)
            int64_t min_val_leaf = currentElements[0];
            int64_t max_val_leaf = currentElements[0];
            for (int k = 1; k < numElementsInLeaf; k++) {
                if (currentElements[k] < min_val_leaf) min_val_leaf = currentElements[k];
                if (currentElements[k] > max_val_leaf) max_val_leaf = currentElements[k];
            }

            float slope_leaf = 0;
            float offset_leaf = 0; // Or (float)min_val_leaf;

            if (min_val_leaf >= max_val_leaf) { // All elements are the same or only one element
                // All elements will go to the first bin or distribute evenly if preferred.
                // For simplicity, they'll all go to bin 0 with slope 0.
                slope_leaf = 0;
                offset_leaf = 0; // All map to bin 0
            } else {
                slope_leaf = (sub_binCount - 1.0f) / (max_val_leaf - min_val_leaf);
                offset_leaf = -slope_leaf * min_val_leaf;
            }

            // Distribute currentElements into localBins
            for (int k = 0; k < numElementsInLeaf; k++) {
                int64_t elem = currentElements[k];
                int local_bin_idx;
                if (min_val_leaf >= max_val_leaf) { // Handle case where all elements are same
                    local_bin_idx = 0;
                } else {
                    local_bin_idx = myclamp((int)roundf(elem * slope_leaf + offset_leaf), 0, sub_binCount - 1);
                }

                Bin* currentLocalBin = &localBins[local_bin_idx];
                if (currentLocalBin->size >= currentLocalBin->capacity) {
                    currentLocalBin->capacity = (currentLocalBin->capacity == 0) ? 1 : currentLocalBin->capacity * LOCAL_BIN_GROWTH_FACTOR;
                    int64_t* new_elements_local = (int64_t*)realloc(currentLocalBin->elements, currentLocalBin->capacity * sizeof(int64_t));
                    if (!new_elements_local) {
                        perror("Failed to reallocate for localBin elements");
                        // Comprehensive cleanup
                        for(int lbi=0; lbi < sub_binCount; ++lbi) if(localBins[lbi].elements) free(localBins[lbi].elements);
                        free(localBins);
                        for (int k_lb = 0; k_lb < numLeafBuffers; k_lb++) if (leafBuffers[k_lb].elements) free(leafBuffers[k_lb].elements);
                        free(leafBuffers);
                        free(sampleTree);
                        return;
                    }
                    currentLocalBin->elements = new_elements_local;
                }
                currentLocalBin->elements[currentLocalBin->size++] = elem;
            }

            // Sort localBins and copy to arr
            // int localBinThreshold = (int)(5 * numElementsInLeaf / (float)sub_binCount);
            // Using a fixed small threshold for simplicity, or same as smallBufferThreshold
            int localBinThreshold = smallBufferThreshold;


            for (int j = 0; j < sub_binCount; j++) {
                if (localBins[j].size == 0) {
                    free(localBins[j].elements);
                    continue;
                }
                if (localBins[j].size < localBinThreshold) {
                    insertionSort(localBins[j].elements, localBins[j].size);
                } else {
                    TBSort_int64(localBins[j].elements, 0, localBins[j].size - 1, timings, depth + 1);
                }
                if (curpos + localBins[j].size > r + 1) { /* Error guard */ return; }
                memcpy(&arr[curpos], localBins[j].elements, localBins[j].size * sizeof(int64_t));
                curpos += localBins[j].size;
                free(localBins[j].elements);
            }
            free(localBins);
        }
    }

    if (depth == 0 && timings != NULL) {
        sort_end_time = clock();
        timings->sort_duration = ((double)(sort_end_time - sort_start_time)) / CLOCKS_PER_SEC;
    }

    // Final cleanup of leafBuffers
    if (leafBuffers != NULL) {
        for (int i = 0; i < numLeafBuffers; i++) {
            if (leafBuffers[i].elements != NULL) {
                free(leafBuffers[i].elements);
            }
        }
        free(leafBuffers);
        leafBuffers = NULL; // Good practice
    }

    // Old frees for bins, targetbin, slope, offset are removed from here.
    free(sampleTree);
}

// Main function removed as per requirement
/*
int main(int argc, char *argv[]) {
  srand(time(NULL)); // Seed random number generator

  // Test qsort
  int64_t arr_bubble[] = {64, 34, 25, 12, 22, 11, 90};
  int n_bubble = sizeof(arr_bubble) / sizeof(arr_bubble[0]);
  printf("Array before qsort: \n");
  printArray_int64(arr_bubble, n_bubble);
  qsort(arr_bubble, n_bubble, sizeof(int64_t), compare_int64_t);
  printf("\nArray after qsort: \n");
  printArray_int64(arr_bubble, n_bubble);
  printf("\n");

  // Test myclamp
  printf("Testing myclamp:\n");
  printf("myclamp(5, 0, 10) = %lld\n", myclamp(5, 0, 10));   // Expected: 5
  printf("myclamp(-5, 0, 10) = %lld\n", myclamp(-5, 0, 10));  // Expected: 0
  printf("myclamp(15, 0, 10) = %lld\n", myclamp(15, 0, 10)); // Expected: 10
  printf("\n");

  // Test search (binary search)
  // Note: binary search requires a sorted array. Using the already sorted arr_bubble
  printf("Testing search (binary search on sorted array: ");
  printArray_int64(arr_bubble, n_bubble);
  printf("search(arr_bubble, n_bubble, 22) = index %d (value %lld)\n", search(arr_bubble, n_bubble, 22), arr_bubble[search(arr_bubble, n_bubble, 22)]);
  printf("search(arr_bubble, n_bubble, 23) = index %d (value %lld)\n", search(arr_bubble, n_bubble, 23), arr_bubble[search(arr_bubble, n_bubble, 23)]);
  printf("search(arr_bubble, n_bubble, 10) = index %d\n", search(arr_bubble, n_bubble, 10));
  printf("search(arr_bubble, n_bubble, 95) = index %d (value %lld)\n", search(arr_bubble, n_bubble, 95), arr_bubble[search(arr_bubble, n_bubble, 95)]);
  printf("search(arr_bubble, n_bubble, 11) = index %d (value %lld)\n", search(arr_bubble, n_bubble, 11), arr_bubble[search(arr_bubble, n_bubble, 11)]);
  printf("\n");

  // Test insertionSort
  int64_t arr_insertion[] = {5, 1, 4, 2, 8};
  int n_insertion = sizeof(arr_insertion) / sizeof(arr_insertion[0]);
  printf("Array before insertion sort: \n");
  printArray_int64(arr_insertion, n_insertion);
  insertionSort(arr_insertion, n_insertion);
  printf("\nArray after insertion sort: \n");
  printArray_int64(arr_insertion, n_insertion);
  printf("\n");

  // Test TBSort_int64
  int64_t arr_tbsort[] = {12, 11, 13, 5, 6, 7, 1, 4, 17, 3, 5, 20, 22, 2, 100, 0, -5, 12, 5, 5, 22, -10, 30, 15};
  int n_tbsort = sizeof(arr_tbsort) / sizeof(arr_tbsort[0]);
  printf("Array before TBSort_int64 (new test case): \n");
  printArray_int64(arr_tbsort, n_tbsort);
  TBSort_int64(arr_tbsort, 0, n_tbsort - 1);
  printf("\nArray after TBSort_int64 (new test case): \n");
  printArray_int64(arr_tbsort, n_tbsort);
  printf("\n");

  return 0;
}
*/
