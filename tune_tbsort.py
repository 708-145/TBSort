import random
import re
import subprocess
import shutil
import os
import atexit

# --- Configuration Variables ---
C_FILE_PATH = "tbsort_int64.c"
ORIGINAL_C_FILE_BACKUP_PATH = "tbsort_int64.c.original"
BENCH_FILE_PATH = "tbsort_bench.c"
EXECUTABLE_NAME = "tbsort_bench_executable"
# Updated COMPILATION_COMMAND to use -O0
COMPILATION_COMMAND = ["gcc", "-Wall", "-Wextra", "-O0", C_FILE_PATH, BENCH_FILE_PATH, "-o", EXECUTABLE_NAME, "-lm"]
BENCHMARK_EXEC_COMMAND_TEMPLATE = [f"./{EXECUTABLE_NAME}"] # Renamed to indicate it's a template
# Updated NUM_ITERATIONS to 2
NUM_ITERATIONS = 2
# Updated BENCHMARK_ARRAY_SIZE_N to 1000000
BENCHMARK_ARRAY_SIZE_N = 1000000

PARAMETERS_TO_TUNE = {
    "INSERTION_SORT_THRESHOLD": {"min": 2, "max": 64, "type": int},
    "LEAF_BUFFER_INITIAL_CAPACITY": {"values": [2, 4, 8, 16, 32, 64], "type": "choice"},
    "SMALL_LEAF_BUFFER_THRESHOLD": {"min": 16, "max": 512, "type": int},
    "LOCAL_BIN_INITIAL_CAPACITY": {"values": [2, 4, 8, 16, 32, 64], "type": "choice"},
}

# --- File Handling & Cleanup ---
original_c_content = None

def backup_original_c_file():
    global original_c_content
    if not os.path.exists(C_FILE_PATH):
        print(f"Error: C file '{C_FILE_PATH}' not found.")
        exit(1)
    try:
        shutil.copy(C_FILE_PATH, ORIGINAL_C_FILE_BACKUP_PATH)
        print(f"Backed up '{C_FILE_PATH}' to '{ORIGINAL_C_FILE_BACKUP_PATH}'")
        with open(C_FILE_PATH, 'r') as f:
            original_c_content = f.read()
    except Exception as e:
        print(f"Error backing up C file: {e}")
        exit(1)

def restore_original_c_file():
    if os.path.exists(ORIGINAL_C_FILE_BACKUP_PATH):
        try:
            shutil.copy(ORIGINAL_C_FILE_BACKUP_PATH, C_FILE_PATH)
            print(f"\nRestored '{C_FILE_PATH}' from backup.")
            os.remove(ORIGINAL_C_FILE_BACKUP_PATH)
            print(f"Removed backup file '{ORIGINAL_C_FILE_BACKUP_PATH}'.")
        except Exception as e:
            print(f"Error restoring C file: {e}")
    elif original_c_content:
        try:
            with open(C_FILE_PATH, 'w') as f:
                f.write(original_c_content)
            print(f"\nRestored '{C_FILE_PATH}' from in-memory content.")
        except Exception as e:
            print(f"Error restoring C file from memory: {e}")

atexit.register(restore_original_c_file)

# --- Core Functions ---
def generate_random_params(param_definitions):
    params = {}
    for name, definition in param_definitions.items():
        if definition["type"] == int:
            params[name] = random.randint(definition["min"], definition["max"])
        elif definition["type"] == "choice":
            params[name] = random.choice(definition["values"])
        else:
            print(f"Warning: Unknown parameter type for {name}: {definition['type']}")
    return params

def update_c_file(c_file_path, params):
    try:
        with open(c_file_path, 'r') as f:
            content = f.read()
        for param_name, param_value in params.items():
            pattern = re.compile(rf"(#define\s+{param_name}\s+)\d+")
            replacement = rf"\g<1>{param_value}"
            content, num_subs = pattern.subn(replacement, content)
            if num_subs == 0:
                print(f"Warning: Parameter '{param_name}' not found or not updated in C file.")
        with open(c_file_path, 'w') as f:
            f.write(content)
        return True
    except Exception as e:
        print(f"Error updating C file '{c_file_path}': {e}")
        return False

# Modified compile_and_benchmark function
def compile_and_benchmark(compilation_cmd, benchmark_cmd_template, benchmark_n):
    try:
        compile_result = subprocess.run(compilation_cmd, capture_output=True, text=True, check=False)
        if compile_result.returncode != 0:
            print(f"Compilation failed with error:\n{compile_result.stderr}")
            return float('inf')

        # Construct actual benchmark command
        actual_benchmark_cmd = list(benchmark_cmd_template) + [str(benchmark_n)]
        # print(f"Executing benchmark: {' '.join(actual_benchmark_cmd)}") # For debugging

        benchmark_result = subprocess.run(actual_benchmark_cmd, capture_output=True, text=True, check=False)
        if benchmark_result.returncode != 0:
            # Check if it's a known error from tbsort_bench.c for invalid N
            if "Error: Invalid N value" in benchmark_result.stderr or \
               "Error: N value" in benchmark_result.stderr:
                 print(f"Benchmark executable reported an error for N={benchmark_n}:\n{benchmark_result.stderr}")
            else:
                print(f"Benchmark execution failed with error (cmd: {' '.join(actual_benchmark_cmd)}):\n{benchmark_result.stderr}")
            return float('inf')

        output_lines = benchmark_result.stdout.splitlines()
        time_taken = float('inf')
        pattern = re.compile(rf"N={benchmark_n}.*TBSort_int64 time:\s*([0-9]+\.[0-9]+)\s*seconds")
        for line in output_lines:
            match = pattern.search(line)
            if match:
                time_taken = float(match.group(1))
                break

        if time_taken == float('inf'):
            print(f"Failed to parse benchmark time for N={benchmark_n} from output (cmd: {' '.join(actual_benchmark_cmd)}):\n{benchmark_result.stdout}")

        return time_taken
    except Exception as e:
        print(f"Error during compilation or benchmark: {e}")
        return float('inf')

# --- Main Loop ---
def main():
    backup_original_c_file()

    best_params = None
    best_time = float('inf')

    print(f"Starting random search for {NUM_ITERATIONS} iterations, N={BENCHMARK_ARRAY_SIZE_N}...\n")

    for i in range(NUM_ITERATIONS):
        current_params = generate_random_params(PARAMETERS_TO_TUNE)

        if not update_c_file(C_FILE_PATH, current_params):
            print(f"Iteration {i+1}/{NUM_ITERATIONS}: Failed to update C file. Skipping.")
            continue

        # Pass BENCHMARK_EXEC_COMMAND_TEMPLATE to compile_and_benchmark
        current_time = compile_and_benchmark(COMPILATION_COMMAND, BENCHMARK_EXEC_COMMAND_TEMPLATE, BENCHMARK_ARRAY_SIZE_N)

        print(f"Iteration {i+1}/{NUM_ITERATIONS}:")
        print(f"  Params: {current_params}")
        print(f"  Time: {current_time:.6f}s")

        if current_time < best_time:
            best_time = current_time
            best_params = current_params
            print(f"  New best time found!")

        print(f"  Best time so far: {best_time:.6f}s with params: {best_params}\n")

    print("\n--- Random Search Finished ---")
    if best_params:
        print(f"Best parameters found: {best_params}")
        print(f"Best time: {best_time:.6f} seconds")
    else:
        print("No successful runs completed or all runs resulted in errors/infinite time.")

if __name__ == "__main__":
    main()
