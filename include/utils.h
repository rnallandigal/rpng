#pragma once

namespace rpng {

void bitcpy_unaligned(uint8_t * dst, uint8_t src, int doff, int soff, int n) {
	uint8_t src_aligned = (src << soff) >> doff;

	// TODO: review all usages of right shift (>>) for SAR vs SHL
	uint8_t mask = ((uint8_t)(~(0xff >> n))) >> doff;
	*dst = *dst ^ ((*dst ^ src_aligned) & mask);
}

}
