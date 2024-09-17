/**
 * File: main.c
 *
 * Main function for the Game of Life simulator.
 *
 * Author: Your Name
 *
 * Date: Today's Date
 */

#define _XOPEN_SOURCE 600

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <curses.h>
#include <pthread.h>
#include <errno.h>

#include "gol.h"
#include "barrier.h"  // Include the custom barrier

typedef struct {
    int *world;
    int *world_copy;
    int width;
    int height;
    int num_turns;
    int delay;
    bool step_mode;
    int start_row;
    int end_row;
    int thread_id;
    int num_threads;
    pthread_barrier_t *barrier;
} ThreadData;

/**
 * Prints usage information for the program.
 *
 * @param prog_name The name of the program.
 */
static void usage(char *prog_name) {
    fprintf(stderr, "usage: %s [-s] -c <config-file> -t <number of turns> -d <delay in ms> -p <number of threads>\n", prog_name);
    exit(1);
}

/**
 * Thread function to simulate the Game of Life world.
 *
 * @param arg Pointer to a ThreadData struct containing thread parameters.
 * @return NULL
 */
void *simulate_world(void *arg) {
    ThreadData *data = (ThreadData *)arg;

    int *world = data->world;
    int *world_copy = data->world_copy;
    int width = data->width;
    int height = data->height;
    int num_turns = data->num_turns;
    int delay = data->delay;
    bool step_mode = data->step_mode;
    int start_row = data->start_row;
    int end_row = data->end_row;
    int thread_id = data->thread_id;
    pthread_barrier_t *barrier = data->barrier;

    // Print thread partitioning info
    printf("\rThread %2d: rows %3d:%3d (%d)\n", thread_id, start_row, end_row - 1, end_row - start_row);

    for (int turn = 0; turn <= num_turns; turn++) {
        // Only one thread handles copying the world
        if (thread_id == 0) {
            memcpy(world_copy, world, width * height * sizeof(int));
        }

        // Barrier to ensure world_copy is ready
        int rc = pthread_barrier_wait(barrier);
        if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
            perror("pthread_barrier_wait");
            exit(1);
        }

        if (turn < num_turns) {
            // Each thread updates its portion of the world
            update_world(world, world_copy, width, height, start_row, end_row);
        }

        // Barrier to ensure all threads have updated before printing
        rc = pthread_barrier_wait(barrier);
        if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
            perror("pthread_barrier_wait");
            exit(1);
        }

        // Only one thread handles printing and delay/step
        if (thread_id == 0) {
            clear(); // Clear the screen

            // Print the world
            print_world(world, width, height, turn);

            if (step_mode) {
                getch(); // wait for user input
            } else {
                usleep(delay * 1000); // delay is in milliseconds, usleep takes microseconds
            }
        }

        // Barrier to ensure all threads have printed before next turn
        rc = pthread_barrier_wait(barrier);
        if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
            perror("pthread_barrier_wait");
            exit(1);
        }
    }

    return NULL;
}

/**
 * Runs the threads for the simulation.
 *
 * @param world The world to simulate.
 * @param width The width of the world.
 * @param height The height of the world.
 * @param num_turns Number of simulation turns.
 * @param delay Delay between turns in milliseconds.
 * @param step_mode Whether step mode is enabled.
 * @param num_threads Number of threads to use.
 */
