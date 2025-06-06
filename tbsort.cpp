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

#include <math.h>
#include <vector>
#include <cstdio>
#include <cstring>
#include <bits/stdc++.h>

// compile:	 gcc -o tbsort tbsort.cpp -lm -lstdc++; ./tbsort

// UTILITY FUNCTIONS
template <typename T>
T myclamp(const T& n, const T& lower, const T& upper) {
  return n <= lower ? lower : n >= upper ? upper : n;
}

// Function to print an array
void printArray(int A[], int size)
{
    for (int i = 0; i < size; i++)
        printf("%d ", A[i]);
    printf("\n");
}

// Binary search in array a of size n for element e
// Returns position of element in array that is <= e
int search(int a[], int n, int e)
{
    int l,r,m;
    l=0;
    r=n-1;

    while(l<=r)
    {
        m=(l+r)/2;
        if(e==a[m])
            return(m);
        else
            if(e>a[m])
                l=m+1;
            else
                r=m-1;
    }
    return l;
}

// l is for left index and r is
// right index of the sub-array
// of arr to be sorted
void TBSort(int arr[],
            int l, int r)
{
    if (l < r)
    {
		// TREE step : sample and sort tree content
		int numElements = r - l + 1;
		int treeSize = pow(2,round(log2(log2(numElements))));
		int sampleTree[treeSize];
		for (int i = 0; i < treeSize; i++) sampleTree[i] = arr[ l + (rand() % (r - l + 1)) ]; // TODO: sample unique values
		std::sort(sampleTree, sampleTree+treeSize);
		printf("Search Tree: "); printArray(sampleTree, treeSize);

		// BIN step : use single pass with growable arrays (std::vector)
		int binCount = (int) numElements / log2(numElements);
		int targetbinSize = treeSize+2;
		if (binCount < targetbinSize) binCount = targetbinSize;
		std::vector <int> bin[binCount];

		int targetbin[targetbinSize];
		double slope[targetbinSize-1], offset[targetbinSize-1];
		for (int i = 1; i < targetbinSize-1; i++) {
			targetbin[i] = trunc(i*binCount/targetbinSize);
		}
		targetbin[0] = 0;
		targetbin[targetbinSize-1] = binCount-1;
		for (int i = 1; i < targetbinSize-1; i++) {
			slope[i] = (double) (targetbin[i+1]-targetbin[i]) / (sampleTree[i]-sampleTree[i-1]);
		}
		slope [0] = slope [1];
		slope [targetbinSize-2] = slope [targetbinSize-3];
		for (int i = 1; i < targetbinSize-1; i++) {
			offset[i]= (double) targetbin[i] - sampleTree[i-1] * slope[i];
		}
		offset[0] = targetbin[1] - sampleTree[0] / slope[0];

		int mypos, mybin;
		for (int element = 0; element < numElements; element++) {
			mypos = search(&sampleTree[0], treeSize, arr[element]); // tree search
			mybin = myclamp((int) (arr[element] * slope[mypos] + offset[mypos]), 0, binCount-1);
			bin[mybin].push_back(arr[element]);
		}

		// SORT step
		int binThreshold = (int) 5 * numElements / binCount;
		int curpos = 0;
		for (int bins = 0; bins < binCount; bins++) {
			if (bin[bins].empty()) continue;
			if (bin[bins].size() < binThreshold)
				std::stable_sort(bin[bins].begin(), bin[bins].end());
			else
				TBSort(&bin[bins][0], 0, bin[bins].size()-1);
			memcpy(&arr[curpos], &bin[bins][0], bin[bins].size() * sizeof(int) );
			curpos += bin[bins].size();
		}
    }
}

int main()
{
    int arr[] = {12, 11, 13, 5, 6, 7, 1, 4, 17, 3, 5, 20, 22, 2};
    int arr_size = sizeof(arr) / sizeof(arr[0]);

    printf("Given array is ");
    printArray(arr, arr_size);

    TBSort(arr, 0, arr_size - 1);

    printf("\nTBSorted array is ");
    printArray(arr, arr_size);
    return 0;
}
