#pragma once

#include "pass.h"
#include "utils.h"

namespace rpng {

std::vector<uint8_t> deinterlace_aligned(
	uint8_t const * src,
	int width, int height, int stride
) {
	std::vector<uint8_t> out(width * height * stride);
	for(int p = 1; p <= 7; p++) {
		interlace_pass_t const & pass = interlace_passes[p];
		for(int y = pass.offset_y; y < height; y += pass.period_y) {
			for(int x = pass.offset_x; x < width; x += pass.period_x) {
				for(int k = 0; k < stride; k++) {
					out[(y * width + x) * stride + k] = *(src++);
				}
			}
		}
	}
	return out;
}

std::vector<uint8_t> deinterlace_unaligned(
	uint8_t const * src,
	int width, int height, int stride_bits
) {
	int bytes_per_row = (width * stride_bits + 7) / 8;
	int samples_per_byte = 8 / stride_bits;

	std::vector<uint8_t> out(height * bytes_per_row);
	for(int p = 1; p <= 7; p++) {
		interlace_pass_t const & pass = interlace_passes[p];

		int pass_width = count(width, pass.offset_x, pass.period_x);
		int bytes_per_pass_row = (pass_width * stride_bits + 7) / 8;

		for(int y = pass.offset_y; y < height; y += pass.period_y) {
			int x = pass.offset_x;
			for(int b = 0; b < bytes_per_pass_row; b++, src++) {
				for(int s = 0; s < samples_per_byte; s++, x += pass.period_x) {
					bitcpy_unaligned(
						&out[y * bytes_per_row + (x * stride_bits / 8)],
						*src,
						(x * stride_bits) % 8,
						s * stride_bits,
						stride_bits
					);
				}
			}
		}
	}
	return out;
}

std::vector<uint8_t> deinterlace(
	std::vector<uint8_t> const & in,
	chunk_ihdr_data_t const & ihdr,
	colour_properties_t const & colours
) {
	int stride_bits	= ihdr.bit_depth * colours.num_channels;
	int stride = (stride_bits + 7) / 8;
	return stride_bits >= 8
		? deinterlace_aligned(in.data(), ihdr.width, ihdr.height, stride)
		: deinterlace_unaligned(in.data(), ihdr.width, ihdr.height, stride_bits);
}

}
