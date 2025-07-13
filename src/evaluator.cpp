module;

#include <algorithm>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

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
	Solver(GameState state, std::size_t transposition_table_size = 1 << 15)
		: state{std::move(state)}, _transposition_table_size{transposition_table_size},
		  _transposition_table{std::make_unique<TranspositionTableEntry[]>(_transposition_table_size)}
	{
	}

	std::vector<solve_result> solve(Cell player)
	{
		std::vector<solve_result> evaluations;
		for (Move move : state.get_legal_moves(player))
		{
			state.commit(move);
			evaluations.emplace_back(move, -evaluate(opponent(player), true, 0, -10000, 10000));
			state.uncommit(move);
		}
		std::ranges::sort(evaluations, [](auto const &a, auto const &b) { return a.score > b.score; });
		return evaluations;
	}

  private:
	int evaluate(Cell player, bool blue, int depth, int alpha, int beta)
	{
		auto hash = state.hash();
		auto &entry = get_transposition_entry(hash);
		if (entry.is_valid and entry.depth > depth)
		{
			return entry.score;
		}
		else if (blue)
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
			int score = (5 * no_spawn_score + avg_spawn_score) / 6;
			insert_transposition(hash, score, depth);
			return score;
		}
		else if (depth > 0)
		{
			int score = std::numeric_limits<int>::min();
			for (Move move : state.get_legal_moves(player))
			{
				state.commit(move);
				score = std::max(score, -evaluate(opponent(player), true, depth - 1, -beta, -alpha));
				state.uncommit(move);
				if (score >= beta)
				{
					break;
				}
				alpha = std::max(alpha, score);
			}
			insert_transposition(hash, score, depth);
			return score;
		}
		else
		{
			return heuristic(player);
		}
	}

	int heuristic(Cell player)
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
				else if (state.get(row, col) == opponent(player))
				{
					--score;
				}
			}
		}
		insert_transposition(state.hash(), score, 0);
		return score * 100;
	}

	struct TranspositionTableEntry
	{
		bool is_valid;
		int depth;
		int score;
	};

	void insert_transposition(std::uint64_t key, int score, int depth)
	{
		auto &entry = _transposition_table[key % _transposition_table_size];
		entry.is_valid = true;
		entry.depth = depth;
		entry.score = score;
	}

	TranspositionTableEntry &get_transposition_entry(std::uint64_t key)
	{
		return _transposition_table[key % _transposition_table_size];
	}

	GameState state;

	std::size_t _transposition_table_size;
	std::unique_ptr<TranspositionTableEntry[]> _transposition_table;
};

} // namespace flit