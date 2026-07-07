#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "allocator.h"

// ── timer helper ─────────────────────────────────────────────────────────────
static double now_ms() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000.0 + t.tv_nsec / 1e6;
}

// ── allocation patterns ───────────────────────────────────────────────────────

// Pattern 1: bulk — alloc everything, then free everything
// simulates a program that builds a big data structure then tears it down
void pattern_bulk(int n) {
    void **ptrs = malloc(n * sizeof(void*));
    for (int i = 0; i < n; i++)
        ptrs[i] = my_malloc(64);
    for (int i = 0; i < n; i++)
        my_free(ptrs[i]);
    free(ptrs);
}

// Pattern 2: interleaved — alloc and free alternately
// simulates a server handling short-lived requests
void pattern_interleaved(int n) {
    void **ptrs = malloc(n * sizeof(void*));
    // alloc half
    for (int i = 0; i < n / 2; i++)
        ptrs[i] = my_malloc(64);
    // now interleave frees and allocs
    for (int i = 0; i < n / 2; i++) {
        my_free(ptrs[i]);
        ptrs[i] = my_malloc(64);
    }
    // cleanup
    for (int i = 0; i < n / 2; i++)
        my_free(ptrs[i]);
    free(ptrs);
}

// Pattern 3: random sizes — alloc random sizes between 8 and 512 bytes
// simulates real-world mixed allocation patterns
void pattern_random_sizes(int n) {
    void **ptrs = malloc(n * sizeof(void*));
    srand(42);  // fixed seed for reproducibility
    for (int i = 0; i < n; i++)
        ptrs[i] = my_malloc((rand() % 505) + 8);  // 8..512 bytes
    // free in reverse order (stresses coalescing)
    for (int i = n - 1; i >= 0; i--)
        my_free(ptrs[i]);
    free(ptrs);
}

// ── benchmark runner ──────────────────────────────────────────────────────────
typedef void (*PatternFn)(int);

void run_benchmark(const char *strategy, const char *pattern_name,
                   PatternFn fn, int n) {
    double start = now_ms();
    fn(n);
    double elapsed = now_ms() - start;

    double ops_per_sec = (n * 2.0) / (elapsed / 1000.0); // *2 for alloc+free
    double frag = fragmentation_ratio();

    printf("%-12s | %-18s | %8.2f ms | %10.0f ops/s | frag: %.4f\n",
           strategy, pattern_name, elapsed, ops_per_sec, frag);
}

FILE *csv;
void run_benchmark_csv(const char *strategy, const char *pattern,
                       PatternFn fn, int n) {
    double start = now_ms();
    fn(n);
    double elapsed = now_ms() - start;
    double ops     = (n * 2.0) / (elapsed / 1000.0);
    double frag    = fragmentation_ratio();

    // print to terminal
    printf("%-12s | %-18s | %8.2f ms | %10.0f ops/s | frag: %.4f\n",
           strategy, pattern, elapsed, ops, frag);

    // write to CSV
    fprintf(csv, "%s,%s,%.2f,%.0f,%.4f\n",
            strategy, pattern, elapsed, ops, frag);
}

// ── main ──────────────────────────────────────────────────────────────────────
int main() {
    int N = 10000;  // operations per test

    printf("\n");
    printf("run_benchmark: \n");
    printf("%-12s | %-18s | %10s | %14s | %s\n",
           "Strategy", "Pattern", "Time", "Throughput", "Fragmentation");
    printf("-------------|--------------------|-----------"
           "|----------------|-------------\n");

    // First-fit
    set_strategy(STRATEGY_FIRST_FIT);
    run_benchmark("first-fit", "bulk",         pattern_bulk,         N);
    run_benchmark("first-fit", "interleaved",  pattern_interleaved,  N);
    run_benchmark("first-fit", "random-sizes", pattern_random_sizes, N);
        
    // Best-fit
    set_strategy(STRATEGY_BEST_FIT);
    run_benchmark("best-fit",  "bulk",         pattern_bulk,         N);
    run_benchmark("best-fit",  "interleaved",  pattern_interleaved,  N);
    run_benchmark("best-fit",  "random-sizes", pattern_random_sizes, N);
        
    // Buddy
    set_strategy(STRATEGY_BUDDY);
    run_benchmark("buddy",     "bulk",         pattern_bulk,         N);
    run_benchmark("buddy",     "interleaved",  pattern_interleaved,  N);
    run_benchmark("buddy",     "random-sizes", pattern_random_sizes, N);

    
    //
    // --------------------------------------------------------------------------
    //
    printf("\n");
    printf("run_benchmark_csv: \n");
    csv = fopen("results.csv", "w");
    fprintf(csv, "strategy,pattern,time_ms,ops_per_sec,fragmentation\n");
    printf("%-12s | %-18s | %10s | %14s | %s\n",
           "Strategy", "Pattern", "Time", "Throughput", "Fragmentation");
    printf("-------------|--------------------|-----------"
           "|----------------|-------------\n");

    // First-fit
    set_strategy(STRATEGY_FIRST_FIT);
    run_benchmark_csv("first-fit", "bulk",         pattern_bulk,         N);
    run_benchmark_csv("first-fit", "interleaved",  pattern_interleaved,  N);
    run_benchmark_csv("first-fit", "random-sizes", pattern_random_sizes, N);
        
    // Best-fit
    set_strategy(STRATEGY_BEST_FIT);
    run_benchmark_csv("best-fit",  "bulk",         pattern_bulk,         N);
    run_benchmark_csv("best-fit",  "interleaved",  pattern_interleaved,  N);
    run_benchmark_csv("best-fit",  "random-sizes", pattern_random_sizes, N);
        
    // Buddy
    set_strategy(STRATEGY_BUDDY);
    run_benchmark_csv("buddy",     "bulk",         pattern_bulk,         N);
    run_benchmark_csv("buddy",     "interleaved",  pattern_interleaved,  N);
    run_benchmark_csv("buddy",     "random-sizes", pattern_random_sizes, N);

    fclose(csv);
    return 0;
}


// to compile:
// gcc -Wall -Wextra -pthread -O2 -o benchmark allocator.c buddy.c benchmark.c && ./benchmark