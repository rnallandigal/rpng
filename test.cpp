#include <gtest/gtest.h>

#include "load_test.h"

void register_tests() {
	using namespace std::filesystem;
	for(auto dentry : directory_iterator(path("./resources/pngsuite"))) {
		if(!dentry.is_regular_file()) continue;
		if(dentry.path().extension().string() != ".png") continue;

		std::string filepath = dentry.path().string();
		bool should_fail = dentry.path().filename().string().starts_with('x');

		testing::RegisterTest(
			"ParseTest",
			path(filepath).filename().string().c_str(),
			nullptr,
			nullptr,
			__FILE__,
			__LINE__,
			[=]() { return new rpng::ParseTest(filepath, should_fail); }
		);

		if(should_fail) continue;

		testing::RegisterTest(
			"DecodeTest",
			path(filepath).filename().string().c_str(),
			nullptr,
			nullptr,
			__FILE__,
			__LINE__,
			[=]() { return new rpng::DecodeTest(filepath); }
		);
	}
}

int main(int argc, char **argv) {
	spdlog::set_pattern("%^[%L]%$ %v");
	spdlog::set_level(spdlog::level::trace);

	::testing::InitGoogleTest(&argc, argv);
	register_tests();
	return RUN_ALL_TESTS();
}
