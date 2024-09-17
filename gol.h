#ifndef __GOL_H__
#define __GOL_H__
/**
 * File: gol.h
 *
 * Header file of the Game of Life simulator functions.
 */

/**
 * Creates and initializes the world based on the given configuration file.
 *
 * @param config_filename The name of the file containing the simulation
 *    configuration data (e.g., world dimensions)
 * @param width Location where to store the width of the world.
 * @param height Location where to store the height of the world.
 *
 * @return A 1D array representing the created/initialized world, or NULL if
 *   there was a problem with initialization.
 */
int *initialize_world(char *config_filename, int *width, int *height);

/**
 * Updates the world for one step of simulation, based on the rules of the
 * game of life.
 *
 * @param world The world to update.
 * @param world_copy A copy of the world to read from.
 * @param width The width of the world.
 * @param height The height of the world.
 * @param start_row The starting row index (inclusive).
 * @param end_row The ending row index (exclusive).
 */
void update_world(int *world, int *world_copy, int width, int height, int start_row, int end_row);

/**
 * Prints the given world using the ncurses UI library.
 *
 * @param world The world to print.
 * @param width The width of the world.
 * @param height The height of the world.
 * @param turn The current turn number.
 */
void print_world(int *world, int width, int height, int turn);

/**
 * Translates a 2D coordinate to a 1D index, with toroidal wrap-around.
 *
 * @param row The row index.
 * @param col The column index.
 * @param width The width of the grid (number of columns).
 * @param height The height of the grid (number of rows).
 * @return The corresponding index in the 1D array.
 */
int translate_to_1D(int row, int col, int width, int height);

#endif

