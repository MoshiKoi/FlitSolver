module;

#include <libassert/assert.hpp>
#include <xxhash.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <generator>
#include <ranges>

export module flit.game;

namespace flit
{

export constexpr std::uint_fast8_t rows = 12;
export constexpr std::uint_fast8_t cols = 12;
constexpr std::uint_fast8_t num_cells = rows * cols;

export constexpr std::uint_fast8_t
from_rc(std::uint_fast8_t row, std::uint_fast8_t col) noexcept
{
	LIBASSERT_DEBUG_ASSERT(row < rows);
	LIBASSERT_DEBUG_ASSERT(col < cols);
	return row * cols + col;
}

export struct Move
{
	/// Cell index of origin
	std::uint_fast8_t from;
	/// Cell index of destination
	std::uint_fast8_t to;
	// Direction of any blues that will be converted by this move
	std::uint_fast8_t blue_flags;

	friend bool operator==(Move, Move);
};

[[clang::always_inline]]
bool operator==(Move, Move) = default;

export enum class Cell : std::uint_fast8_t {
	Empty = 0,
	Green = 1,
	Purple = 2,
	Blue = 3,
};

export Cell
opponent(Cell player) noexcept
{
	LIBASSERT_DEBUG_ASSERT(player == Cell::Green or player == Cell::Purple);
	return static_cast<Cell>(std::to_underlying(player) ^ std::uint_fast8_t{0b11});
}

namespace
{

std::array<std::uint_fast8_t, 4>
get_neighbors(std::uint_fast8_t idx)
{
	LIBASSERT_DEBUG_ASSERT(idx < num_cells);
	return {
		static_cast<std::uint_fast8_t>((idx + num_cells - rows) % num_cells),
		static_cast<std::uint_fast8_t>((idx + rows) % num_cells),
		static_cast<std::uint_fast8_t>((idx + 1) - rows * !((idx + 1) % cols)),
		static_cast<std::uint_fast8_t>((idx - 1) + rows * !(idx % cols)),
	};
}

} // namespace

std::array<std::array<std::uint_fast8_t, 4>, num_cells> const neighbors = []
{
	std::remove_cv_t<decltype(neighbors)> result{};
	std::ranges::copy(
		std::views::iota(std::uint_fast8_t{0}, num_cells) | std::views::transform(get_neighbors), result.begin());
	return result;
}();

export class GameState
{
  public:
	void set(std::uint_fast8_t row, std::uint_fast8_t col, Cell cell)
	{
		std::uint_fast8_t idx = from_rc(row, col);
		unset(idx);
		set(idx, cell);
	}

	Cell get(std::uint_fast8_t row, std::uint_fast8_t col) const { return _board[from_rc(row, col)]; }

	void commit(Move move)
	{
		LIBASSERT_DEBUG_ASSERT(_board[move.from] == _turn);
		LIBASSERT_DEBUG_ASSERT(_board[move.to] == Cell::Empty);

		set(move.to, _board[move.from]);
		for (int i = 0; i < 4; ++i)
		{
			if (move.blue_flags & (1 << i))
			{
				set(neighbors[move.to][i], _board[move.from]);
			}
		};
		unset(move.from);
		_turn = opponent(_turn);
	}

	void uncommit(Move move)
	{
		LIBASSERT_DEBUG_ASSERT(_board[move.to] == opponent(_turn));
		LIBASSERT_DEBUG_ASSERT(_board[move.from] == Cell::Empty);

		set(move.from, _board[move.to]);
		for (int i = 0; i < 4; ++i)
		{
			if (move.blue_flags & (1 << i))
			{
				unset(neighbors[move.to][i]);
				set(neighbors[move.to][i], Cell::Blue);
			}
		};
		unset(move.to);
		_turn = opponent(_turn);
	}

	std::generator<Move> get_legal_moves() const
	{
		LIBASSERT_DEBUG_ASSERT(_turn == Cell::Green or _turn == Cell::Purple);
		std::uint8_t const *player_cover = _turn == Cell::Green ? _green_cover : _purple_cover;

		for (std::uint_fast8_t target = 0; target < num_cells; ++target)
		{
			if (_board[target] == Cell::Empty and player_cover[target] > 0)
			{
				for (std::uint_fast8_t source = 0; source < num_cells; ++source)
				{
					if (_board[source] != _turn)
					{
						continue;
					}
					else if (player_cover[target] == 1 and std::ranges::contains(neighbors[target], source))
					{
						continue;
					}
					std::uint_fast8_t blue_flags = 0;
					for (auto [dir, neighbor] : neighbors[target] | std::views::enumerate)
					{
						if (_board[neighbor] == Cell::Blue)
						{
							blue_flags |= 1 << dir;
						}
					}
					co_yield Move{.from = source, .to = target, .blue_flags = blue_flags};
				}
			}
		}
	}

