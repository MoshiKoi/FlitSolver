module;

#include <algorithm>
#include <limits>
#include <unordered_map>
#include <utility>

export module flit.evaluator;

export import flit.game;

namespace flit
{

export struct solve_result
{
	Move move;
	int score;
};

export class Solver
{
  public:
	Solver(GameState state) : state{std::move(state)} {}

	std::vector<solve_result> solve(Cell player)
	{
		std::vector<solve_result> evaluations;
		for (Move move : state.get_legal_moves(player))
		{
			state.commit(move);
			evaluations.emplace_back(move, -evaluate(opponent(player), true, 1, -10000, 10000));
			state.uncommit(move);
		}
		std::ranges::sort(evaluations, [](auto const &a, auto const &b) { return a.score > b.score; });
		return evaluations;
	}

  private:
	int evaluate(Cell player, bool blue, int depth, int alpha, int beta)
	{
		if (blue)
		{
			int total_spawn_score = 0;
			int count = 0;
			for (auto idx : state.get_possible_spawns())
			{
				state.set(idx, Cell::Blue);
				total_spawn_score += evaluate(player, false, depth, alpha, beta);
				++count;
				state.unset(idx);
			}
			int avg_spawn_score = total_spawn_score / count;
			int no_spawn_score = evaluate(player, false, depth, alpha, beta);
			// There is a 1/6 chance of actually having a spawn
			return (5 * no_spawn_score + avg_spawn_score) / 6;
		}
		else if (depth > 0)
		{
			int score = std::numeric_limits<int>::min();
			for (Move move : state.get_legal_moves(player))
			{
				state.commit(move);
				score = std::max(score, -evaluate(opponent(player), true, depth - 1, alpha, beta));
				state.uncommit(move);
			}
			return score;
		}
		else
		{
			return heuristic(player);
		}
	}

	int heuristic(Cell player) const
	{
		int score = 0;
		for (std::uint_fast8_t row = 0; row < rows; ++row)
		{
			for (std::uint_fast8_t col = 0; col < cols; ++col)
			{
				if (state.get(row, col) == player)
				{
					++score;
				}
				else if (state.get(row, col) == opponent(player)) {
					--score;
				}
			}
		}
		return score * 100;
	}

	GameState state;
	std::unordered_map<std::uint64_t, int> _scores;
};

} // namespace flit