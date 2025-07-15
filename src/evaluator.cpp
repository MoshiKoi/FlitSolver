module;

#include <libassert/assert.hpp>

#include <algorithm>
#include <limits>
#include <memory>
#include <print>
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
	Solver(GameState state, std::size_t transposition_table_size = 1 << 25)
		: state{std::move(state)}, _transposition_table_size{transposition_table_size},
		  _transposition_table{std::make_unique<TranspositionTableEntry[]>(_transposition_table_size)}
	{
	}

	std::vector<solve_result> solve(Cell player, int depth)
	{
		std::vector<solve_result> evaluations;
		state.turn(player);
		for (Move move : state.get_legal_moves())
		{
			state.commit(move);
			evaluations.emplace_back(move, -evaluate(true, depth, -100000, 100000));
			state.uncommit(move);
		}
		std::ranges::sort(evaluations, [](auto const &a, auto const &b) { return a.score > b.score; });
		std::println("Evaluated nodes: {} ({} leaves)", _nodes, _leaf_nodes);
		std::println(
			"Transposition table hits: {} ({:.2f}%)",
			_transposition_table_hits,
			100.0 * static_cast<double>(_transposition_table_hits) / static_cast<double>(_nodes));
		return evaluations;
	}

  private:
	int evaluate(bool blue, int depth, int alpha, int beta)
	{
		++_nodes;
		int const original_alpha = alpha;
		int const original_beta = beta;
		auto hash = state.hash();
		auto &entry = get_transposition_entry(hash);
		if (not blue and entry.is_valid and entry.depth >= depth)
		{
			++_transposition_table_hits;
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
			int total_spawn_score = 0;
			int count = 0;
			auto possible_spawns = state.get_possible_spawns() | std::ranges::to<std::vector>();
			for (int i = 0; i < possible_spawns.size() and i < 10; ++i)
			{
				std::ranges::swap(possible_spawns[i], possible_spawns[rand() % (possible_spawns.size() - i) + i]);
				auto idx = possible_spawns[i];
				state.set(idx, Cell::Blue);
				total_spawn_score += evaluate(false, depth, alpha, beta);
				state.unset(idx);
				++count;
			}
			int avg_spawn_score = total_spawn_score / count;
			int no_spawn_score = evaluate(false, depth, alpha, beta);
			// There is a 1/6 chance of actually having a spawn
			int score = (5 * no_spawn_score + avg_spawn_score) / 6;
			return score;
		}
		else if (depth > 0)
		{
			int score = std::numeric_limits<int>::min();
			for (Move move : state.get_legal_moves())
			{
				state.commit(move);
				score = std::max(score, -evaluate(true, depth - 1, -beta, -alpha));
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
			++_leaf_nodes;
			int score = state.heuristic();
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
	std::size_t _nodes = 0;
	std::size_t _leaf_nodes = 0;
	std::size_t _transposition_table_hits = 0;
};

} // namespace flit