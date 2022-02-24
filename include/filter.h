#pragma once

#include <vector>
#include <cmath>

#include "chunk/ihdr.h"

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

std::vector<uint8_t> reconstruct(
	std::vector<uint8_t> const & in,
	chunk_ihdr_data_t const & ihdr,
	colour_properties_t const & colors
) {
	int stride_bits	= ihdr.bit_depth * colors.num_channels;
	int stride = (stride_bits + 7) / 8;
	int bytes_per_row = (ihdr.width * stride_bits + 7) / 8;

	std::vector<uint8_t> out(ihdr.height * bytes_per_row);
	uint8_t const * curr = in.data();
	uint8_t * dest = out.data();

	// first stride bytes
	filter_fn_t R = recon_fn[*(curr++)];
	for(int i = 0; i < stride; i++)
		*(dest++) = R(*(curr++), 0, 0, 0);

	// first scanline
	for(int i = stride; i < bytes_per_row; i++, curr++, dest++) {
		*dest = R(*curr, *(dest - stride), 0, 0);
	}

	// remaining scanlines
	for(int i = 1; i < (int)ihdr.height; i++) {
		R = recon_fn[*(curr++)];

		// first stride bytes
		for(int j = 0; j < stride; j++, curr++, dest++)
			*dest = R(*curr, 0, *(dest - bytes_per_row), 0);

		// remaining bytes
		for(int j = stride; j < bytes_per_row; j++, curr++, dest++) {
			*dest = R(*curr, *(dest - stride), *(dest - bytes_per_row), *(dest - stride - bytes_per_row));
		}
	}
	return out;
}

}
