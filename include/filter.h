#pragma once

#include <vector>
#include <cmath>

#include "netpbm.h"
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

struct interlace_pass_t {
	int offset_x, period_x;
	int offset_y, period_y;
};

constexpr std::array<interlace_pass_t, 9> interlace_passes {{
	{ 0, 1, 0, 1 },		// pass 0 (no interlace)
	{ 0, 8, 0, 8 },
	{ 4, 8, 0, 8 },
	{ 0, 4, 4, 8 },
	{ 2, 4, 0, 4 },
	{ 0, 2, 2, 4 },
	{ 1, 2, 0, 2 },
	{ 0, 1, 1, 2 }
}};

/**
 * input : interlaced, filtered scanlines
 * output: deinterlaced, reconstructed, packed image
 * stride bits: 1, 2, 4, 8, 16, 24, 32, 48, 64
 * 	-> next source byte always starts at sample boundary
 *	-> samples never straddle a byte boundary
 */
std::vector<uint8_t> reconstruct_interlaced(
	std::vector<uint8_t> const & in,
	chunk_ihdr_data_t const & ihdr,
	colour_properties_t const & colours
) {
	static auto count = [](int length, int offset, int period) -> int {
		return (length - offset + period - 1) / period;
	};

	std::vector<uint8_t> interlaced;

	int stride_bits	= ihdr.bit_depth * colours.num_channels;
	int stride = (stride_bits + 7) / 8;
	int bytes_per_row = (ihdr.width * stride_bits + 7) / 8;

	uint8_t const * curr = in.data();
	for(int p = 1; p <= 7; p++) {
		interlace_pass_t const & pass = interlace_passes[p];

		int pass_width = count(ihdr.width, pass.offset_x, pass.period_x);
		int pass_height = count(ihdr.height, pass.offset_y, pass.period_y);
		int bytes_per_pass_row = (pass_width * stride_bits + 7) / 8;

		if(!pass_height || !pass_width) continue;

		int sz = interlaced.size();
		interlaced.resize(interlaced.size() + pass_height * bytes_per_pass_row);
		uint8_t * dest = &interlaced[sz];

		// first pixel
		filter_fn_t R = recon_fn[*(curr++)];
		for(int i = 0; i < stride; i++)
			*(dest++) = R(*(curr++), 0, 0, 0);

		// first scanline
		for(int i = stride; i < bytes_per_pass_row; i++, curr++, dest++) {
			*dest = R(*curr, *(dest - stride), 0, 0);
		}

		// remaining scanlines
		for(int i = 1; i < pass_height; i++) {
			R = recon_fn[*(curr++)];

			// first stride bytes
			for(int j = 0; j < stride; j++, curr++, dest++)
				*dest = R(*curr, 0, *(dest - bytes_per_pass_row), 0);

			// remaining bytes
			for(int j = stride; j < bytes_per_pass_row; j++, curr++, dest++) {
				*dest = R(*curr, *(dest - stride), *(dest - bytes_per_pass_row), *(dest - stride - bytes_per_pass_row));
			}
		}

		// test save img
		/*
		chunk_ihdr_data_t reduced_ihdr = ihdr;
		reduced_ihdr.width = pass_width;
		reduced_ihdr.height = pass_height;

		std::vector<uint8_t> reduced_img(pass_height * bytes_per_pass_row);
		std::copy(dest - reduced_img.size(), dest, reduced_img.data());

		save_greyscale(fmt::format("out/basi0g01_{}.pgm", p), reduced_img, reduced_ihdr);
		*/
	}

	auto bitcpy_unaligned = [](uint8_t * dst, uint8_t src, int doff, int soff, int n) {
		uint8_t src_aligned = (src << soff) >> doff;

		// TODO: review all usages of right shift (>>) for SAR vs SHL
		uint8_t mask = ((uint8_t)(~(0xff >> n))) >> doff;
		*dst = *dst ^ ((*dst ^ src_aligned) & mask);
	};

	// rearrange interlaced image
	std::vector<uint8_t> out(ihdr.height * bytes_per_row);
	uint8_t * src = interlaced.data();

	if(stride_bits < 8) {
		int samples_per_byte = 8 / stride_bits;
		for(int p = 1; p <= 7; p++) {
			interlace_pass_t const & pass = interlace_passes[p];

			int pass_width = count(ihdr.width, pass.offset_x, pass.period_x);
			int bytes_per_pass_row = (pass_width * stride_bits + 7) / 8;

			for(int y = pass.offset_y; y < (int)ihdr.height; y += pass.period_y) {
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
	} else {
		for(int p = 1; p <= 7; p++) {
			interlace_pass_t const & pass = interlace_passes[p];

			for(int y = pass.offset_y; y < (int)ihdr.height; y += pass.period_y) {
				for(int x = pass.offset_x; x < (int)ihdr.width; x += pass.period_x) {
					for(int k = 0; k < stride; k++) {
						out[y * bytes_per_row + stride * x + k] = *(src++);
					}
				}
			}
		}
	}

	return out;
}

}