void run_threads(int *world, int width, int height, int num_turns, int delay, bool step_mode, int num_threads) {
    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];
    int rc;

    // Allocate the world copy
    int size = width * height;
    int *world_copy = malloc(size * sizeof(int));
    if (world_copy == NULL) {
        fprintf(stderr, "Error allocating memory for world_copy\n");
        exit(1);
    }

    // Initialize the barrier
    pthread_barrier_t barrier;
    if (pthread_barrier_init(&barrier, NULL, num_threads) != 0) {
        perror("pthread_barrier_init");
        exit(EXIT_FAILURE);
    }

    // Determine the partitioning of rows
    int rows_per_thread = height / num_threads;
    int remainder = height % num_threads;
    int start_row = 0;

    for (int i = 0; i < num_threads; i++) {
        thread_data[i].world = world;
        thread_data[i].world_copy = world_copy;
        thread_data[i].width = width;
        thread_data[i].height = height;
        thread_data[i].num_turns = num_turns;
        thread_data[i].delay = delay;
        thread_data[i].step_mode = step_mode;
        thread_data[i].thread_id = i;
        thread_data[i].barrier = &barrier;

        int rows = rows_per_thread + (i < remainder ? 1 : 0);
        thread_data[i].start_row = start_row;
        thread_data[i].end_row = start_row + rows;
        start_row += rows;

        rc = pthread_create(&threads[i], NULL, simulate_world, &thread_data[i]);
        if (rc != 0) {
            fprintf(stderr, "Error creating thread %d: %s\n", i, strerror(rc));
            exit(1);
        }
    }

    for (int i = 0; i < num_threads; i++) {
        rc = pthread_join(threads[i], NULL);
        if (rc != 0) {
            fprintf(stderr, "Error joining thread %d: %s\n", i, strerror(rc));
            exit(1);
        }
    }

    // Destroy the barrier
    if (pthread_barrier_destroy(&barrier) != 0) {
        perror("pthread_barrier_destroy");
        exit(1);
    }

    // Free the world_copy
    free(world_copy);
}

int main(int argc, char *argv[]) {
    // Step 1: Parse command line args
    char *config_filename = NULL;
    int delay = 250;      // default value for delay between turns is 250 ms
    int num_turns = 20;   // default to 20 turns per simulation
    bool step_mode = false; // unless specified, step_mode is off
    int num_threads = 2;  // default to 2 threads

    char ch;

    while ((ch = getopt(argc, argv, "c:t:d:sp:")) != -1) {
        switch (ch) {
            case 's':
                step_mode = true;
                break;
            case 'c':
                config_filename = optarg;
                break;
            case 't':
                if (sscanf(optarg, "%d", &num_turns) != 1) {
                    fprintf(stderr, "Invalid value for -t: %s\n", optarg);
                    usage(argv[0]);
                }
                break;
            case 'd':
                if (sscanf(optarg, "%d", &delay) != 1) {
                    fprintf(stderr, "Invalid value for -d: %s\n", optarg);
                    usage(argv[0]);
                }
                break;
            case 'p':
                if (sscanf(optarg, "%d", &num_threads) != 1 || num_threads <= 0) {
                    fprintf(stderr, "Invalid value for -p: %s\n", optarg);
                    usage(argv[0]);
                }
                break;
            default:
                usage(argv[0]);
        }
    }

    // if config_filename is NULL, then the -c option was missing.
    if (config_filename == NULL) {
        fprintf(stderr, "Missing -c option\n");
        usage(argv[0]);
    }

    // Print summary of simulation options
    fprintf(stdout, "Config Filename: %s\n", config_filename);
    fprintf(stdout, "Number of turns: %d\n", num_turns);
    fprintf(stdout, "Number of threads: %d\n", num_threads);

    if (step_mode == true) {
        fprintf(stdout, "Step mode: Enabled\n");
    }
    else {
        fprintf(stdout, "Step mode: Disabled\n");
        fprintf(stdout, "Delay between turns: %d ms\n", delay);
    }

    // Step 2: Set up the text-based ncurses UI window.
    initscr();   // initialize screen
    cbreak();    // set mode that allows user input to be immediately available
    noecho();    // don't print the characters that the user types in
    clear();     // clears the window

    // Step 3: Create and initialize the world.
    int width, height;
    int *world = initialize_world(config_filename, &height, &width);
    if (world == NULL) {
        endwin(); // close the ncurses UI window
        fprintf(stderr, "Error initializing world\n");
        exit(1);
    }

    // Run the simulation
    run_threads(world, width, height, num_turns, delay, step_mode, num_threads);

    // Frees the world
    free(world);

    // Step 5: Wait for the user to type a character before ending the program.
    // Don't change anything below here.

    // print message to the bottom of the screen (i.e. on the last line)
    mvaddstr(LINES - 1, 0, "Press any key to end the program.");

    getch();   // wait for user to enter a key
    endwin();  // close the ncurses UI window

    return 0;
}
