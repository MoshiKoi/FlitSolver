module;

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <format>
#include <generator>

export module flit.game;

namespace flit
{

export constexpr std::uint_fast8_t rows = 12;
export constexpr std::uint_fast8_t cols = 12;
constexpr std::uint_fast8_t num_cells = rows * cols;

static_assert(num_cells < 256);

export constexpr std::uint_fast8_t
from_rc(std::uint_fast8_t row, std::uint_fast8_t col) noexcept
{
	return row * cols + col;
}

export struct Move
{
	/// Cell index of origin
	std::uint_fast8_t from;
	/// Cell index of destination
	std::uint_fast8_t to;

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
	return static_cast<Cell>(std::to_underlying(player) ^ std::uint_fast8_t{0b11});
}

namespace
{

std::generator<std::uint_fast8_t>
neighbors(std::uint_fast8_t idx)
{
	co_yield (idx + num_cells - rows) % num_cells;
	co_yield (idx + rows) % num_cells;
	co_yield (idx + 1) - rows * !((idx + 1) % cols);
	co_yield (idx - 1) + rows * !(idx % cols);
}

} // namespace

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
		set(move.to, _board[move.from]);
		unset(move.from);
	};

	void uncommit(Move move)
	{
		set(move.from, _board[move.to]);
		unset(move.to);
	}

	std::generator<Move> get_legal_moves(Cell player) const
	{
		std::uint8_t const *player_cover = player == Cell::Green ? _green_cover : _purple_cover;

		for (std::uint_fast8_t target = 0; target < num_cells; ++target)
		{
			if (_board[target] == Cell::Empty and player_cover[target] > 0)
			{
				for (std::uint_fast8_t source = 0; source < num_cells; ++source)
				{
					if (_board[source] != player)
					{
						continue;
					}
					else if (player_cover[target] == 1 and std::ranges::contains(neighbors(target), source))
					{
						continue;
					}
					co_yield Move{source, target};
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
		for (auto neighbor : neighbors(idx))
		{
			switch (_board[idx])
			{
			case Cell::Empty: break;
			case Cell::Green: --_green_cover[neighbor]; break;
			case Cell::Purple: --_purple_cover[neighbor]; break;
			case Cell::Blue: break;
			default: std::unreachable();
			}
		}
		_board[idx] = Cell::Empty;
	}

	void set(std::uint_fast8_t idx, Cell cell)
	{
		_board[idx] = cell;
		for (auto neighbor : neighbors(idx))
		{
			switch (cell)
			{
			case Cell::Empty: break;
			case Cell::Green: ++_green_cover[neighbor]; break;
			case Cell::Purple: ++_purple_cover[neighbor]; break;
			case Cell::Blue: break;
			default: std::unreachable();
			}
		}
	}

  private:
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