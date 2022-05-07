#pragma once

namespace rpng {

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

int count(int length, int offset, int period) {
	return (length - offset + period - 1) / period;
}

}
