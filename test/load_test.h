#include <filesystem>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include "load.h"
#include "libpng.h"

namespace rpng {

class ParseTest : public testing::Test {
private:
	std::string filepath;
	bool should_fail;

	void test_success() {
		EXPECT_EXIT(
			(load(filepath), exit(0)),
			testing::ExitedWithCode(0),
			".*"
		) << filepath;
	}

	void test_failure() {
		EXPECT_EXIT(
			{
				try {
					load(filepath);
				} catch(std::runtime_error const & e) {
					exit(0);
				}
				exit(1);
			},
			testing::ExitedWithCode(0),
			".*"
		) << filepath;
	}

public:
	ParseTest(std::string const & filepath, bool should_fail)
		: filepath(filepath), should_fail(should_fail) {}

	void TestBody() override {
		should_fail ? test_failure() : test_success(); 
	}
};

class DecodeTest : public testing::Test {
private:
	std::string filepath;

public:
	DecodeTest(std::string const & filepath) : filepath(filepath) {}

	void TestBody() override {
		auto a = load(filepath);
		auto b = decode(filepath);

		SPDLOG_DEBUG("Checking {}, result: {}\n", filepath, a == b);
		EXPECT_EQ(a, b);
	}
};

}
