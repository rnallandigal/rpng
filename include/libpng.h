#pragma once

#include <cstdio>
#include <vector>
#include <string>

#include <fmt/format.h>
#include <png.h>

std::vector<uint8_t> decode(std::string const & filepath) {
	FILE * file = fopen(filepath.c_str(), "rb");
	if(!file)
		throw std::runtime_error(fmt::format("Cannot open file: {}", filepath));

	png_struct * png = png_create_read_struct(
		PNG_LIBPNG_VER_STRING,
		nullptr,
		nullptr,
		nullptr
	);
	if(!png) throw std::runtime_error("Couldn't create a png struct");

	png_info * info = png_create_info_struct(png);
	if(!info) {
		png_destroy_read_struct(&png, nullptr, nullptr);
		throw std::runtime_error("Couldn't create a png info struct");
	}

	png_init_io(png, file);
	png_read_png(png, info, PNG_TRANSFORM_IDENTITY, nullptr);

	png_byte channels = png_get_channels(png, info);
	png_byte bit_depth = png_get_bit_depth(png, info);
	png_uint_32 width = png_get_image_width(png, info);
	png_uint_32 height = png_get_image_height(png, info);

	uint32_t bytes_per_row = ((uint32_t) channels * bit_depth * width + 7) / 8;

	png_byte ** rows = png_get_rows(png, info);

	std::vector<uint8_t> img;
	for(uint32_t i = 0; i < height; i++) {
		for(uint32_t j = 0; j < bytes_per_row; j++) {
			img.push_back(rows[i][j]);
		}
	}

	png_destroy_read_struct(&png, &info, nullptr);
	return img;
}
