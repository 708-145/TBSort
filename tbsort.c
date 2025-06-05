// This file implements a sorting algorithm.

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

// Function to implement bubble sort
void bubbleSort(int arr[], int n) {
    int i, j;
    for (i = 0; i < n - 1; i++) {
        // Last i elements are already in place
        for (j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                swap(&arr[j], &arr[j + 1]);
            }
        }
    }
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
typedef struct {
    int* elements;
    int size;
    int capacity;
} Bin;

// TBSort function (prototype for now)
void TBSort(int arr[], int l, int r);

int main(int argc, char *argv[]) {
  srand(time(NULL)); // Seed random number generator

  // Test bubbleSort
  int arr_bubble[] = {64, 34, 25, 12, 22, 11, 90};
  int n_bubble = sizeof(arr_bubble) / sizeof(arr_bubble[0]);
  printf("Array before bubble sort: \n");
  printArray(arr_bubble, n_bubble);
  bubbleSort(arr_bubble, n_bubble);
  printf("\nArray after bubble sort: \n");
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


  return 0;
}

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


    Bin* bins = (Bin*)calloc(binCount, sizeof(Bin)); // Use calloc to zero-initialize
    if (!bins) {
        perror("Failed to allocate memory for bins");
        free(sampleTree);
        return;
    }

    for (int i = 0; i < binCount; i++) {
        bins[i].capacity = 4; // Initial capacity
        bins[i].elements = (int*)malloc(bins[i].capacity * sizeof(int));
        if (!bins[i].elements) {
            perror("Failed to allocate memory for bin elements");
            // Free previously allocated memory
            for (int j = 0; j < i; j++) free(bins[j].elements);
            free(bins);
            free(sampleTree);
            return;
        }
        bins[i].size = 0;
    }

    // Calculate targetbin, slope, and offset
    int targetbinSize = treeSize + 2;
    int* targetbin = (int*)malloc(targetbinSize * sizeof(int));
    float* slope = (float*)malloc((targetbinSize -1) * sizeof(float)); // size treeSize + 1
    float* offset = (float*)malloc((targetbinSize -1) * sizeof(float)); // size treeSize + 1

    if (!targetbin || !slope || !offset) {
         perror("Failed to allocate memory for targetbin/slope/offset");
         for (int i = 0; i < binCount; i++) free(bins[i].elements);
         free(bins);
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
        if (bins[mybin_idx].size >= bins[mybin_idx].capacity) {
            bins[mybin_idx].capacity = (bins[mybin_idx].capacity == 0) ? 1 : bins[mybin_idx].capacity * 2;
            int* new_elements = (int*)realloc(bins[mybin_idx].elements, bins[mybin_idx].capacity * sizeof(int));
            if (!new_elements) {
                perror("Failed to reallocate memory for bin elements");
                // Extensive cleanup needed here
                return;
            }
            bins[mybin_idx].elements = new_elements;
        }
        bins[mybin_idx].elements[bins[mybin_idx].size++] = element_val;
    }

    // 4. SORT Step
    int binThreshold = (int)(5 * numElements / (float)binCount);
    if (binCount == 0) binThreshold = numElements +1; // Avoid division by zero if binCount somehow is 0

    int curpos = l;
    for (int i = 0; i < binCount; i++) {
        if (bins[i].size == 0) {
            free(bins[i].elements); // Free even if empty, as it was allocated
            continue;
        }

        if (bins[i].size < binThreshold) {
            insertionSort(bins[i].elements, bins[i].size);
        } else {
            TBSort(bins[i].elements, 0, bins[i].size - 1);
        }

        if (curpos + bins[i].size > r + 1) {
            // Error: trying to write past the allocated space for arr segment
            // This indicates an issue with bin distribution or numElements calculation
            fprintf(stderr, "Error: TBSort trying to write out of bounds.\n");
            // Free remaining allocated memory
            for (int k = i; k < binCount; k++) free(bins[k].elements);
            free(bins);
            free(sampleTree);
            free(targetbin);
            free(slope);
            free(offset);
            return; // Critical error
        }
        memcpy(&arr[curpos], bins[i].elements, bins[i].size * sizeof(int));
        curpos += bins[i].size;
        free(bins[i].elements);
    }

    free(bins);
    free(sampleTree);
    free(targetbin);
    free(slope);
    free(offset);
}