	std::generator<std::uint_fast8_t> get_possible_spawns() const
	{
		for (std::uint_fast8_t idx = 0; idx < num_cells; ++idx)
		{
			if (_board[idx] == Cell::Empty and _green_cover[idx] == 0 and _purple_cover[idx] == 0)
				co_yield idx;
		}
	}

	void unset(std::uint_fast8_t idx)
	{
		switch (_board[idx])
		{
		case Cell::Empty: break;
		case Cell::Green:
			for (auto neighbor : neighbors[idx])
			{
				--_green_cover[neighbor];
			}
			--_heuristic;
			break;
		case Cell::Purple:
			for (auto neighbor : neighbors[idx])
			{
				--_purple_cover[neighbor];
			}
			++_heuristic;
			break;
		case Cell::Blue: break;
		default: std::unreachable();
		}
		_board[idx] = Cell::Empty;
	}

	void set(std::uint_fast8_t idx, Cell cell)
	{
		LIBASSERT_DEBUG_ASSERT(_board[idx] != Cell::Green);
		LIBASSERT_DEBUG_ASSERT(_board[idx] != Cell::Purple);
		LIBASSERT_DEBUG_ASSERT(cell != Cell::Empty);
		_board[idx] = cell;
		switch (cell)
		{
		case Cell::Green:
			for (auto neighbor : neighbors[idx])
			{
				++_green_cover[neighbor];
			}
			++_heuristic;
			break;
		case Cell::Purple:
			for (auto neighbor : neighbors[idx])
			{
				++_purple_cover[neighbor];
			}
			--_heuristic;
			break;
		case Cell::Blue: break;
		default: std::unreachable();
		}
	}

	// TODO: Incremental hash
	std::uint64_t hash() const
	{
		return XXH3_64bits(_board, num_cells * sizeof(*_board)) & XXH3_64bits(&_turn, sizeof(Cell));
	}
	int heuristic() const { return (_turn == Cell::Green ? _heuristic : -_heuristic) * 1000; }

	Cell turn() const { return _turn; }
	void turn(Cell turn) { _turn = turn; }

  private:
	Cell _turn = Cell::Empty;
	int _heuristic = 0;
	Cell _board[num_cells] = {};
	std::uint8_t _green_cover[num_cells] = {};
	std::uint8_t _purple_cover[num_cells] = {};

	friend std::string dump(flit::GameState const &state);
};

export std::string
dump(flit::GameState const &state)
{
	std::string out;
	std::format_to(std::back_inserter(out), "{: ^{}}|{: ^{}}|{: ^{}}\n", "Board", cols, "Green", cols, "Purple", cols);
	for (std::uint_fast8_t row = 0; row < flit::rows; ++row)
	{
		for (std::uint_fast8_t col = 0; col < flit::cols; ++col)
		{
			char c = ".GPB"[static_cast<std::size_t>(state.get(row, col))];
			out.push_back(c);
		}
		out.push_back('|');
		for (std::uint_fast8_t col = 0; col < flit::cols; ++col)
		{
			char c = '0' + state._green_cover[flit::from_rc(row, col)];
			out.push_back(c);
		}
		out.push_back('|');
		for (std::uint_fast8_t col = 0; col < flit::cols; ++col)
		{
			char c = '0' + state._purple_cover[flit::from_rc(row, col)];
			out.push_back(c);
		}
		out.push_back('\n');
	}
	return out;
}

} // namespace flit

template <>
struct std::formatter<flit::Move> : std::formatter<std::string_view>
{
	auto format(const flit::Move &move, auto &ctx) const
	{
		int from_row = move.from / flit::cols + 1;
		char from_col = move.from % flit::cols + 'A';
		int to_row = move.to / flit::cols + 1;
		char to_col = move.to % flit::cols + 'A';
		return std::format_to(ctx.out(), "{}{}-{}{}", from_col, from_row, to_col, to_row);
	}
};