#include <catch2/catch_test_macros.hpp>
#include <libassert/assert-catch2.hpp>

import flit.game;
import flit.evaluator;

TEST_CASE("Best move should be immediate capture", "[evaluator]")
{
	flit::GameState state{};
	state.set(4, 5, flit::Cell::Green);
	state.set(5, 5, flit::Cell::Green);
	state.set(7, 5, flit::Cell::Blue);
	state.set(0, 0, flit::Cell::Purple);
	state.set(0, 1, flit::Cell::Purple);
	state.turn(flit::Cell::Green);
	INFO(flit::dump(state));
	flit::Solver evaluator{state};

	auto results = evaluator.solve(flit::Cell::Green, 0);
	ASSERT(results.size() > 0);
	auto [best_move, score] = results[0];
	ASSERT(best_move.from == flit::from_rc(4, 5));
	ASSERT(best_move.to == flit::from_rc(6, 5));
}

TEST_CASE("Best move should try to reach blue", "[evaluator]")
{
	flit::GameState state{};
	state.set(4, 5, flit::Cell::Green);
	state.set(5, 5, flit::Cell::Green);
	state.set(8, 5, flit::Cell::Blue);
	state.set(0, 0, flit::Cell::Purple);
	state.set(0, 1, flit::Cell::Purple);
	state.turn(flit::Cell::Green);
	INFO(flit::dump(state));
	flit::Solver evaluator{state};

	auto results = evaluator.solve(flit::Cell::Green, 2);
	auto [best_move, score] = results[0];
	ASSERT(best_move.from == flit::from_rc(4, 5));
	ASSERT(best_move.to == flit::from_rc(6, 5));
}

TEST_CASE("Should capture blue before opponent", "[evaluator]")
{
	flit::GameState state{};
	state.set(4, 8, flit::Cell::Green);
	state.set(5, 8, flit::Cell::Green);
	state.set(4, 10, flit::Cell::Blue);
	state.set(8, 8, flit::Cell::Blue);
	state.set(8, 5, flit::Cell::Purple);
	state.set(8, 4, flit::Cell::Purple);
	state.turn(flit::Cell::Green);
	INFO(flit::dump(state));
	flit::Solver evaluator{state};
	auto results = evaluator.solve(flit::Cell::Green, 3);
	ASSERT(results.size() > 0);
	auto [best_move, score] = results[0];
	ASSERT(best_move.from == flit::from_rc(4, 8));
	ASSERT(best_move.to == flit::from_rc(6, 8));
}