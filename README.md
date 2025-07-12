# FlitSolver

Alpha-Beta search implementation for the game of [Flit](https://github.com/trichoplax/flit).

# Build

Building requires a C++23 compiler and CMake 3.28 or later.

Tests require the Catch2 framework.

# Algorithm

FlitSolver is based on [*-minimax](https://en.wikipedia.org/wiki/Expectiminimax) with iterative deepening, alpha-beta pruning and transposition table lookup.

# CLI

When running without any arguments, FlitSolver starts a repl-like session with an empty board.

FlitSolver expects moves of the following form:

 - Moves of the form `a1-l11` are interpreted as a move.
 - Moves of the form `a1 g` are interpreted as placing a piece