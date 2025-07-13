# FlitSolver

Alpha-Beta search implementation for the game of [Flit](https://github.com/trichoplax/flit).

# Build

Building requires a C++23 compiler and CMake 3.28 or later.

Tests require the Catch2 framework.

# Algorithm

FlitSolver is based on the [*-minimax](https://en.wikipedia.org/wiki/Expectiminimax) algorithm

- [x] alpha-beta pruning
- [x] transposition table lookup.
- [ ] iterative deepening


# CLI

When running without any arguments, FlitSolver starts a repl-like session with an empty board.
