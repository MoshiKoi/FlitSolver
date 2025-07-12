#include <iostream>
#include <print>
#include <random>
#include <string>

import flit.game;
import flit.evaluator;

void
repl()
{
	std::random_device device{};
	std::mt19937 gen{device()};
	std::uniform_int_distribution<int> row_dist{0, flit::rows - 1};
	std::uniform_int_distribution<int> col_dist{0, flit::cols - 1};

	flit::GameState state{};
	std::string line;
	while (true)
	{
		std::println("{}", flit::dump(state));
		std::print("> ");
		std::getline(std::cin, line);
		if (line == "done")
		{
			break;
		}
		else if (line == "random")
		{
			auto try_set = [&](flit::Cell cell)
			{
				std::uint_fast8_t row;
				std::uint_fast8_t col;
				do
				{
					row = row_dist(gen);
					col = col_dist(gen);
				} while (state.get(row, col) != flit::Cell::Empty);
				state.set(row, col, cell);
			};
			try_set(flit::Cell::Green);
			try_set(flit::Cell::Green);
			try_set(flit::Cell::Purple);
			try_set(flit::Cell::Purple);
		}
		else if (line == "clear")
		{
			state = {};
		}
		else if (line == "eval green")
		{
			flit::Solver solver{state};
			for (auto [move, score] : solver.solve(flit::Cell::Green))
			{
				std::println("{} : {}", move, score);
			}
		}
		else if (line == "eval purple")
		{
			flit::Solver solver{state};
			for (auto [move, score] : solver.solve(flit::Cell::Purple))
			{
				std::println("{} : {}", move, score);
			}
		}
		else
		{
			int row;
			char col;
			char color;
			std::sscanf(line.c_str(), " %c%d%c", &col, &row, &color);
			bool valid = row >= 1 && row <= flit::rows && col >= 'A' && col < 'A' + flit::cols;
			flit::Cell cell;
			switch (color)
			{
			case 'G': cell = flit::Cell::Green; break;
			case 'P': cell = flit::Cell::Purple; break;
			case 'B': cell = flit::Cell::Blue; break;
			default: valid = false; break;
			}
			if (!valid)
				continue;
			state.set(row - 1, col - 'A', cell);
		}
	}
}

int
main()
{
	repl();
}