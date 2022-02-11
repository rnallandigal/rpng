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
		fclose(file);
		throw std::runtime_error("Couldn't create a png info struct");
	}

	png_init_io(png, file);
	png_read_png(png, info, PNG_TRANSFORM_IDENTITY, nullptr);

	png_uint_32 width = png_get_image_width(png, info);
	png_uint_32 height = png_get_image_height(png, info);

	png_byte num_channels = png_get_channels(png, info);
	png_byte bit_depth = png_get_bit_depth(png, info);

	int bits_per_row = width * num_channels * bit_depth;
	int bytes_per_row = (bits_per_row + 7) / 8;

	uint8_t final_byte_mask = 0xff;
 	if(bits_per_row % 8) {
		final_byte_mask	= ~(0xff >> (bits_per_row % 8));
	}

	png_byte ** rows = png_get_rows(png, info);

	std::vector<uint8_t> img;
	for(uint32_t i = 0; i < height; i++) {
		for(int j = 0; j < bytes_per_row - 1; j++) {
			img.push_back(rows[i][j]);
		}
		img.push_back(rows[i][bytes_per_row - 1] & final_byte_mask);
	}

	png_destroy_read_struct(&png, &info, nullptr);
	fclose(file);
	return img;
}
