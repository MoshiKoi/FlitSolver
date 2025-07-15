#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>
#include <libassert/assert-catch2.hpp>

#include <ranges>

import flit.game;

using Catch::Matchers::IsEmpty;
using Catch::Matchers::SizeIs;
using Catch::Matchers::UnorderedRangeEquals;

namespace Catch
{
template <>
struct StringMaker<flit::Move>
{
	static std::string convert(flit::Move const &value) { return std::format("{}", value); }
};
} // namespace Catch

TEST_CASE("Invalid board has no moves")
{
	SECTION("Empty board")
	{
		flit::GameState state{};
		INFO(flit::dump(state));
		state.turn(flit::Cell::Green);
		std::vector result = state.get_legal_moves() | std::ranges::to<std::vector>();
		REQUIRE_THAT(result, IsEmpty());
	}

	SECTION("Single piece")
	{
		flit::GameState state{};
		INFO(flit::dump(state));
		state.set(5, 5, flit::Cell::Purple);
		state.turn(flit::Cell::Green);
		std::vector result = state.get_legal_moves() | std::ranges::to<std::vector>();
		REQUIRE_THAT(result, IsEmpty());
	}
}

TEST_CASE("Piece can move to other pieces' adjacent cells")
{
	SECTION("Adjacent pieces")
	{
		flit::GameState state{};
		state.set(4, 5, flit::Cell::Green);
		state.set(5, 5, flit::Cell::Green);
		state.turn(flit::Cell::Green);
		INFO(flit::dump(state));

		std::vector result = state.get_legal_moves() | std::ranges::to<std::vector>();
		std::array expected{
			flit::Move{flit::from_rc(4, 5), flit::from_rc(5, 6)},
			flit::Move{flit::from_rc(4, 5), flit::from_rc(6, 5)},
			flit::Move{flit::from_rc(4, 5), flit::from_rc(5, 4)},
			flit::Move{flit::from_rc(5, 5), flit::from_rc(4, 6)},
			flit::Move{flit::from_rc(5, 5), flit::from_rc(4, 4)},
			flit::Move{flit::from_rc(5, 5), flit::from_rc(3, 5)},
		};

		REQUIRE_THAT(result, UnorderedRangeEquals(expected));
	}

	SECTION("Corner")
	{
		flit::GameState state{};
		state.set(0, 0, flit::Cell::Green);
		state.set(5, 5, flit::Cell::Green);
		state.turn(flit::Cell::Green);
		INFO(flit::dump(state));

		std::vector result = state.get_legal_moves() | std::ranges::to<std::vector>();
		std::array expected{
			flit::Move{flit::from_rc(5, 5), flit::from_rc(0, 1)},
			flit::Move{flit::from_rc(5, 5), flit::from_rc(1, 0)},
			flit::Move{flit::from_rc(5, 5), flit::from_rc(0, 11)},
			flit::Move{flit::from_rc(5, 5), flit::from_rc(11, 0)},
			flit::Move{flit::from_rc(0, 0), flit::from_rc(5, 6)},
			flit::Move{flit::from_rc(0, 0), flit::from_rc(6, 5)},
			flit::Move{flit::from_rc(0, 0), flit::from_rc(5, 4)},
			flit::Move{flit::from_rc(0, 0), flit::from_rc(4, 5)},
		};

		REQUIRE_THAT(result, UnorderedRangeEquals(expected));
	}

	SECTION("One space in between")
	{
		flit::GameState state{};
		state.set(5, 5, flit::Cell::Green);
		state.set(5, 7, flit::Cell::Green);
		state.turn(flit::Cell::Green);
		INFO(flit::dump(state));

		std::vector result = state.get_legal_moves() | std::ranges::to<std::vector>();
		std::array expected{
			flit::Move{flit::from_rc(5, 5), flit::from_rc(5, 8)},
			flit::Move{flit::from_rc(5, 5), flit::from_rc(6, 7)},
			flit::Move{flit::from_rc(5, 5), flit::from_rc(5, 6)},
			flit::Move{flit::from_rc(5, 5), flit::from_rc(4, 7)},
			flit::Move{flit::from_rc(5, 7), flit::from_rc(5, 6)},
			flit::Move{flit::from_rc(5, 7), flit::from_rc(6, 5)},
			flit::Move{flit::from_rc(5, 7), flit::from_rc(5, 4)},
			flit::Move{flit::from_rc(5, 7), flit::from_rc(4, 5)},
		};

		REQUIRE_THAT(result, UnorderedRangeEquals(expected));
	}
}

