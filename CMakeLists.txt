cmake_minimum_required(VERSION 3.16)

set(MAIN rpng)
project(rpng)

# top level properties
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_USE_RELATIVE_PATHS True)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -Wno-unused-parameter -Wno-missing-field-initializers")

# functions
function(import_libraries)
	foreach(name IN LISTS ARGN)
		string(TOUPPER ${name} uppername)
		add_library(lib${name} UNKNOWN IMPORTED)
		find_library(${uppername}_LIB ${name} REQUIRED)
		set_target_properties(lib${name} PROPERTIES IMPORTED_LOCATION ${${uppername}_LIB})
	endforeach()
endfunction()

include_directories(BEFORE ${INCLUDE_DIR} ${TEST_DIR})
import_libraries(fmt spdlog docopt gtest pthread z png)
add_compile_definitions(
	SPDLOG_FMT_EXTERNAL
)

# main
add_executable(${MAIN} ${MAIN}.cpp)
target_link_libraries(${MAIN} docopt fmt spdlog z png)
target_compile_options(${MAIN} PRIVATE -O0)
target_compile_definitions(${MAIN}
	PRIVATE SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG
)

# test
add_executable(test test.cpp)
target_link_libraries(test fmt spdlog z png gtest pthread)
target_compile_options(test PRIVATE -O0)
target_compile_definitions(test
	PRIVATE SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG
)

# bench
add_executable(bench bench.cpp)
target_link_libraries(bench fmt spdlog z png pthread benchmark)
target_compile_options(bench PRIVATE -O3)
target_compile_definitions(bench
	PRIVATE SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_OFF
)

# coroutines
add_executable(coroutines test/coroutines.cpp)
target_link_libraries(coroutines fmt spdlog)
target_compile_options(coroutines PRIVATE -O0 -fcoroutines)
target_compile_definitions(coroutines
	PRIVATE SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG
)
