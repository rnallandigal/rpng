#pragma once

#include <vector>
#include <cmath>

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
	//return x + (uint8_t)(((uint16_t)a + b) / 2);
	uint16_t y = (uint16_t)a + b;
	return x + y / 2;
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

std::vector<uint8_t> reconstruct(std::vector<uint8_t> const & in, int w, int stride) {
	std::vector<uint8_t> ret;

	int i = 0;
	filter_fn_t R = recon_fn[in[i++]];

	while((int)ret.size() % w < stride) ret.push_back(R(in[i++], 0, 0, 0));
	while(ret.size() % w != 0) {
		ret.push_back(R(in[i++], ret[ret.size() - stride], 0, 0));
	}

	while(i < (int)in.size()) {
		R = recon_fn[in[i++]];

		while((int)ret.size() % w < stride)
			ret.push_back(R(in[i++], 0, ret[ret.size() - w], 0));

		while(ret.size() % w != 0) {
			ret.push_back(R(
				in[i++],
				ret[ret.size() - stride],
				ret[ret.size() - w],
				ret[ret.size() - stride - w]
			));
		}
	}
	return ret;
}

}
