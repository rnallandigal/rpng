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

// Colour type
constexpr uint8_t COLOUR_TYPE_GREYSCALE			= 0;
constexpr uint8_t COLOUR_TYPE_TRUECOLOR			= 2;
constexpr uint8_t COLOUR_TYPE_INDEXED			= 3;
constexpr uint8_t COLOUR_TYPE_GREYSCALE_ALPHA	= 4;
constexpr uint8_t COLOUR_TYPE_TRUECOLOR_ALPHA	= 6;

}

namespace fmt {

template <>
struct formatter<rpng::chunk_ihdr_data_t> : base_formatter {
	template <class FormatContext>
	auto format(rpng::chunk_ihdr_data_t const & ihdr, FormatContext & ctx) {
		std::string rendered_colour_type = [](uint8_t colour_type) {
			using namespace rpng;
			switch(colour_type) {
				case COLOUR_TYPE_GREYSCALE:		 	return "Greyscale";
				case COLOUR_TYPE_TRUECOLOR:		 	return "Truecolor";
				case COLOUR_TYPE_INDEXED:		 	return "Indexed-color";
				case COLOUR_TYPE_GREYSCALE_ALPHA:	return "Greyscale with alpha";
				case COLOUR_TYPE_TRUECOLOR_ALPHA:	return "Truecolor with alpha";
				default:							return "Unrecognised";
			};
		}(ihdr.colour_type);

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
			rendered_colour_type, ihdr.colour_type,
			rendered_compression, ihdr.compression,
			rendered_filter, ihdr.filter,
			rendered_interlace, ihdr.interlace
		);
	}
};

}
