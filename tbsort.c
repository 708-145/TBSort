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

    int* final_bin_sizes = (int*)calloc(binCount, sizeof(int));
    if (!final_bin_sizes) {
        perror("Failed to allocate memory for final_bin_sizes");
        // Free other allocated memory before returning
        free(sampleTree);
        free(targetbin);
        free(slope);
        free(offset);
        return;
    }

    // Distribute elements from arr[l...r] into bins
    for (int i = 0; i < numElements; i++) {
        int element_val = arr[l + i];
        int mypos = search(sampleTree, treeSize, element_val); // search returns index of element <= e

        // Adjust mypos for slope/offset array indexing and logic
        // if element_val is smaller than smallest in sampleTree, search returns -1
        // if element_val is larger than largest in sampleTree, search returns treeSize-1
        // The slope/offset arrays are indexed from 0 to treeSize.
        // mypos from search: -1 to treeSize-1
        // if mypos is -1 (element_val < sampleTree[0]), use slope[0], offset[0] -> maps to M_(-1) to M_0
        // if mypos is k where sampleTree[k] <= element_val < sampleTree[k+1], use slope[k+1], offset[k+1] -> maps to M_k to M_{k+1}
        // if mypos is treeSize-1 and element_val >= sampleTree[treeSize-1], use slope[treeSize], offset[treeSize] -> maps to M_{treeSize-1} to M_{treeSize}

        int slope_offset_idx;
        if (mypos == -1) { // element_val < sampleTree[0]
            slope_offset_idx = 0;
        } else if (mypos == treeSize - 1 && element_val >= sampleTree[treeSize - 1]) { // element_val >= largest element in sampleTree
             // This means element_val falls in the last segment, defined by sampleTree[treeSize-1] and "infinity"
            slope_offset_idx = treeSize; // Use the last slope/offset pair
        } else {
            // sampleTree[mypos] <= element_val.
            // If element_val < sampleTree[mypos+1], then it's in segment mypos to mypos+1. Use slope_offset_idx = mypos + 1.
            // If element_val == sampleTree[mypos], it is also covered by mypos+1 (segment from sampleTree[mypos] to sampleTree[mypos+1])
            // unless sampleTree[mypos] == sampleTree[mypos+1]. The search gives first index k where a[k]<=e.
            slope_offset_idx = mypos + 1;
        }
        // slope and offset arrays have (treeSize+1) elements, indexed 0 to treeSize.
        // So slope_offset_idx should be clamped to [0, treeSize].
        // The above logic should result in slope_offset_idx in [0, treeSize].
        // Example: treeSize = 2. sampleTree has T0, T1.
        // slope/offset indices: 0, 1, 2.
        // < T0: mypos = -1. idx = 0. (Correct: uses M-1 to M0)
        // >= T1: mypos = 1. idx = 2. (Correct: uses M1 to M2 (infinity))
        // T0 <= val < T1: mypos = 0. idx = 1. (Correct: uses M0 to M1)

        int mybin_idx = myclamp((int)roundf(element_val * slope[slope_offset_idx] + offset[slope_offset_idx]), 0, binCount - 1);

        // Add element_val to bins[mybin_idx]
        // if (bins[mybin_idx].size >= bins[mybin_idx].capacity) {
        //     bins[mybin_idx].capacity = (bins[mybin_idx].capacity == 0) ? 1 : bins[mybin_idx].capacity * 2;
        //     int* new_elements = (int*)realloc(bins[mybin_idx].elements, bins[mybin_idx].capacity * sizeof(int));
        //     if (!new_elements) {
        //         perror("Failed to reallocate memory for bin elements");
        //         // Extensive cleanup needed here
        //         return;
        //     }
        //     bins[mybin_idx].elements = new_elements;
        // }
        // bins[mybin_idx].elements[bins[mybin_idx].size++] = element_val;
        final_bin_sizes[mybin_idx]++;
    }

    int* temp_arr = (int*)malloc(numElements * sizeof(int));
    if (!temp_arr) {
        perror("Failed to allocate memory for temp_arr");
        free(sampleTree);
        free(targetbin);
        free(slope);
        free(offset);
        free(final_bin_sizes); // final_bin_sizes was allocated before temp_arr
        return;
    }

    int* bin_write_pointers = (int*)malloc(binCount * sizeof(int));
    if (!bin_write_pointers) {
        perror("Failed to allocate memory for bin_write_pointers");
        free(sampleTree);
        free(targetbin);
        free(slope);
        free(offset);
        free(final_bin_sizes);
        free(temp_arr); // temp_arr was allocated before bin_write_pointers
        return;
    }

    int current_offset = 0;
    for (int i = 0; i < binCount; i++) {
        bin_write_pointers[i] = current_offset;
        current_offset += final_bin_sizes[i];
    }

    for (int i = 0; i < numElements; i++) {
        int element_val = arr[l + i];
        // Copy the logic for calculating mybin_idx from the sizing loop
        int mypos = search(sampleTree, treeSize, element_val);
        int slope_offset_idx;
        if (mypos == -1) { slope_offset_idx = 0; }
        else if (mypos == treeSize - 1 && element_val >= sampleTree[treeSize - 1]) { slope_offset_idx = treeSize; }
        else { slope_offset_idx = mypos + 1; }
        int mybin_idx = myclamp((int)roundf(element_val * slope[slope_offset_idx] + offset[slope_offset_idx]), 0, binCount - 1);

        temp_arr[bin_write_pointers[mybin_idx]] = element_val;
        bin_write_pointers[mybin_idx]++;
    }

    free(bin_write_pointers);

    // 4. SORT Step
    int binThreshold = (int)(5 * numElements / (float)binCount);
    if (binCount == 0) binThreshold = numElements +1; // Avoid division by zero if binCount somehow is 0

    // int curpos = l; // No longer used here
    int current_temp_arr_offset = 0;
    for (int i = 0; i < binCount; i++) {
        int current_bin_size = final_bin_sizes[i];
        if (current_bin_size == 0) {
            // free(bins[i].elements); // Already handled, bins[i].elements doesn't exist
            current_temp_arr_offset += current_bin_size; // Should be 0, but for completeness
            continue;
        }

        int bin_l_in_temp = current_temp_arr_offset;
        int bin_r_in_temp = current_temp_arr_offset + current_bin_size - 1;

        if (current_bin_size < binThreshold) {
            insertionSort(&temp_arr[bin_l_in_temp], current_bin_size);
        } else {
            TBSort(temp_arr, bin_l_in_temp, bin_r_in_temp);
        }

        // memcpy(&arr[curpos], bins[i].elements, bins[i].size * sizeof(int)); // Replaced by copy back from temp_arr
        // curpos += bins[i].size; // Replaced by overall copy
        // free(bins[i].elements); // Already handled

        current_temp_arr_offset += current_bin_size;
    }

    if (temp_arr) { // Ensure temp_arr was allocated before trying to copy from it
        memcpy(&arr[l], temp_arr, numElements * sizeof(int));
    }

    // free(bins); // Already handled
    if (sampleTree) free(sampleTree);
    if (targetbin) free(targetbin);
    if (slope) free(slope);
    if (offset) free(offset);
    if (final_bin_sizes) free(final_bin_sizes);
    if (temp_arr) free(temp_arr);
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
