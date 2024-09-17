# Parallel Game of Life

This is a parallel implementation of Conway's Game of Life, a cellular automaton devised by mathematician John Conway. The Game of Life is a zero-player game that evolves based on its initial state, requiring no further input. The universe of the Game of Life is an infinite two-dimensional orthogonal grid of square cells, each of which is in one of two possible states: alive or dead.

This implementation uses Pthreads to parallelize the computation of each generation, allowing the simulation to run faster on multi-core systems. It includes a custom barrier implementation to synchronize threads, ensuring correctness across different operating systems, including macOS.

## Author

Rodolfo Lopez

## Date

November 29, 2020

## Features

- **Parallel Execution**: Utilizes multiple threads to compute the next state of the game board concurrently.
- **Custom Barrier Synchronization**: Implements a custom barrier using mutexes and condition variables for thread synchronization.
- **Toroidal World**: The game board is a torus; cells on the edges wrap around to the opposite edge.
- **Configurable Parameters**: Users can specify the configuration file, number of turns, delay between turns, and number of threads.
- **Interactive Mode**: Supports step mode, where the user can proceed to the next generation by pressing a key.

## Files

- `main.c`: The main driver program that initializes the game and manages threads.
- `gol.h` and `gol.c`: Header and implementation files containing the Game of Life logic.
- `barrier.h` and `barrier.c`: Custom barrier implementation for thread synchronization.
- `Makefile`: Build script to compile the program.
- `tests/`: Directory containing sample configuration files.
  - `oscillator.txt`
  - `diehard.txt`
  - `multi-oscillator.txt`
  - `r-pentomino.txt`

## Configuration File Format

```
<num_rows>
<num_cols>
<num_live_cells>
<col1> <row1>
<col2> <row2>
...
```

## Dependencies

- **C Compiler**: GCC or Clang with support for C11 standard.
- **Pthreads Library**: For threading support.
- **ncurses Library**: For text-based UI rendering.

## Compilation

Clean Previous Builds (Optional):

```
make clean
```

Use the provided Makefile to compile the project:

```
make
```

## Usage

Run the compiled program with the following options:

```
./gol [-s] -c <config-file> -t <number of turns> -d <delay in ms> -p <number of threads>
```

- `-s`: Enable step mode (optional)
- `-c`: Specify the configuration file (required)
- `-t`: Set the number of simulation turns (default: 20)
- `-d`: Set the delay between turns in milliseconds (default: 250)
- `-p`: Set the number of threads to use (default: 2)

## Example

```
./gol -c tests/r-pentomino.txt -t 100 -d 100 -p 4
```

This runs the simulation using the R-pentomino pattern for 100 turns, with a 100ms delay between turns, using 4 threads.

```
Config Filename: tests/r-pentomino.txt
Number of turns: 100
Number of threads: 4
Step mode: Disabled
Delay between turns: 100 ms
Thread  0: rows   0:  4 (5)
Thread  1: rows   5:  9 (5)
Thread  2: rows  10: 14 (5)
Thread  3: rows  15: 18 (4)
```

## How It Works

1. **Initialization**: The program reads the configuration file to set up the initial state of the world.

2. **Thread Partitioning**: The game board is partitioned row-wise among the specified number of threads. Each thread is responsible for updating a specific range of rows.

3. **Custom Barrier**: A custom barrier synchronization mechanism ensures that all threads complete their computation for a generation before moving to the next one.

4. **Simulation Loop**: For each generation:

- Threads compute the next state for their assigned rows.
- Synchronization barriers ensure correct sequencing.
- The main thread handles printing the world and managing delays or step mode.

5. **Termination**: After completing the specified number of generations, the program waits for a key press before exiting.

## Test

No memory leaks or errors are reported when running the program with Valgrind:

```
valgrind --leak-check=full ./gol -c tests/oscillator.txt -p 5
```

## Notes

This program converts the serial program [Rodolfo Lopez's GOL](https://github.com/lopezrodolfo/GOL.git) into a parallel program.
