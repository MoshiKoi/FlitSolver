module;

#include <functional>
#include <map>
#include <memory>
#include <string>

export module flit.bots;

export import flit.bots.base;
import flit.bots.randombot;

namespace flit::bots
{
export std::map<std::string, std::function<std::unique_ptr<Bot>()>> const bots{
	{"Random Bot", [] { return std::make_unique<RandomBot>(); }},
};

} // namespace flit::bots