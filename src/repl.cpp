module;

#include <chrono>
#include <iostream>
#include <map>
#include <print>
#include <random>
#include <stdexcept>
#include <string>

export module flit.repl;

import flit.game;
import flit.evaluator;

namespace flit
{

struct Coord
{
	std::uint_fast8_t row;
	std::uint_fast8_t col;
};

class Tokenizer
{
  public:
	Tokenizer(std::string_view view = {}) : _ptr{view.data()}, _end{view.data() + view.size()} {}

	std::string_view read_word()
	{
		skip_whitespace();
		char const *start = _ptr;
		while (_ptr != _end and not std::isspace(*_ptr))
		{
			++_ptr;
		}
		return {start, _ptr};
	}

	Coord read_coord()
	{
		skip_whitespace();

		if (_ptr > _end - 2)
		{
			throw std::runtime_error{"Invalid coordinate"};
		}

		int col = *_ptr - 'A';
		int row;
		auto [ptr, errc] = std::from_chars(_ptr + 1, _end, row);

		if (errc != std::errc{})
		{
			throw std::runtime_error{"Invalid coordinate"};
		}

		row -= 1;

		if (row < 0 or row >= flit::rows or col < 0 or col >= flit::cols)
		{
			throw std::runtime_error{"Invalid coordinate"};
		}
		_ptr = ptr;
		return {static_cast<std::uint_fast8_t>(row), static_cast<std::uint_fast8_t>(col)};
	}

	Cell read_color()
	{
		static const std::map<std::string, Cell, std::less<>> s_colors{
			{"green", Cell::Green},
			{"purple", Cell::Purple},
			{"blue", Cell::Blue},
		};
		if (auto iter = s_colors.find(read_word()); iter != s_colors.end())
		{
			return iter->second;
		}
		else
		{
			throw std::runtime_error{"Invalid color"};
		}
	}

	int read_int()
	{
		skip_whitespace();
		if (_ptr == _end)
		{
			throw std::runtime_error{"Expected a value"};
		}
		int value;
		auto [ptr, errc] = std::from_chars(_ptr, _end, value);
		if (errc != std::errc{})
		{
			throw std::runtime_error{"Invalid value"};
		}
		_ptr = ptr;
		return value;
	}

  private:
	void skip_whitespace()
	{
		while (_ptr != _end and std::isspace(*_ptr))
		{
			++_ptr;
		}
	}
	char const *_ptr;
	char const *_end;
};

class Repl
{
  public:
	Repl(std::mt19937 gen) : _gen{std::move(gen)} {}

	void interpret(std::string_view line)
	{
		static std::map<std::string, void (Repl::*)(), std::less<>> const s_commands{
			{"random", &Repl::random},
			{"clear", &Repl::clear},
			{"set", &Repl::set},
			{"eval", &Repl::eval},
		};

		_tokenizer = {line};
		auto string = _tokenizer.read_word();
		auto iter = s_commands.find(string);
		if (iter != s_commands.end())
		{
			(this->*(iter->second))();
		}
		else
		{
			throw std::runtime_error{"Invalid command"};
		}
	}

	GameState const &state() const { return _state; }

  private:
	void random()
	{
		std::uniform_int_distribution<int> row_dist{0, flit::rows - 1};
		std::uniform_int_distribution<int> col_dist{0, flit::cols - 1};

		auto try_set = [&](flit::Cell cell)
		{
			std::uint_fast8_t row;
			std::uint_fast8_t col;
			do
			{
				row = row_dist(_gen);
				col = col_dist(_gen);
			} while (_state.get(row, col) != flit::Cell::Empty);
			_state.set(row, col, cell);
		};
		try_set(flit::Cell::Green);
		try_set(flit::Cell::Green);
		try_set(flit::Cell::Purple);
		try_set(flit::Cell::Purple);
	}

	void clear() { _state = {}; }

	void set()
	{
		Coord coord = _tokenizer.read_coord();
		Cell cell = _tokenizer.read_color();
		_state.set(coord.row, coord.col, cell);
	}

	void eval()
	{
		Cell color = _tokenizer.read_color();
		int depth = _tokenizer.read_int();
		Solver solver{_state};
		std::println("Move : Evaluation");
		for (auto [move, score] : solver.solve(color, depth))
		{
			std::println("{} : {}", move, score);
		}
	}

	Tokenizer _tokenizer;
	GameState _state;
	std::mt19937 _gen;
};

export void
repl()
{
	std::random_device device{};
	Repl repl{std::mt19937{device()}};

	std::string line;
	while (true)
	{
		std::println("{}", flit::dump(repl.state()));
		std::print("> ");
		std::getline(std::cin, line);
		try
		{
			auto begin = std::chrono::steady_clock::now();
			repl.interpret(line);
			auto end = std::chrono::steady_clock::now();
			std::println("Completed in {}", std::chrono::duration_cast<std::chrono::seconds>(end - begin));
		}
		catch (std::exception &ex)
		{
			std::println("{}", ex.what());
		}
	}
}

} // namespace flit