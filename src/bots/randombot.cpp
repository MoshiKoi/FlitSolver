module;

#include <random>
#include <ranges>
#include <vector>

export module flit.bots.randombot;

import flit.bots.base;

namespace flit::bots
{

export class RandomBot : public Bot
{
  public:
	Move choose_move(GameState game) override
	{
		std::vector<Move> moves = game.get_legal_moves() | std::ranges::to<std::vector>();
		std::uniform_int_distribution<std::size_t> dist{0, moves.size() - 1};
		return moves[dist(engine)];
	}

  private:
	std::mt19937 engine{};
};

} // namespace flit::bots