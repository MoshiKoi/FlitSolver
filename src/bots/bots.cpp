module;

#include <functional>
#include <map>
#include <memory>
#include <string>

export module flit.bots;

export import flit.bots.base;
import flit.bots.randombot;
import flit.bots.alphabetabot;

namespace flit::bots
{
export std::map<std::string, std::function<std::unique_ptr<Bot>()>> const bots{
	{"Random Bot", [] { return std::make_unique<RandomBot>(); }},
	{"Alpha-Beta Bot", [] { return std::make_unique<AlphaBetaBot>(); }},
};

} // namespace flit::bots