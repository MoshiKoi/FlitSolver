#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

#include <ranges>

import flit.game;

using Catch::Matchers::IsEmpty;
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
		std::vector result = state.get_legal_moves(flit::Cell::Green) | std::ranges::to<std::vector>();
		REQUIRE_THAT(result, IsEmpty());
	}

	SECTION("Single piece")
	{
		flit::GameState state{};
		INFO(flit::dump(state));
		state.set(5, 5, flit::Cell::Purple);
		std::vector result = state.get_legal_moves(flit::Cell::Green) | std::ranges::to<std::vector>();
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
		INFO(flit::dump(state));

		std::vector result = state.get_legal_moves(flit::Cell::Green) | std::ranges::to<std::vector>();
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
		INFO(flit::dump(state));

		std::vector result = state.get_legal_moves(flit::Cell::Green) | std::ranges::to<std::vector>();
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

	SECTION("One space in between") {
		flit::GameState state{};
		state.set(5, 5, flit::Cell::Green);
		state.set(5, 7, flit::Cell::Green);
		INFO(flit::dump(state));

		std::vector result = state.get_legal_moves(flit::Cell::Green) | std::ranges::to<std::vector>();
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