TEST_CASE("Moving adjacent to blue captures the blue")
{
	SECTION("One blue piece")
	{
		flit::GameState state{};
		state.set(4, 5, flit::Cell::Green);
		state.set(5, 5, flit::Cell::Green);
		state.set(5, 7, flit::Cell::Blue);
		state.turn(flit::Cell::Green);
		INFO(flit::dump(state));
		std::vector moves = state.get_legal_moves() | std::ranges::to<std::vector>();

		std::vector capturing_moves = moves
			| std::views::filter([](flit::Move move) { return move.blue_flags != 0; })
			| std::ranges::to<std::vector>();

		REQUIRE_THAT(capturing_moves, SizeIs(1));

		flit::Move capture_move = capturing_moves[0];

		ASSERT(capture_move.from == flit::from_rc(4, 5));
		ASSERT(capture_move.to == flit::from_rc(5, 6));
		ASSERT(std::popcount(capture_move.blue_flags) == 1);

		state.commit(capture_move);

		ASSERT(state.get(4, 5) == flit::Cell::Empty);
		ASSERT(state.get(5, 6) == flit::Cell::Green);
		ASSERT(state.get(5, 7) == flit::Cell::Green);

		state.uncommit(capture_move);

		ASSERT(state.get(4, 5) == flit::Cell::Green);
		ASSERT(state.get(5, 6) == flit::Cell::Empty);
		ASSERT(state.get(5, 7) == flit::Cell::Blue);
	}

	SECTION("Multiple blue pieces")
	{

		flit::GameState state{};
		state.set(4, 5, flit::Cell::Green);
		state.set(5, 5, flit::Cell::Green);
		state.set(5, 7, flit::Cell::Blue);
		state.set(6, 6, flit::Cell::Blue);
		state.turn(flit::Cell::Green);
		INFO(flit::dump(state));
		std::vector moves = state.get_legal_moves() | std::ranges::to<std::vector>();

		std::vector capturing_moves = moves
			| std::views::filter([](flit::Move move) { return move.blue_flags != 0; })
			| std::ranges::to<std::vector>();

		REQUIRE_THAT(capturing_moves, SizeIs(2));

		std::ranges::sort(
			capturing_moves,
			[](flit::Move a, flit::Move b) { return std::popcount(a.blue_flags) < std::popcount(b.blue_flags); });

		flit::Move capture_move1 = capturing_moves[0];

		ASSERT(capture_move1.from == flit::from_rc(4, 5));
		ASSERT(capture_move1.to == flit::from_rc(6, 5));
		ASSERT(std::popcount(capture_move1.blue_flags) == 1);

		state.commit(capture_move1);

		ASSERT(state.get(4, 5) == flit::Cell::Empty);
		ASSERT(state.get(6, 5) == flit::Cell::Green);
		ASSERT(state.get(6, 6) == flit::Cell::Green);

		state.uncommit(capture_move1);

		ASSERT(state.get(4, 5) == flit::Cell::Green);
		ASSERT(state.get(6, 5) == flit::Cell::Empty);
		ASSERT(state.get(6, 6) == flit::Cell::Blue);

		flit::Move capture_move2 = capturing_moves[1];

		ASSERT(capture_move2.from == flit::from_rc(4, 5));
		ASSERT(capture_move2.to == flit::from_rc(5, 6));

		state.commit(capture_move2);

		ASSERT(state.get(4, 5) == flit::Cell::Empty);
		ASSERT(state.get(5, 6) == flit::Cell::Green);
		ASSERT(state.get(6, 6) == flit::Cell::Green);
		ASSERT(state.get(5, 7) == flit::Cell::Green);

		state.uncommit(capture_move2);

		ASSERT(state.get(4, 5) == flit::Cell::Green);
		ASSERT(state.get(5, 6) == flit::Cell::Empty);
		ASSERT(state.get(6, 6) == flit::Cell::Blue);
		ASSERT(state.get(5, 7) == flit::Cell::Blue);
	}
}