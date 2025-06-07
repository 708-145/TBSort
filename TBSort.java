/* TBSort.java - A general O(n * log(log(n)) sort
Copyright (C) 2024 Tobias Bergmann

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>. */

import java.util.Arrays;
import java.util.Random;

public class TBSort {

    // UTILITY FUNCTIONS
    public static int myclamp(int n, int lower, int upper) {
        return n <= lower ? lower : n >= upper ? upper : n;
    }

    // Function to print an array
    public static void printArray(int[] A) {
        for (int i = 0; i < A.length; i++)
            System.out.print(A[i] + " ");
        System.out.println();
    }

    // Binary search in array a for element e
    // Returns position of element in array that is <= e
    public static int search(int[] a, int e) {
        int l = 0;
        int r = a.length - 1;

        while (l <= r) {
            int m = (l + r) / 2;
            if (e == a[m])
                return m;
            else if (e > a[m])
                l = m + 1;
            else
                r = m - 1;
        }
        return l;
    }

    // l is for left index and r is
    // right index of the sub-array
    // of arr to be sorted
    public static void TBSort(int[] arr, int l, int r) {
        if (l < r) {
            // TREE step : sample and sort tree content
            int numElements = r - l + 1;
            int treeSize = (int) Math.pow(2, Math.round(Math.log(Math.log(numElements) / Math.log(2)) / Math.log(2)));
            int[] sampleTree = new int[treeSize];
            Random rand = new Random();
            for (int i = 0; i < treeSize; i++)
                sampleTree[i] = arr[l + rand.nextInt(r - l + 1)];
            Arrays.sort(sampleTree);
            System.out.print("Search Tree: ");
            printArray(sampleTree);

            // BIN step : use single pass with growable arrays
            int binCount = (int) numElements / (int) (Math.log(numElements) / Math.log(2));
            int targetbinSize = treeSize + 2;
            if (binCount < targetbinSize)
                binCount = targetbinSize;
            int[][] bin = new int[binCount][];

            int[] targetbin = new int[targetbinSize];
            double[] slope = new double[targetbinSize - 1];
            double[] offset = new double[targetbinSize - 1];
            for (int i = 1; i < targetbinSize - 1; i++) {
                targetbin[i] = (int) (i * binCount / (double) targetbinSize);
            }
            targetbin[0] = 0;
            targetbin[targetbinSize - 1] = binCount - 1;
            for (int i = 1; i < targetbinSize - 2; i++) {
                slope[i] = (double) (targetbin[i + 1] - targetbin[i]) / (double) (sampleTree[i] - sampleTree[i - 1]);
            }
            slope[0] = slope[1];
            slope[targetbinSize - 2] = slope[targetbinSize - 3];
            for (int i = 1; i < targetbinSize - 1; i++) {
                offset[i] = (double) targetbin[i] - (double) sampleTree[i - 1] * slope[i];
            }
            offset[0] = targetbin[1] - (double) sampleTree[0] / slope[0];

            int mypos, mybin;
            int[] binSize = new int[binCount];
            for (int element = 0; element < numElements; element++) {
                mypos = search(sampleTree, arr[element]); // tree search
                mybin = myclamp((int) (arr[element] * slope[mypos] + offset[mypos]), 0, binCount - 1);
                binSize[mybin]++;
            }

            for (int i = 0; i < binCount; i++) {
                bin[i] = new int[binSize[i]];
            }

            int[] binIndex = new int[binCount];
            for (int element = 0; element < numElements; element++) {
                mypos = search(sampleTree, arr[element]); // tree search
                mybin = myclamp((int) (arr[element] * slope[mypos] + offset[mypos]), 0, binCount - 1);
                bin[mybin][binIndex[mybin]++] = arr[element];
            }

            // SORT step
            int binThreshold = (int) 5 * numElements / binCount;
            int curpos = l; // Start from the original array's left index
            for (int bins = 0; bins < binCount; bins++) {
                if (bin[bins].length == 0)
                    continue;
                if (bin[bins].length < binThreshold)
                    Arrays.sort(bin[bins]);
                else
                    TBSort(bin[bins], 0, bin[bins].length - 1);
                System.arraycopy(bin[bins], 0, arr, curpos, bin[bins].length);
                curpos += bin[bins].length;
            }
        }
    }

    public static void main(String[] args) {
        int[] arr = {12, 11, 13, 5, 6, 7, 1, 4, 17, 3, 5, 20, 22, 2};
        System.out.print("Given array is ");
        printArray(arr);

        TBSort(arr, 0, arr.length - 1);

        System.out.print("\nTBSorted array is ");
        printArray(arr);
    }
}
