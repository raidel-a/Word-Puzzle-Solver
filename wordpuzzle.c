#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_WORD_LEN 100
#define MAX_DICT_WORDS 100000
#define MAX_THREADS 100
#define BUFFER_SIZE 2000000L

// Global variables
char* buffer;
char* dict[MAX_DICT_WORDS];
int dictionary_size = 0;
int* word_lengths = NULL;
int num_lengths = 0;
int num_threads = 1;
int verbose = 0;
int time_flag = 0;
int sorted_flag = 0;
int distinct_flag = 0;
pthread_mutex_t time_mutex = PTHREAD_MUTEX_INITIALIZER;
long total_search_time = 0;

// Structure to store found words
typedef struct {
    char** words;
    int count;
    int capacity;
} WordList;

WordList found_words = {NULL, 0, 0};
pthread_mutex_t words_mutex = PTHREAD_MUTEX_INITIALIZER;

// Thread argument structure
typedef struct {
    int thread_id;
    long start_pos;
    long end_pos;
} ThreadArg;

// Function to get current time in nanoseconds
static long get_nanos(void) {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (long)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

// Binary search implementation
int binsearch(char* dictionary_words[], int list_size, char* keyword) {
    long start_time = 0;
    if (time_flag) start_time = get_nanos();
    
    int mid, low = 0, high = list_size - 1;
    while (high >= low) {
        mid = low + (high - low) / 2;
        if (strcmp(dictionary_words[mid], keyword) < 0)
            low = mid + 1;
        else if (strcmp(dictionary_words[mid], keyword) > 0)
            high = mid - 1;
        else {
            if (time_flag) {
                long search_time = get_nanos() - start_time;
                pthread_mutex_lock(&time_mutex);
                total_search_time += search_time;
                pthread_mutex_unlock(&time_mutex);
            }
            return mid;
        }
    }
    
    if (time_flag) {
        long search_time = get_nanos() - start_time;
        pthread_mutex_lock(&time_mutex);
        total_search_time += search_time;
        pthread_mutex_unlock(&time_mutex);
    }
    return -1;
}

// Add word to found words list
void add_word(const char* word) {
    pthread_mutex_lock(&words_mutex);
    
    if (found_words.count >= found_words.capacity) {
        int new_capacity = (found_words.capacity == 0) ? 1000 : found_words.capacity * 2;
        char** new_words = realloc(found_words.words, new_capacity * sizeof(char*));
        if (new_words == NULL) {
            pthread_mutex_unlock(&words_mutex);
            return;
        }
        found_words.words = new_words;
        found_words.capacity = new_capacity;
    }
    
    found_words.words[found_words.count] = strdup(word);
    found_words.count++;
    
    pthread_mutex_unlock(&words_mutex);
}

// Thread worker function
void* word_puzzle_solver(void* arg) {
    ThreadArg* thread_arg = (ThreadArg*)arg;
    char* temp_word = malloc(MAX_WORD_LEN);
    
    if (verbose) {
        fprintf(stderr, "Thread #%d started!\n", thread_arg->thread_id);
    }
    
    for (int len_idx = 0; len_idx < num_lengths; len_idx++) {
        int curr_len = word_lengths[len_idx];
        
        for (long i = thread_arg->start_pos; i <= thread_arg->end_pos - curr_len + 1; i++) {
            strncpy(temp_word, buffer + i, curr_len);
            temp_word[curr_len] = '\0';
            
            if (binsearch(dict, dictionary_size, temp_word) >= 0) {
                add_word(temp_word);
            }
        }
    }
    
    free(temp_word);
    return NULL;
}

// Compare function for qsort
int compare_strings(const void* a, const void* b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

// Process command line arguments
void process_args(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-len") == 0 && i + 1 < argc) {
            char* token = strtok(argv[++i], ",");
            word_lengths = malloc(100 * sizeof(int));
            while (token != NULL) {
                word_lengths[num_lengths++] = atoi(token);
                token = strtok(NULL, ",");
            }
        } else if (strcmp(argv[i], "-nthreads") == 0 && i + 1 < argc) {
            num_threads = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-verbose") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "-time") == 0) {
            time_flag = 1;
        } else if (strcmp(argv[i], "-sorted") == 0) {
            sorted_flag = 1;
        } else if (strcmp(argv[i], "-distinct") == 0) {
            distinct_flag = 1;
        }
    }
    
    if (word_lengths == NULL) {
        word_lengths = malloc(2 * sizeof(int));
        word_lengths[0] = 8;
        word_lengths[1] = 9;
        num_lengths = 2;
    }
}

int main(int argc, char** argv) {
    long start_time = get_nanos();
    
    // Process command line arguments
    process_args(argc, argv);
    
    // Read dictionary
    FILE* f = fopen("dict.txt", "r");
    if (f == NULL) {
        fprintf(stderr, "Error: Cannot open dictionary file\n");
        return 1;
    }
    
    char line[1000];
    while (fgets(line, sizeof(line), f)) {
        char* newline = strchr(line, '\n');
        if (newline) *newline = '\0';
        
        if (strlen(line) == 0) continue;
        
        dict[dictionary_size] = strdup(line);
        dictionary_size++;
    }
    fclose(f);
    
    // Read input buffer
    buffer = malloc(BUFFER_SIZE + 1);
    if (fread(buffer, 1, BUFFER_SIZE, stdin) != BUFFER_SIZE) {
        fprintf(stderr, "Error: Cannot read input stream\n");
        return 1;
    }
    buffer[BUFFER_SIZE] = '\0';
    
    // Create threads
    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    ThreadArg* thread_args = malloc(num_threads * sizeof(ThreadArg));
    long chunk_size = BUFFER_SIZE / num_threads;
    
    for (int i = 0; i < num_threads; i++) {
        thread_args[i].thread_id = i;
        thread_args[i].start_pos = i * chunk_size;
        thread_args[i].end_pos = (i == num_threads - 1) ? BUFFER_SIZE - 1 : (i + 1) * chunk_size - 1;
        
        if (pthread_create(&threads[i], NULL, word_puzzle_solver, &thread_args[i]) != 0) {
            fprintf(stderr, "Error: Failed to create thread %d\n", i);
            return 1;
        }
    }
    
    // Join threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        if (verbose) {
            fprintf(stderr, "Thread #%d joined!\n", i);
        }
    }
    
    // Process results
    if (sorted_flag) {
        qsort(found_words.words, found_words.count, sizeof(char*), compare_strings);
    }
    
    if (distinct_flag) {
        int write_pos = 1;
        for (int read_pos = 1; read_pos < found_words.count; read_pos++) {
            if (strcmp(found_words.words[read_pos], found_words.words[write_pos-1]) != 0) {
                found_words.words[write_pos++] = found_words.words[read_pos];
            }
        }
        found_words.count = (found_words.count > 0) ? write_pos : 0;
    }
    
    // Print results
    for (int i = 0; i < found_words.count; i++) {
        printf("%s\n", found_words.words[i]);
    }
    
    // Print timing information
    if (verbose) {
        fprintf(stderr, "Total run time: %.2f seconds\n", 
                (get_nanos() - start_time) / 1000000000.0);
    }
    
    if (time_flag) {
        fprintf(stderr, "Total binary search time: %.2f seconds\n", 
                total_search_time / 1000000000.0);
    }
    
    // Cleanup
    free(buffer);
    free(threads);
    free(thread_args);
    free(word_lengths);
    for (int i = 0; i < found_words.count; i++) {
        free(found_words.words[i]);
    }
    free(found_words.words);
    for (int i = 0; i < dictionary_size; i++) {
        free(dict[i]);
    }
    
    return 0;
}
