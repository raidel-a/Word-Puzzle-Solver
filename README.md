# Word Puzzle Solver

A multi-threaded C program that finds meaningful English words in a randomly generated string of characters.

## Overview

This program processes a 2-million-character random string to find valid English words of specified lengths. It uses binary search for word validation against a dictionary and supports multi-threading for improved performance.

## Features

- Multi-threaded word searching
- Configurable word lengths
- Optional sorting of results
- Duplicate word removal
- Performance timing
- Verbose execution mode

## Requirements

- GCC compiler
- POSIX threads support
- Python 3.x (for performance plotting)
- Required Python packages: matplotlib, numpy

## Building the Program

```bash
make clean    # Clean previous builds
make         # Compile the program
```

## Usage

Basic usage:
```bash
./randomgen 2000000 | ./wordpuzzle [options]
```

### Command Line Options

- `-len list`: Specify word lengths to search for (comma-separated)
  - Example: `-len 5,6,7`
  - Default: 8,9

- `-nthreads n`: Set number of puzzle-solving threads
  - Example: `-nthreads 4`
  - Default: 1

- `-verbose`: Enable detailed output
  - Shows thread creation/joining
  - Reports total runtime

- `-time`: Show binary search timing statistics

- `-sorted`: Sort the output words alphabetically

- `-distinct`: Remove duplicate words from output

### Examples

1. Search for 5-letter words using 4 threads:
```bash
./randomgen 2000000 | ./wordpuzzle -len 5 -nthreads 4
```

2. Find 6-letter words, sorted and without duplicates:
```bash
./randomgen 2000000 | ./wordpuzzle -len 6 -sorted -distinct
```

3. Comprehensive search with timing:
```bash
./randomgen 2000000 | ./wordpuzzle -len 5,6 -sorted -distinct -time -verbose -nthreads 4
```

## Testing

The repository includes a test suite that verifies all functionality:

```bash
# Run all tests
bash test_wordpuzzle.sh

# Generate performance plot
source venv/bin/activate  # Activate Python virtual environment
python plot_timing.py     # Generate thread performance plot
```

The test suite:
- Runs various combinations of command-line options
- Tests different thread counts
- Generates timing data
- Creates a performance plot (saved as `test_results/performance_plot.jpg`)

## Performance Analysis

Optimal performance is typically achieved with 4-8 threads. Adding more threads may not improve performance due to:
- Thread creation overhead
- Memory contention
- System resource limitations

The performance plot (`test_results/performance_plot.jpg`) shows the relationship between thread count and execution time.

## Files Description

- `wordpuzzle.c`: Main program implementation
- `randomgen.c`: Random string generator
- `dict.txt`: Dictionary file for word validation
- `Makefile`: Build configuration
- `test_wordpuzzle.sh`: Automated test suite
- `plot_timing.py`: Performance visualization script

## Notes

- The program uses standard error for timing and verbose output
- Word matching is case-sensitive
- Dictionary lookups use binary search for efficiency
- Thread count should be adjusted based on your system's capabilities 