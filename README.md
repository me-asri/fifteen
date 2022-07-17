# Fifteen

_Fifteen_ is [15 puzzle](https://en.wikipedia.org/wiki/15_puzzle) (a.k.a Game of Fifteen) implemented in C++ with IDA* based solver.

# Features

 * GUI using FLTK
 * Step by step solver (using IDA*) with 3 different heuristics:
   * Manhattan Distance
   * Linear Conflict
   * Misplaced Tiles

# Playing

 * Move the blank tile around using WASD keys.
 * Edit tiles directly by clicking on them.
 * Randomize puzzle using the _Shuffle_ button.

# Building

## Requirements

 * GNU make
 * GNU C++ Compiler (9.1 and higher)
 * FLTK (inc. FLUID)
 * On Windows: MSYS2

## Installing dependencies

### Linux

#### Debian/Ubuntu

    sudo apt install build-essential libfltk1.3-dev fluid

## Compiling

### Linux

    make -j$(nproc)
    
