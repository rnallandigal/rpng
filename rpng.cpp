#include <fmt/format.h>
#include <docopt/docopt.h>

#include "load.h"

using namespace rpng;

const char COMMAND[] =
R"(rpng
Load a file in Portable Network Graphics (PNG) format.

Usage:   rpng load (--file FILE)

Options:
    -f, --file FILE         The path to the PNG file to load.
    -h, --help              Show this screen.
)";

int main(int argc, char ** argv) {
	auto args = docopt::docopt(COMMAND, { argv + 1, argv + argc }, true, "");
	if(args["load"].asBool()) {
		load(args["--file"].asString());
	} else {
		throw std::runtime_error(argc > 0
			? fmt::format("Unable to interpret command: {}", argv[0])
			: fmt::format("Unable to interpret command")
		);
	}
	return 0;
}
