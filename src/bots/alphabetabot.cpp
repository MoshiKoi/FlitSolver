module;

#include <utility>

export module flit.bots.alphabetabot;

import flit.bots.base;
import flit.evaluator;

namespace flit::bots
{

export class AlphaBetaBot : public Bot
{
  public:
	Move choose_move(GameState game) override
	{
		Solver solver{std::move(game)};
		auto evaluations = solver.solve(game.turn(), 1);
		return evaluations[0].move;
	}

  private:
};

} // namespace flit::bots