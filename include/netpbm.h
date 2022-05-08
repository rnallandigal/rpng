#pragma once

#include <fstream>
#include <filesystem>
#include <vector>
#include <string>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "chunk/ihdr.h"

namespace rpng {

void save_image(
	std::string const & filepath,
	std::vector<uint8_t> const & img,
	chunk_ihdr_data_t const & ihdr_data
) {
	SPDLOG_DEBUG("width     : {}\n", ihdr_data.width);
	SPDLOG_DEBUG("height    : {}\n", ihdr_data.height);
	SPDLOG_DEBUG("bit depth : {}\n", ihdr_data.bit_depth);
	SPDLOG_DEBUG("img size  : {}\n", img.size());

	int channels = 3, byte_depth = ihdr_data.bit_depth / 8;
	int stride = channels * byte_depth;
	int range = (1 << ihdr_data.bit_depth) - 1;

	std::ofstream ofs(filepath);

	ofs << "P3\n";
	ofs << fmt::format("{} {}\n", (int)ihdr_data.width, (int)ihdr_data.height);
	ofs << fmt::format("{}\n", range);
	for(int i = 0; i < (int)img.size(); i += stride) {
		int64_t r = 0, g = 0, b = 0;
		for(int j = 0; j < byte_depth; j++) {
			r = (r << 8) | img[i + 0 * byte_depth + j];
			g = (g << 8) | img[i + 1 * byte_depth + j];
			b = (b << 8) | img[i + 2 * byte_depth + j];
		}
		ofs << fmt::format(
			"{} {} {}\n",
			std::to_string(r),
			std::to_string(g),
			std::to_string(b)
		);
	}
}

void save_greyscale(
	std::string const & filepath,
	std::vector<uint8_t> const & img,
	chunk_ihdr_data_t const & ihdr_data
) {
	SPDLOG_DEBUG("width     : {}\n", ihdr_data.width);
	SPDLOG_DEBUG("height    : {}\n", ihdr_data.height);
	SPDLOG_DEBUG("bit depth : {}\n", ihdr_data.bit_depth);
	SPDLOG_DEBUG("img size  : {}\n", img.size());

	int range = (1 << ihdr_data.bit_depth) - 1;

	std::ofstream ofs(filepath);

	ofs << "P2\n";
	ofs << fmt::format("{} {}\n", (int)ihdr_data.width, (int)ihdr_data.height);
	ofs << fmt::format("{}\n", range);

	if(ihdr_data.bit_depth < 8) {
		int samples_per_byte = 8 / ihdr_data.bit_depth;
		int bits_per_row = ihdr_data.width * ihdr_data.bit_depth;

		for(int i = 0; i < (int)img.size(); i++) {
			uint8_t mask = ~(0xff >> ihdr_data.bit_depth);
			// TODO: buggy logic - need to properly handle last byte
			for(int j = 0; j < std::min(samples_per_byte, bits_per_row); j++) {
				ofs << fmt::format(
					"{}\n",
					(img[i] & (mask >> (j * ihdr_data.bit_depth))) >> ((samples_per_byte - j - 1) * ihdr_data.bit_depth)
				);
			}
		}
	} else {
		int bytes_per_sample = ihdr_data.bit_depth / 8;
		for(int i = 0; i < (int)img.size(); i += bytes_per_sample) {
			int sample = 0;
			for(int j = 0; j < bytes_per_sample; j++) {
				sample = (sample << 8) | img[i + j];
			}
			ofs << fmt::format("{}\n", sample);
		}
	}
}

}
