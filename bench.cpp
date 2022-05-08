#include <vector>
#include <string>
#include <functional>

#include <fmt/format.h>
#include <benchmark/benchmark.h>

#include "load.h"
#include "libpng.h"

using namespace rpng;
using namespace std::filesystem;

using load_fn_t = std::function<std::vector<uint8_t>(std::string)>;
using tests_t = std::vector<std::pair<std::string, load_fn_t>>;

void RunDecodeTest(
	benchmark::State & state,
	load_fn_t func,
	std::string filepath
) {
	for(auto _ : state) {
		func(filepath);
	}
}

std::vector<path> pngsuite() {
	std::vector<path> pngs;
	path pngdir("resources/pngsuite");
	for(auto const & dentry : directory_iterator(pngdir)) {
		if(!dentry.is_regular_file()) continue;
		if(dentry.path().extension().string() != ".png") continue;
		if(dentry.path().stem().string().starts_with('x')) continue;

		pngs.push_back(dentry.path());
	}
	return pngs;
}

void RegisterDecodeTests(tests_t tests) {
	auto pngs = pngsuite();
	std::sort(pngs.begin(), pngs.end(), [](auto const & a, auto const & b) {
		std::string as = a.stem().string(), bs = b.stem().string();
		return std::lexicographical_compare(
			as.begin(), as.end(),
			bs.begin(), bs.end()
		);
	});
	
	for(auto const & file : pngs) {
		std::string stem = file.stem();
		for(auto const & [prefix, func] : tests) {
			std::string testname = fmt::format("decode/{}/{}", stem, prefix);

			benchmark::RegisterBenchmark(
				testname.c_str(),
				RunDecodeTest,
				func,
				file.string()
			)->Unit(benchmark::kMicrosecond);
		}
	}
}

int main(int argc, char ** argv) {
	RegisterDecodeTests({
		{ "rpng", load },
		{ "libpng", decode }
	});

	benchmark::Initialize(&argc, argv);
	if(benchmark::ReportUnrecognizedArguments(argc, argv)) {
		return 1;
	} else {
		benchmark::RunSpecifiedBenchmarks();
	}
}
