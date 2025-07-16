export module flit.bots.base;

export import flit.game;

namespace flit::bots
{

export class Bot
{
  public:
	virtual ~Bot() = default;
	virtual Move choose_move(GameState game) = 0;
};

} // namespace flit::bots