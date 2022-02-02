#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include "load.h"

namespace rpng {

class LoadTest : public testing::Test {
private:
	std::string filepath;
	bool should_fail;

public:
	LoadTest(std::string const & filepath, bool should_fail)
		: filepath(filepath), should_fail(should_fail) {}

	void TestBody() override {
		if(should_fail) {
			EXPECT_THROW(load(filepath), std::runtime_error) << filepath;
		} else {
			EXPECT_NO_THROW(load(filepath)) << filepath;
		}
	}
};

}

void register_load_tests() {
	using namespace std::filesystem;
	for(auto dentry : directory_iterator(path("./resources/pngsuite"))) {
		if(!dentry.is_regular_file()) continue;
		if(dentry.path().extension().string() != ".png") continue;

		std::string filepath = dentry.path().string();
		bool should_fail = dentry.path().filename().string().starts_with('x');

		testing::RegisterTest(
			"LoadTest",
			path(filepath).filename().string().c_str(),
			nullptr,
			nullptr,
			__FILE__,
			__LINE__,
			[=]() { return new rpng::LoadTest(filepath, should_fail); }
		);
	}
}


