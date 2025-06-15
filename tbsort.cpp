/* tbsort.cpp - A general O(n * log(log(n)) sort
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

#include "tbsort.hpp" // Include the header file

// All necessary standard library headers like <cmath>, <vector>, <cstdio>,
// <cstring>, <algorithm>, <cstdint>, <cstdlib> are now included via tbsort.hpp.

// compile:	 g++ -o tbsort tbsort.cpp -lm -std=c++11; ./tbsort
// (When compiling, tbsort.hpp is implicitly compiled via its inclusion in tbsort.cpp)

// Tunable parameters are now in tbsort.hpp
// Structure for LeafBuffers is now in tbsort.hpp
// UTILITY FUNCTIONS
// myclamp template is now fully in tbsort.hpp

// Function to print an array (implementation remains)
void printArray(int64_t A[], int size)
{
    for (int i = 0; i < size; i++)
        printf("%ld ", A[i]);
    printf("\n");
}

// Function to perform binary search for element e in array a of size n
// Returns the index of the rightmost element in the array that is <= e
// Assumes the array is sorted.
// Returns -1 if e is smaller than all elements or if the array is empty.
int search_le_element(const int64_t* arr, int n, int64_t e) {
    if (n <= 0) { // Handle empty array
        return -1;
    }
    int low = 0, high = n - 1;
    int ans = -1;

    while (low <= high) {
        int mid = low + (high - low) / 2; // Avoid potential overflow
        if (arr[mid] <= e) {
            ans = mid;
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    // After the loop, if ans is -1, it means e is smaller than all elements.
    // If e is not smaller than all elements, ans will hold the index of the largest element <= e.
    // If arr[0] > e (first element is already greater than e), ans remains -1, which is correct.
    return ans;
}

// l is for left index and r is
// right index of the sub-array
// of arr to be sorted
void TBSort(int64_t arr[],
            int l, int r)
{
    int numElements = r - l + 1;
    if (numElements <= 1) { // Base case for recursion, also handles l < r for single element.
        return;
    }

    // Handle small arrays with insertion sort directly for stability and efficiency
    if (numElements < INSERTION_SORT_THRESHOLD) {
        if (numElements > 1) { // std::stable_sort requires at least 2 elements for a range
             std::stable_sort(arr + l, arr + l + numElements);
        }
        return;
    }

    if (l < r) // This check is now mostly for the main logic, base cases handled above
    {
		// TREE step : sample and sort tree content
		// int numElements = r - l + 1; // Moved up
		int treeSize = pow(2,round(log2(log2(numElements))));
        if (treeSize == 0 && numElements > 1) treeSize = 1; // Ensure treeSize is at least 1 if there are elements
        else if (treeSize == 0 && numElements <=1) return; // Should be caught by earlier base cases

		int64_t sampleTree[treeSize];
		for (int i = 0; i < treeSize; i++) sampleTree[i] = arr[ l + (rand() % (r - l + 1)) ]; // TODO: sample unique values
		std::sort(sampleTree, sampleTree+treeSize);
		//printf("Search Tree: "); printArray(sampleTree, treeSize);

		// Phase 1: Distribute elements from arr[l...r] into leafBuffers
		int numLeafBuffers = treeSize + 1;
		std::vector<LeafBuffer> leafBuffers(numLeafBuffers);

		for (int i = 0; i < numElements; ++i) {
		    int64_t element_val = arr[l + i];
		    int mypos = search_le_element(sampleTree, treeSize, element_val);
		    int leafBufferIndex;

		    if (mypos == -1) { // Element is smaller than sampleTree[0]
		        leafBufferIndex = 0;
		    } else if (mypos == treeSize - 1 && element_val >= sampleTree[treeSize - 1]) { // Element is larger than or equal to the largest sample
		        leafBufferIndex = treeSize; // This is numLeafBuffers - 1
		    } else {
		        leafBufferIndex = mypos + 1;
		    }
		    // Ensure index is within bounds, though logic should prevent out of bounds
            leafBufferIndex = myclamp(leafBufferIndex, 0, numLeafBuffers - 1);
		    leafBuffers[leafBufferIndex].elements.push_back(element_val);
		}

		// Phase 2: Process LeafBuffers
		int current_arr_idx = l;
		for (LeafBuffer& currentLeafBuffer : leafBuffers) {
		    if (currentLeafBuffer.elements.empty()) {
		        continue;
		    }

		    if (currentLeafBuffer.elements.size() < SMALL_LEAF_BUFFER_THRESHOLD) {
		        std::stable_sort(currentLeafBuffer.elements.begin(), currentLeafBuffer.elements.end());
		        memcpy(&arr[current_arr_idx], currentLeafBuffer.elements.data(), currentLeafBuffer.elements.size() * sizeof(int64_t));
		        current_arr_idx += currentLeafBuffer.elements.size();
		    } else {
		        // Large buffer handling - Implement Local Bin strategy
		        size_t numElementsInLeaf = currentLeafBuffer.elements.size();
		        std::vector<int64_t>& currentElements = currentLeafBuffer.elements; // Use reference

		        size_t sub_binCount; // Changed to size_t
		        double logValLeaf = (numElementsInLeaf > 1) ? log2(numElementsInLeaf) : 0.0;
		        if (logValLeaf <= 0 || numElementsInLeaf < 2) {
		            sub_binCount = 2;
		        } else {
		            sub_binCount = (size_t)(numElementsInLeaf / logValLeaf); // Cast to size_t
		        }
		        if (sub_binCount < 2) sub_binCount = 2;
		        // Ensure sub_binCount is not excessively large if numElementsInLeaf is very small (e.g., 2, 3)
		        // logValLeaf can be small, making numElementsInLeaf / logValLeaf large.
		        if (sub_binCount > numElementsInLeaf && numElementsInLeaf >=2) sub_binCount = numElementsInLeaf;


		        std::vector<std::vector<int64_t>> localBins(sub_binCount);
		        for(size_t j=0; j < sub_binCount; ++j) { // Reserve capacity for inner vectors (j also size_t)
		            localBins[j].reserve(LOCAL_BIN_INITIAL_CAPACITY);
		        }

		        if (numElementsInLeaf == 0) { // Should be caught by currentLeafBuffer.elements.empty()
		            continue;
		        }

		        int64_t min_val_leaf = currentElements[0];
		        int64_t max_val_leaf = currentElements[0];
		        for (size_t k = 1; k < numElementsInLeaf; ++k) {
		            if (currentElements[k] < min_val_leaf) min_val_leaf = currentElements[k];
		            if (currentElements[k] > max_val_leaf) max_val_leaf = currentElements[k];
		        }

		        float slope_val = 0.0f;
		        float offset_val = 0.0f;
		        if (min_val_leaf >= max_val_leaf) {
		            slope_val = 0.0f;
		            offset_val = 0.0f; // All elements will go to bin 0
		        } else {
		            slope_val = (float)((sub_binCount > 0 ? sub_binCount - 1 : 0) * 1.0f) / (max_val_leaf - min_val_leaf); // Ensure sub_binCount-1 is not negative if sub_binCount is 0 (though it's set to min 2)
		            offset_val = -slope_val * min_val_leaf;
		        }

		        for (int64_t elem : currentElements) {
		            size_t local_bin_idx; // Changed to size_t
		            if (min_val_leaf >= max_val_leaf) {
		                local_bin_idx = 0;
		            } else {
		                // Ensure myclamp's third argument is not negative if sub_binCount is 0 or 1.
		                // It's guaranteed sub_binCount >= 2, so sub_binCount - 1 >= 1.
		                local_bin_idx = myclamp((size_t)roundf(elem * slope_val + offset_val), (size_t)0, sub_binCount - 1);
		            }
		            localBins[local_bin_idx].push_back(elem);
		        }

		        // Sort localBins and copy back to arr
		        for (size_t j = 0; j < sub_binCount; ++j) { // j also size_t
		            if (localBins[j].empty()) {
		                continue;
		            }
		            if (localBins[j].size() < SMALL_LEAF_BUFFER_THRESHOLD) { // Using same threshold for local bins
		                std::stable_sort(localBins[j].begin(), localBins[j].end());
		            } else {
		                // Pass the data pointer of the vector for recursive call
		                TBSort(localBins[j].data(), 0, localBins[j].size() - 1);
		            }
		            memcpy(&arr[current_arr_idx], localBins[j].data(), localBins[j].size() * sizeof(int64_t));
		            current_arr_idx += localBins[j].size();
		        }
		    }
		}
		// Ensure all elements from arr[l...r] have been processed and placed back.
		// This assertion might be useful for debugging:
		// assert(current_arr_idx == r + 1);

    }
}

#ifdef MAIN_TBSORT_CPP // Guard for the main function in tbsort.cpp
int main()
{
    int64_t arr[] = {12, 11, 13, 5, 6, 7, 1, 4, 17, 3, 5, 20, 22, 2};
    int arr_size = sizeof(arr) / sizeof(arr[0]);

    printf("Given array is ");
    printArray(arr, arr_size);

    TBSort(arr, 0, arr_size - 1);

    printf("\nTBSorted array is ");
    printArray(arr, arr_size);
    return 0;
}
#endif // MAIN_TBSORT_CPP
