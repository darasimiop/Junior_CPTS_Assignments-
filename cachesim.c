#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>

// Constants
#define ADDRESS_LENGTH 64  // 64-bit memory addressing

// Function prototypes
void print_summary(int hits, int misses, int evictions);
void print_usage(char* argv[]);
void init_cache(int s, int E, int b);
void access_cache(unsigned long address, char type);

// Cache data structures
typedef struct {
    int valid;        // Is the line valid?
    int tag;          // The tag of the cached block
    int lru_counter;  // For LRU replacement policy
} CacheLine;

typedef struct {
    CacheLine *lines; // Array of lines (E lines per set)
} CacheSet;

typedef struct {
    CacheSet *sets;   // Array of sets (2^s sets)
    int s, E, b;      // Cache parameters
} Cache;

Cache cache;
int hit_count = 0;
int miss_count = 0;
int eviction_count = 0;

// Initialize the cache with given parameters
void init_cache(int s, int E, int b) {
    int S = 1 << s;  // Number of sets (S = 2^s)

    cache.s = s;
    cache.E = E;
    cache.b = b;

    // Allocate memory for sets and lines
    cache.sets = malloc(S * sizeof(CacheSet));
    for (int i = 0; i < S; i++) {
        cache.sets[i].lines = malloc(E * sizeof(CacheLine));
        for (int j = 0; j < E; j++) {
            cache.sets[i].lines[j].valid = 0; // Initialize to invalid
            cache.sets[i].lines[j].lru_counter = 0;
        }
    }
}

// Function to print the cache summary statistics
void print_summary(int hits, int misses, int evictions) {
    printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);
}

// Function to print usage information
void print_usage(char* argv[]) {
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/trace01.dat\n", argv[0]);
    printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/trace01.dat\n", argv[0]);
    exit(0);
}

// Simulate cache access and update statistics
void access_cache(unsigned long address, char type) {
    unsigned long set_index = (address >> cache.b) & ((1 << cache.s) - 1);
    unsigned long tag = address >> (cache.s + cache.b);

    // Access the cache set
    CacheSet *set = &cache.sets[set_index];
    int hit = 0;
    int empty_line = -1; // Store index of an empty line for a potential miss
    int lru_line = 0;    // Index of the line to evict (LRU)

    // Iterate over cache lines to check for hits
    for (int i = 0; i < cache.E; i++) {
        CacheLine *line = &set->lines[i];
        if (line->valid && line->tag == tag) {
            hit = 1;
            line->lru_counter = 0;  // Reset LRU counter for a hit
            hit_count++;
            break;
        }

        // Track the first empty line for potential use
        if (!line->valid && empty_line == -1) {
            empty_line = i;
        }

        // Track the line with the highest LRU counter for eviction
        if (set->lines[lru_line].lru_counter < line->lru_counter) {
            lru_line = i;
        }
    }

    // If miss, handle miss and eviction
    if (!hit) {
        miss_count++;
        if (empty_line != -1) {
            // Miss without eviction (use empty line)
            set->lines[empty_line].valid = 1;
            set->lines[empty_line].tag = tag;
            set->lines[empty_line].lru_counter = 0;  // Reset LRU counter for new line
        } else {
            // Miss with eviction
            eviction_count++;
            set->lines[lru_line].valid = 1;         // Set the line as valid
            set->lines[lru_line].tag = tag;         // Replace LRU line's tag
            set->lines[lru_line].lru_counter = 0;   // Reset LRU counter for evicted line
        }
    }

    // Update LRU counters for all lines in the set
    for (int i = 0; i < cache.E; i++) {
        if (!hit || i != lru_line) {
            set->lines[i].lru_counter++; // Increment LRU counters for all except the current hit/evict line
        }
    }
}

int main(int argc, char* argv[]) {
    int opt, s = 0, E = 0, b = 0;
    char *tracefile = NULL;
    int verbose = 0;

    // Parse command-line arguments
    while ((opt = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
        switch (opt) {
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                tracefile = optarg;
                break;
            case 'v':
                verbose = 1;
                break;
            case 'h':
            default:
                print_usage(argv);
                exit(EXIT_FAILURE);
        }
    }

    // If any required argument is missing, show usage and exit
    if (s == 0 || E == 0 || b == 0 || tracefile == NULL) {
        print_usage(argv);
        exit(EXIT_FAILURE);
    }

    // Initialize cache
    init_cache(s, E, b);

    // Debug: Print the trace file path being used
    printf("Opening trace file: %s\n", tracefile);

    // Open trace file for reading
    FILE *trace_fp = fopen(tracefile, "r");
    if (!trace_fp) {
        fprintf(stderr, "Error opening trace file: %s\n", tracefile);
        exit(EXIT_FAILURE);
    }

    // Read and process each trace line
    char operation;
    unsigned long address;
    int size;
    while (fscanf(trace_fp, " %c %lx,%d", &operation, &address, &size) == 3) {
        if (operation != 'I') { // Ignore instruction loads
            if (operation == 'M') {
                access_cache(address, 'L');  // First access for modify (load)
                access_cache(address, 'S');  // Second access for modify (store)
            } else {
                access_cache(address, operation);
            }

            if (verbose) {
                printf("%c %lx,%d %s\n", operation, address, size, hit_count > miss_count ? "hit" : "miss");
            }
        }
    }

    // Output cache hit, miss, and eviction statistics
    print_summary(hit_count, miss_count, eviction_count);

    // Free dynamically allocated memory
    int S = 1 << s;
    for (int i = 0; i < S; i++) {
        free(cache.sets[i].lines);
    }
    free(cache.sets);

    // Close the trace file
    fclose(trace_fp);

    return 0;
}
