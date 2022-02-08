#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include "load.h"
#include "libpng.h"

namespace rpng {

class ParseTest : public testing::Test {
private:
	std::string filepath;
	bool should_fail;

	void test_success() {
		EXPECT_NO_THROW(load(filepath)) << filepath;
	}

	void test_failure() {
		EXPECT_ANY_THROW(load(filepath)) << filepath;
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

		fmt::print("Checking {}, result: {}\n", filepath, a == b);
		EXPECT_EQ(a, b);
	}
};

}
