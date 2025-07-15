module;

#include <libassert/assert.hpp>

#include <algorithm>
#include <limits>
#include <memory>
#include <random>
#include <ranges>
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

	std::vector<solve_result> solve(Cell player, int depth)
	{
		std::vector<solve_result> evaluations;
		for (Move move : state.get_legal_moves(player))
		{
			state.commit(move);
			evaluations.emplace_back(move, -evaluate(opponent(player), true, depth, -100000, 100000));
			state.uncommit(move);
		}
		std::ranges::sort(evaluations, [](auto const &a, auto const &b) { return a.score > b.score; });
		return evaluations;
	}

  private:
	int evaluate(Cell player, bool blue, int depth, int alpha, int beta)
	{
		int const original_alpha = alpha;
		int const original_beta = beta;
		auto hash = state.hash();
		auto &entry = get_transposition_entry(hash);
		if (not blue and entry.is_valid and entry.depth >= depth)
		{
			switch (entry.bound)
			{
			case TranspositionBound::Exact: return entry.score;
			case TranspositionBound::LowerBound:
				if (entry.score >= beta)
				{
					return entry.score;
				}
				else
				{
					break;
				}
			case TranspositionBound::UpperBound:
				if (entry.score <= alpha)
				{
					return entry.score;
				}
				else
				{
					break;
				}
			default: LIBASSERT_UNREACHABLE();
			}
		}
		if (blue)
		{
			// int total_spawn_score = 0;
			// int count = 0;
			// auto possible_spawns = state.get_possible_spawns() | std::ranges::to<std::vector>();
			// for (int i = 0; i < possible_spawns.size() and i < 10; ++i)
			// {
			// 	std::ranges::swap(possible_spawns[i], possible_spawns[rand() % (possible_spawns.size() - i) + i]);
			// 	auto idx = possible_spawns[i];
			// 	state.set(idx, Cell::Blue);
			// 	total_spawn_score += evaluate(player, false, depth, alpha, beta);
			// 	++count;
			// 	state.unset(idx);
			// }
			// int avg_spawn_score = total_spawn_score / count;
			int no_spawn_score = evaluate(player, false, depth, alpha, beta);
			// There is a 1/6 chance of actually having a spawn
			// int score = (5 * no_spawn_score + avg_spawn_score) / 6;
			int score = no_spawn_score;
			// entry.is_valid = true;
			// entry.bound = TranspositionBound::Exact;
			// entry.score = score;
			// entry.depth = depth;
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
			entry.bound = (score <= original_alpha) //
				? TranspositionBound::UpperBound
				: (score >= original_beta) //
					? TranspositionBound::LowerBound
					: entry.bound = TranspositionBound::Exact;
			entry.is_valid = true;
			entry.score = score;
			entry.depth = depth;
			return score;
		}
		else
		{
			int score = state.heuristic(player);
			entry.bound = TranspositionBound::Exact;
			entry.is_valid = true;
			entry.score = score;
			entry.depth = 0;
			return score;
		}
	}

	enum class TranspositionBound : std::uint8_t
	{
		Exact,
		LowerBound,
		UpperBound,
	};

	struct TranspositionTableEntry
	{
		bool is_valid;
		TranspositionBound bound;
		int depth;
		int score;
	};

	TranspositionTableEntry &get_transposition_entry(std::uint64_t key)
	{
		return _transposition_table[key % _transposition_table_size];
	}

	GameState state;
	std::minstd_rand _engine{};
	std::size_t _transposition_table_size;
	std::unique_ptr<TranspositionTableEntry[]> _transposition_table;
};

} // namespace flit