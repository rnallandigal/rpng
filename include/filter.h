#pragma once

#include <vector>
#include <cmath>

#include "chunk/ihdr.h"
#include "pass.h"

namespace rpng {

uint8_t paeth(uint8_t a, uint8_t b, uint8_t c) {
    int16_t p = (int16_t)a + b - c;
    uint16_t pa = abs(p - a);
    uint16_t pb = abs(p - b);
    uint16_t pc = abs(p - c);
	return (pa <= pb && pa <= pc) ? a : (pb <= pc ? b : c);
}


uint8_t filter_none(uint8_t x, uint8_t a, uint8_t b, uint8_t c) {
	return x;
}

uint8_t filter_sub(uint8_t x, uint8_t a, uint8_t b, uint8_t c) {
	return x - a;
}

uint8_t filter_up(uint8_t x, uint8_t a, uint8_t b, uint8_t c) {
	return x - b;
}

uint8_t filter_avg(uint8_t x, uint8_t a, uint8_t b, uint8_t c) {
	return x - (uint8_t)(((uint16_t)a + b) / 2);
}

uint8_t filter_paeth(uint8_t x, uint8_t a, uint8_t b, uint8_t c) {
	return x - paeth(a, b, c);
}

uint8_t recon_none(uint8_t x, uint8_t a, uint8_t b, uint8_t c) {
	return x;
}

uint8_t recon_sub(uint8_t x, uint8_t a, uint8_t b, uint8_t c) {
	return x + a;
}

uint8_t recon_up(uint8_t x, uint8_t a, uint8_t b, uint8_t c) {
	return x + b;
}

uint8_t recon_avg(uint8_t x, uint8_t a, uint8_t b, uint8_t c) {
	return x + (uint8_t)(((uint16_t)a + b) / 2);
}

uint8_t recon_paeth(uint8_t x, uint8_t a, uint8_t b, uint8_t c) {
	return x + paeth(a, b, c);
}

using filter_fn_t = uint8_t(*)(uint8_t, uint8_t, uint8_t, uint8_t);
std::array<filter_fn_t, 5> filter_fn {
	filter_none, filter_sub, filter_up, filter_avg, filter_paeth
};
std::array<filter_fn_t, 5> recon_fn {
	recon_none, recon_sub, recon_up, recon_avg, recon_paeth
};

void reconstruct_slice(
	uint8_t * dst, uint8_t const * src,
	int stride, int bytes_per_row, int height
) {
	// first pixel
	filter_fn_t R = recon_fn[*(src++)];
	for(int i = 0; i < stride; i++)
		*(dst++) = R(*(src++), 0, 0, 0);

	// first scanline
	for(int i = stride; i < bytes_per_row; i++, src++, dst++) {
		*dst = R(*src, *(dst - stride), 0, 0);
	}

	// remaining scanlines
	for(int i = 1; i < height; i++) {
		R = recon_fn[*(src++)];

		// first stride bytes
		for(int j = 0; j < stride; j++, src++, dst++)
			*dst = R(*src, 0, *(dst - bytes_per_row), 0);

		// remaining bytes
		for(int j = stride; j < bytes_per_row; j++, src++, dst++) {
			*dst = R(
				*src,
				*(dst - stride),
				*(dst - bytes_per_row),
				*(dst - stride - bytes_per_row)
			);
		}
	}
}

std::vector<uint8_t> reconstruct(
	std::vector<uint8_t> const & in,
	chunk_ihdr_data_t const & ihdr,
	colour_properties_t const & colours
) {
	int stride_bits	= ihdr.bit_depth * colours.num_channels;
	int stride = (stride_bits + 7) / 8;

	// compute reconstructed image size and reduced image dimensions
	int first_pass = ihdr.interlace ? 1 : 0;
	int last_pass = ihdr.interlace ? 7 : 0;

	int reconstructed_size = 0;
	std::vector<std::pair<int, int>> reduced_image_dims;
	for(int p = first_pass; p <= last_pass; p++) {
		interlace_pass_t const & pass = interlace_passes[p];

		int pass_width = count(ihdr.width, pass.offset_x, pass.period_x);
		int pass_height = count(ihdr.height, pass.offset_y, pass.period_y);
		int bytes_per_pass_row = (pass_width * stride_bits + 7) / 8;

		if(!pass_width || !pass_height) continue;

		reduced_image_dims.emplace_back(bytes_per_pass_row, pass_height);
		reconstructed_size += pass_height * bytes_per_pass_row;
	}

	// reconstruct reduced images
	std::vector<uint8_t> reconstructed_image(reconstructed_size);
	uint8_t * dst = reconstructed_image.data();
	uint8_t const * src = in.data();

	for(auto [bytes_per_row, height] : reduced_image_dims) {
		reconstruct_slice(dst, src, stride, bytes_per_row, height);
		dst += height * bytes_per_row;
		src += height * (1 + bytes_per_row);
	}
	return reconstructed_image;
}

}
