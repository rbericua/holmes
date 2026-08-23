#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "geometry.h"
#include "grid.h"
#include "solver.h"
#include "step.h"
#include "techniques/registry.h"

#define RESULT_LINE_LEN 60

typedef struct {
    int total_sudokus;
    int solved_sudokus;
    int total_steps;
    int steps_by_technique[NUM_TECHNIQUES];
    struct timespec start_time;
    struct timespec end_time;
} BenchmarkStats;

static double get_time_diff_sec(struct timespec a, struct timespec b);
static double get_percentage(double part, double total);
static void print_justified(char *left, char *right, int width, char filler);
static void print_title(char *label, char filler);
static void print_stat(char *label, char *format, ...)
    __attribute__((format(printf, 2, 3)));

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: benchmark <dataset> [limit]\n");
        return 1;
    }

    int limit = argc == 3 ? atoi(argv[2]) : INT_MAX;

    geometry_init();

    FILE *f = fopen(argv[1], "r");
    if (!f) {
        fprintf(stderr, "Failed to open %s\n", argv[1]);
        return 1;
    }

    BenchmarkStats stats = {0};
    timespec_get(&stats.start_time, TIME_UTC);

    char line[128];
    int i = 0;
    while (i < limit && fgets(line, sizeof(line), f)) {
        Grid *grid = grid_create(line);
        stats.total_sudokus++;

        SolveStatus status;
        while (true) {
            Step step;
            status = solver_next_step(grid, &step);
            if (status != SOLVE_ONGOING) break;

            solver_apply_step(grid, &step);

            stats.total_steps++;
            stats.steps_by_technique[step.type]++;
        }

        if (status == SOLVE_COMPLETE) {
            stats.solved_sudokus++;
        }

        grid_destroy(grid);
        i++;
    }
    timespec_get(&stats.end_time, TIME_UTC);

    double solved_percentage = get_percentage(stats.solved_sudokus,
                                              stats.total_sudokus);
    double time_elapsed_ms = get_time_diff_sec(stats.end_time, stats.start_time)
                             * 1000;
    int sudokus_per_second = stats.total_sudokus / (time_elapsed_ms / 1000);

    print_title("BENCHMARK RESULTS", '=');
    printf("\n");
    print_stat("Total Sudokus", "%d", stats.total_sudokus);
    print_stat("Sudokus Solved", "%d (%.2f%%)", stats.solved_sudokus,
               solved_percentage);
    print_stat("Total Time", "%.2f ms", time_elapsed_ms);
    print_stat("Mean Time per Sudoku", "%.2f ms",
               time_elapsed_ms / stats.total_steps);
    print_stat("Sudokus per Second", "%d", sudokus_per_second);
    print_stat("Total Steps", "%d", stats.total_steps);
    printf("\n");
    print_title("Technique Distribution", '-');
    printf("\n");
    for (int i = 0; i < NUM_TECHNIQUES; i++) {
        double step_percentage = get_percentage(stats.steps_by_technique[i],
                                                stats.total_steps);
        print_stat(technique_names[i], "%d (%.2f%%)",
                   stats.steps_by_technique[i], step_percentage);
    }

    return 0;
}

static double get_time_diff_sec(struct timespec a, struct timespec b) {
    return (a.tv_sec - b.tv_sec) + (a.tv_nsec - b.tv_nsec) / 1e9;
}

static double get_percentage(double part, double total) {
    return (part / total) * 100;
}

static void print_justified(char *left, char *right, int width, char filler) {
    int left_len = strlen(left);
    int right_len = strlen(right);
    int filler_len = width - left_len - right_len - 2;

    printf("%s ", left);
    for (int i = 0; i < filler_len; i++) {
        printf("%c", filler);
    }
    printf(" %s\n", right);
}

static void print_title(char *label, char filler) {
    int label_len = strlen(label);
    int filler_len = (RESULT_LINE_LEN - label_len - 2) / 2;

    for (int i = 0; i < filler_len; i++) {
        printf("%c", filler);
    }
    printf(" %s ", label);
    for (int i = 0; i < filler_len; i++) {
        printf("%c", filler);
    }
    printf("\n");
}

static void print_stat(char *label, char *format, ...) {
    va_list args;
    va_start(args);

    char buf[32];
    vsprintf(buf, format, args);
    print_justified(label, buf, RESULT_LINE_LEN, '.');

    va_end(args);
}
