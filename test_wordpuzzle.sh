#!/bin/bash

# Create test output directory if it doesn't exist
mkdir -p test_results

# Generate the 2MB random input once and save it
./randomgen 2000000 > test_input.txt

# Function to run test and save output
run_test() {
    local args="$1"
    local outfile="$2"
    echo "Running test with args: $args"
    cat test_input.txt | ./wordpuzzle $args > "test_results/$outfile" 2> "test_results/${outfile%.txt}_stderr.txt"
}

# Basic tests for different lengths
run_test "-len 5" "len_5_basic.txt"
run_test "-len 6" "len_6_basic.txt"
run_test "-len 5,6" "len_5_6_basic.txt"

# Tests with sorting
run_test "-len 5 -sorted" "len_5_sorted.txt"
run_test "-len 6 -sorted" "len_6_sorted.txt"
run_test "-len 5,6 -sorted" "len_5_6_sorted.txt"

# Tests with sorting and distinct
run_test "-len 5 -sorted -distinct" "len_5_sorted_distinct.txt"
run_test "-len 6 -sorted -distinct" "len_6_sorted_distinct.txt"
run_test "-len 5,6 -sorted -distinct" "len_5_6_sorted_distinct.txt"

# Tests with different thread counts
for threads in 1 2 4 8 16 32 64 90; do
    run_test "-nthreads $threads" "threads_${threads}.txt"
done

# Tests with timing and verbose options
run_test "-len 5 -time -verbose" "len_5_time_verbose.txt"
run_test "-len 6 -time -verbose" "len_6_time_verbose.txt"

# Comprehensive tests
run_test "-len 5,6 -sorted -distinct -time -verbose" "comprehensive_5_6.txt"
run_test "-len 8,9 -sorted -distinct -time -verbose -nthreads 4" "comprehensive_8_9.txt"

# Generate timing data for plotting
echo "threads,runtime" > test_results/timing_data.csv
for threads in 1 2 4 8 16 32 64 90; do
    echo "Testing with $threads threads for timing data..."
    runtime=$(cat test_input.txt | ./wordpuzzle -nthreads $threads -time -verbose 2>&1 | grep "Total run time:" | sed 's/Total run time: \([0-9.]*\) seconds/\1/')
    if [ ! -z "$runtime" ]; then
        echo "$threads,$runtime" >> test_results/timing_data.csv
    else
        echo "Failed to get runtime for $threads threads"
    fi
done

echo "All tests completed. Results are in the test_results directory."
echo "Timing data for plotting is in test_results/timing_data.csv" 