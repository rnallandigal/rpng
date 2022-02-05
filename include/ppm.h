#pragma once

#include <fstream>
#include <filesystem>
#include <vector>
#include <string>

#include "ihdr.h"

namespace rpng {

void save_image(
	std::string const & filepath,
	std::vector<uint8_t> const & img,
	chunk_ihdr_data_t const & ihdr_data
) {
	int channels = 4, byte_depth = ihdr_data.bit_depth / 8;
	int stride = channels * byte_depth;
	int range = (1 << ihdr_data.bit_depth) - 1;

	std::ofstream ofs(filepath);

	ofs << "P3\n";
	ofs << fmt::format("{} {}\n", (int)ihdr_data.width, (int)ihdr_data.height);
	ofs << fmt::format("{}\n", range);
	for(int i = 0; i < (int)img.size(); i += stride) {
		int64_t r = 0, g = 0, b = 0, a = 0;
		for(int j = 0; j < byte_depth; j++) {
			r = (r << 8) | img[i + j + 0 * byte_depth];
			g = (g << 8) | img[i + j + 1 * byte_depth];
			b = (b << 8) | img[i + j + 2 * byte_depth];
			a = (a << 8) | img[i + j + 3 * byte_depth];
		}
		ofs << fmt::format(
			"{} {} {}\n",
			std::to_string(r * a / range),
			std::to_string(g * a / range),
			std::to_string(b * a / range)
		);
	}
}

}
