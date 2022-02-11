#include <fmt/format.h>
#include <docopt/docopt.h>

#include "load.h"
#include "libpng.h"

using namespace rpng;

const char COMMAND[] =
R"(rpng
Load a file in Portable Network Graphics (PNG) format.

Usage:   rpng load (--file FILE)
         rpng baseline (--file FILE)
         rpng diff (--file FILE)

Options:
    -f, --file FILE         The path to the PNG file to load.
    -h, --help              Show this screen.
)";

int main(int argc, char ** argv) {
	auto args = docopt::docopt(COMMAND, { argv + 1, argv + argc }, true, "");
	if(args["load"].asBool()) {
		load(args["--file"].asString());
	} else if(args["baseline"].asBool()) {
		decode(args["--file"].asString());
	} else if(args["diff"].asBool()) {
		auto rpng_out = load(args["--file"].asString());
		auto libpng_out = decode(args["--file"].asString());

		fmt::print("rpng size: {}\n", rpng_out.size());
		fmt::print("libpng size: {}\n", libpng_out.size());

		int diff_bytes = 0;
		for(int i = 0; i < (int)std::min(rpng_out.size(), libpng_out.size()); i++) {
			if(rpng_out[i] != libpng_out[i]) {
				fmt::print(
					"differs at byte {:5}: {:02x} {:02x}\n",
					i,
					rpng_out[i],
					libpng_out[i]
				);
				diff_bytes++;
			}
		}
		if(diff_bytes) fmt::print("Images differ in {} bytes\n", diff_bytes);
		else fmt::print("No differences found\n");
	} else {
		throw std::runtime_error(argc > 0
			? fmt::format("Unable to interpret command: {}", argv[0])
			: fmt::format("Unable to interpret command")
		);
	}
	return 0;
}
