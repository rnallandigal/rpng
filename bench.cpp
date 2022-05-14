#include <vector>
#include <string>
#include <functional>

#include <fmt/format.h>
#include <benchmark/benchmark.h>

#include "load.h"
#include "libpng.h"

using namespace rpng;
using namespace std;
using namespace filesystem;

using load_fn_t = function<vector<uint8_t>(string)>;
using tests_t = vector<pair<string, load_fn_t>>;

void RunDecodeTest(benchmark::State & state, load_fn_t func, string filepath) {
	for(auto _ : state) {
		func(filepath);
	}
}

void RunParseTest(benchmark::State & state, string filepath) {
	for(auto _ : state) {
		ifstream ifs(filepath, ios::binary);
		parse_png_header(ifs);
		parse_ihdr(ifs);
		pack_idat_chunks(ifs);
	}
}

void RunInflateTest(benchmark::State & state, vector<uint8_t> packed) {
	for(auto _ : state) {
		inflate(packed);
	}
}

void RunReconstructTest(
	benchmark::State & state,
	vector<uint8_t> filtered,
	chunk_ihdr_data_t ihdr_data,
	colour_properties_t colours
) {
	for(auto _ : state) {
		reconstruct(filtered, ihdr_data, colours);
	}
}

void RunDeinterlaceTest(
	benchmark::State & state,
	vector<uint8_t> reconstructed,
	chunk_ihdr_data_t ihdr_data,
	colour_properties_t colours
) {
	for(auto _ : state) {
		deinterlace(reconstructed, ihdr_data, colours);
	}
}

vector<path> pngsuite() {
	vector<path> pngs;
	path pngdir("resources/pngsuite");
	for(auto const & dentry : directory_iterator(pngdir)) {
		if(!dentry.is_regular_file()) continue;
		if(dentry.path().extension().string() != ".png") continue;
		if(dentry.path().stem().string().starts_with('x')) continue;

		pngs.push_back(dentry.path());
	}
	return pngs;
}

void RegisterTests(tests_t tests) {
	auto pngs = pngsuite();
	sort(pngs.begin(), pngs.end(), [](auto const & a, auto const & b) {
		string as = a.stem().string(), bs = b.stem().string();
		return lexicographical_compare(
			as.begin(), as.end(),
			bs.begin(), bs.end()
		);
	});

	for(auto const & file : pngs) {
		string stem = file.stem();
		for(auto const & [prefix, func] : tests) {
			string testname = fmt::format("decode/{}/{}", stem, prefix);

			benchmark::RegisterBenchmark(
				testname.c_str(),
				RunDecodeTest,
				func,
				file.string()
			)->Unit(benchmark::kMicrosecond);
		}

		string prefix = fmt::format("decode/{}/rpng", stem);

		// Load file into memory and combine chunks
		string testname_parse = fmt::format("{}/parse", prefix);
		benchmark::RegisterBenchmark(
			testname_parse.c_str(),
			RunParseTest,
			file.string()
		)->Unit(benchmark::kMicrosecond);

		auto [ihdr_data, colours, packed] = [](string filepath) {
			ifstream ifs(filepath, ios::binary);
			parse_png_header(ifs);
			auto [ihdr_data, colours] = parse_ihdr(ifs);
			vector<uint8_t> packed = pack_idat_chunks(ifs); 
			return tuple(ihdr_data, colours, packed);
		}(file.string());

		// inflate packed datastream
		string testname_inflate = fmt::format("{}/inflate", prefix);
		benchmark::RegisterBenchmark(
			testname_inflate.c_str(),
			RunInflateTest,
			packed
		)->Unit(benchmark::kMicrosecond);
		auto filtered = inflate(packed);

		// reconstruct filtered datastream
		string testname_reconstruct = fmt::format("{}/reconstruct", prefix);
		benchmark::RegisterBenchmark(
			testname_reconstruct.c_str(),
			RunReconstructTest,
			filtered,
			ihdr_data,
			colours
		)->Unit(benchmark::kMicrosecond);
		auto reconstructed = reconstruct(filtered, ihdr_data, colours);

		if(!ihdr_data.interlace) continue;

		// deinterlace reconstructed datastream
		string testname_deinterlace = fmt::format("{}/deinterlace", prefix);
		benchmark::RegisterBenchmark(
			testname_deinterlace.c_str(),
			RunDeinterlaceTest,
			reconstructed,
			ihdr_data,
			colours
		)->Unit(benchmark::kMicrosecond);
	}
}

int main(int argc, char ** argv) {
	RegisterTests({
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
