#pragma once

#include "fmtutils.h"

namespace rpng {

struct chunk_ihdr_data_t {
	uint32_t width;
	uint32_t height;
	uint8_t bit_depth;
	uint8_t colour_type;
	uint8_t compression;
	uint8_t filter;
	uint8_t interlace;
} __attribute__((packed));

// Colour types
constexpr uint8_t COLOUR_TYPE_GREYSCALE			= 0;
constexpr uint8_t COLOUR_TYPE_TRUECOLOUR		= 2;
constexpr uint8_t COLOUR_TYPE_INDEXED_COLOUR	= 3;
constexpr uint8_t COLOUR_TYPE_GREYSCALE_ALPHA	= 4;
constexpr uint8_t COLOUR_TYPE_TRUECOLOUR_ALPHA	= 6;
constexpr uint8_t COLOUR_TYPE_INVALID			= 0xff;

struct colour_properties_t {
	char name[32];
	uint8_t colour_type			= COLOUR_TYPE_INVALID;
	uint8_t valid_bit_depths;
	uint8_t num_channels;
};

constexpr std::array<colour_properties_t, 8>  colour_properties {{
	{ "Greyscale",				COLOUR_TYPE_GREYSCALE,			0b00011111, 1 },
	{ },
	{ "Truecolour",				COLOUR_TYPE_TRUECOLOUR,			0b00011000, 3 },
	{ "Indexed-colour",			COLOUR_TYPE_INDEXED_COLOUR,		0b00001111, 1 },
	{ "Greyscale with alpha",	COLOUR_TYPE_GREYSCALE_ALPHA,	0b00011000, 2 },
	{ },
	{ "Truecolour with alpha",	COLOUR_TYPE_TRUECOLOUR_ALPHA,	0b00011000,	4 },
	{ }
}};

}

namespace fmt {

template <>
struct formatter<rpng::chunk_ihdr_data_t> : base_formatter {
	template <class FormatContext>
	auto format(rpng::chunk_ihdr_data_t const & ihdr, FormatContext & ctx) {
		std::string rendered_compression = ihdr.compression == 1
			? "Unknown" : "DEFLATE";

		std::string rendered_filter = ihdr.filter == 1
			? "Unknown" : "Adaptive filtering";

		std::string rendered_interlace = ihdr.interlace == 0
			? "No interlace" : "Adam7 interlace";

		return format_to(
			ctx.out(),
			"IHDR content:\n"
			"dimensions (WxH)   : {} x {}\n"
			"bit depth          : {}\n"
			"colour_type        : {} ({})\n"
			"compression method : {} ({})\n"
			"filter method      : {} ({})\n"
			"interlace method   : {} ({})",
			ihdr.width, ihdr.height,
			ihdr.bit_depth,
			rpng::colour_properties[ihdr.colour_type].name, ihdr.colour_type,
			rendered_compression, ihdr.compression,
			rendered_filter, ihdr.filter,
			rendered_interlace, ihdr.interlace
		);
	}
};

}
