use std::vec::Vec;
use rand::Rng; // Import for generating random numbers

// UTILITY FUNCTIONS
fn myclamp<T: PartialOrd + Copy>(n: T, lower: T, upper: T) -> T {
    if n <= lower {
        lower
    } else if n >= upper {
        upper
    } else {
        n
    }
}

// Function to print an array
fn print_array(a: &[i32]) {
    for i in 0..a.len() {
        print!("{} ", a[i]);
    }
    println!();
}

// Binary search in array a of size n for element e
// Returns position of element in array that is <= e
fn search(a: &[i32], n: usize, e: i32) -> usize {
    let mut l = 0;
    let mut r = n - 1;

    while l <= r {
        let m = (l + r) / 2;
        if e == a[m] {
            return m;
        } else if e > a[m] {
            l = m + 1;
        } else {
            r = m - 1;
        }
    }
    l
}

// l is for left index and r is
// right index of the sub-array
// of arr to be sorted
fn tbsort(arr: &mut [i32], l: usize, r: usize) {
    if l < r {
        // Generate a random number generator
        let mut rng = rand::thread_rng(); 
        
        // TREE step : sample and sort tree content
        let num_elements = r - l + 1;
        let tree_size = 2usize.pow(round(log2(log2(num_elements as f64))) as u32);
        let mut sample_tree: Vec<i32> = Vec::with_capacity(tree_size);
        for _ in 0..tree_size {
            sample_tree.push(arr[l + (rng.gen::<usize>() as usize) % (r - l + 1)]);
        }
        sample_tree.sort_unstable();
        println!("Search Tree: ");
        print_array(&sample_tree);

        // BIN step : use single pass with growable arrays (std::vector)
        let bin_count = (num_elements as f64 / log2(num_elements as f64)) as usize;
        let target_bin_size = tree_size + 2;
        let bin_count = if bin_count < target_bin_size {
            target_bin_size
        } else {
            bin_count
        };
        let mut bin: Vec<Vec<i32>> = vec![Vec::new(); bin_count];

        let mut target_bin: Vec<usize> = vec![0; target_bin_size];
        let mut slope: Vec<f64> = vec![0.0; target_bin_size - 1];
        let mut offset: Vec<f64> = vec![0.0; target_bin_size - 1];
        for i in 1..target_bin_size - 1 {
            target_bin[i] = (i as f64 * bin_count as f64 / target_bin_size as f64).trunc() as usize;
        }
        target_bin[0] = 0;
        target_bin[target_bin_size - 1] = bin_count - 1;
        for i in 1..target_bin_size - 1 {
            slope[i] = (target_bin[i + 1] as f64 - target_bin[i] as f64)
                / (sample_tree[i] as f64 - sample_tree[i - 1] as f64);
        }
        slope[0] = slope[1];
        slope[target_bin_size - 2] = slope[target_bin_size - 3];
        for i in 1..target_bin_size - 1 {
            offset[i] = target_bin[i] as f64 - sample_tree[i - 1] as f64 * slope[i];
        }
        offset[0] = target_bin[1] as f64 - sample_tree[0] as f64 / slope[0];

        for element in 0..num_elements {
            let mypos = search(&sample_tree, tree_size, arr[l + element]);
            let mybin = myclamp(
                (arr[l + element] as f64 * slope[mypos] + offset[mypos]).trunc() as usize,
                0,
                bin_count - 1,
            );
            bin[mybin].push(arr[l + element]);
        }

        // SORT step
        let bin_threshold = (5.0 * num_elements as f64 / bin_count as f64).trunc() as usize;
        let mut curpos = l;
        for bins in 0..bin_count {
            if bin[bins].is_empty() {
                continue;
            }
            let len = bin[bins].len(); // Calculate length outside the mutable borrow:
            if len < bin_threshold {
                bin[bins].sort_unstable();
            } else {
                tbsort(&mut bin[bins], 0, len - 1); // Now, only the mutable borrow is needed:
            }
            let start = curpos;
            let end = start + bin[bins].len();
            arr[start..end].copy_from_slice(&bin[bins]);
            curpos = end;
        }
    }
}

fn main() {
    let mut arr = [12, 11, 13, 5, 6, 7, 1, 4, 17, 3, 5, 20, 22, 2];
    let arr_size = arr.len();

    println!("Given array is ");
    print_array(&arr);

    tbsort(&mut arr, 0, arr_size - 1);

    println!("\nTBSorted array is ");
    print_array(&arr);
}

fn log2(x: f64) -> f64 {
    (x / 2f64).log(2f64)
}

fn round(x: f64) -> u32 {
    (x + 0.5f64).trunc() as u32
}