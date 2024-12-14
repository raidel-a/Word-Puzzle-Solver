import matplotlib.pyplot as plt
import numpy as np

# Read the timing data manually
threads = []
runtimes = []

with open('test_results/timing_data.csv', 'r') as f:
    next(f)  # Skip header line
    for line in f:
        thread, runtime = line.strip().split(',')
        threads.append(int(thread))
        runtimes.append(float(runtime))

# Convert to numpy arrays for calculations
threads = np.array(threads)
runtimes = np.array(runtimes)

# Create the plot
plt.figure(figsize=(10, 6))
plt.scatter(threads, runtimes, color='blue', label='Actual runtime')

# Calculate and plot regression line
z = np.polyfit(threads, runtimes, 1)
p = np.poly1d(z)
plt.plot(threads, p(threads), "r--", label='Regression line')

plt.xlabel('Number of Threads')
plt.ylabel('Runtime (seconds)')
plt.title('Word Puzzle Performance vs Number of Threads')
plt.legend()
plt.grid(True)

# Save the plot
plt.savefig('test_results/performance_plot.jpg')
plt.close() 