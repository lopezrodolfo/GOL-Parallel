/**
 * File: gol.c
 *
 * Implementation of the Game of Life simulator functions.
 */

#define _XOPEN_SOURCE 600

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <curses.h>

#include "gol.h"

/**
 * Translates a 2D coordinate to a 1D index, with toroidal wrap-around.
 *
 * @param row The row index.
 * @param col The column index.
 * @param width The width of the grid (number of columns).
 * @param height The height of the grid (number of rows).
 * @return The corresponding index in the 1D array.
 */
int translate_to_1D(int row, int col, int width, int height) {
    row = (row + height) % height;
    col = (col + width) % width;
    return row * width + col;
}

/**
 * Creates and initializes the world based on the given configuration file.
 *
 * @param config_filename The name of the file containing the simulation
 *    configuration data (e.g., world dimensions)
 * @param num_rows Location where to store the number of rows of the world.
 * @param num_cols Location where to store the number of columns of the world.
 *
 * @return A 1D array representing the created/initialized world, or NULL if
 *   there was a problem with initialization.
 */
int *initialize_world(char *config_filename, int *num_rows, int *num_cols) {
    FILE *fp = fopen(config_filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error opening config file: %s\n", config_filename);
        return NULL;
    }

    // Read num_rows
    if (fscanf(fp, "%d", num_rows) != 1) {
        fprintf(stderr, "Error reading num_rows from config file\n");
        fclose(fp);
        return NULL;
    }

    // Read num_cols
    if (fscanf(fp, "%d", num_cols) != 1) {
        fprintf(stderr, "Error reading num_cols from config file\n");
        fclose(fp);
        return NULL;
    }

    int num_live_cells;
    if (fscanf(fp, "%d", &num_live_cells) != 1) {
        fprintf(stderr, "Error reading number of live cells from config file\n");
        fclose(fp);
        return NULL;
    }

    int size = (*num_rows) * (*num_cols);

    int *world = calloc(size, sizeof(int));
    if (world == NULL) {
        fprintf(stderr, "Error allocating memory for world\n");
        fclose(fp);
        return NULL;
    }

    // Read each coordinate pair and set the corresponding cell to 1
    for (int i = 0; i < num_live_cells; i++) {
        int c, r;
        if (fscanf(fp, "%d %d", &c, &r) != 2) {
            fprintf(stderr, "Error reading coordinate pair from config file\n");
            free(world);
            fclose(fp);
            return NULL;
        }
        // Set the cell at (r, c) to 1
        int index = translate_to_1D(r, c, *num_cols, *num_rows);
        world[index] = 1;
    }

    fclose(fp);
    return world;
}

/**
 * Updates the world for one time step, according to the rules of the Game of Life.
 *
 * @param world The current world, which will be updated in-place.
 * @param world_copy A copy of the world to read from.
 * @param width The width of the world (number of columns).
 * @param height The height of the world (number of rows).
 * @param start_row The starting row index (inclusive).
 * @param end_row The ending row index (exclusive).
 */
void update_world(int *world, int *world_copy, int width, int height, int start_row, int end_row) {
    // For each cell, compute the new state
    for (int row = start_row; row < end_row; row++) {
        for (int col = 0; col < width; col++) {
            int index = translate_to_1D(row, col, width, height);
            int live_neighbors = 0;

            // Check all eight neighbors
            for (int d_row = -1; d_row <= 1; d_row++) {
                for (int d_col = -1; d_col <= 1; d_col++) {
                    if (d_row == 0 && d_col == 0) {
                        // Skip self
                        continue;
                    }
                    int n_row = row + d_row;
                    int n_col = col + d_col;
                    int n_index = translate_to_1D(n_row, n_col, width, height);
                    if (world_copy[n_index] == 1) {
                        live_neighbors++;
                    }
                }
            }

            // Apply the rules
            if (world_copy[index] == 1) {
                // Cell is alive
                if (live_neighbors <= 1 || live_neighbors >= 4) {
                    // Dies
                    world[index] = 0;
                } else {
                    // Lives on
                    world[index] = 1;
                }
            } else {
                // Cell is dead
                if (live_neighbors == 3) {
                    // Becomes alive
                    world[index] = 1;
                } else {
                    // Stays dead
                    world[index] = 0;
                }
            }
        }
    }
}

/**
 * Prints the given world using the ncurses UI library.
 *
 * @param world The world to print.
 * @param width The width of the world.
 * @param height The height of the world.
 * @param turn The current turn number.
 */
void print_world(int *world, int width, int height, int turn) {
    // clear(); // We moved clear() to the thread function to avoid race conditions

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            int index = translate_to_1D(row, col, width, height);
            char ch = (world[index] == 1) ? '@' : '.';
            mvaddch(row, col, ch);
        }
    }

    // Print a blank line after the world
    int last_row = height;
    mvaddstr(last_row, 0, "");

    // Print "Time Step: %d"
    char buf[50];
    snprintf(buf, sizeof(buf), "Time Step: %d", turn);
    mvaddstr(last_row + 1, 0, buf);

    refresh(); // displays the text we've added
}
