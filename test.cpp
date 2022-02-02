#include <gtest/gtest.h>

#include "load_test.h"

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	register_load_tests();
	return RUN_ALL_TESTS();
}
