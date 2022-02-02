#pragma once

#include "fmtutils.h"

namespace rpng {

struct chunk_t {
	uint32_t length;
	uint32_t type;
	std::vector<uint8_t> data;
	uint32_t crc;
};

}

namespace fmt {

template <>
struct formatter<rpng::chunk_t> : base_formatter {
	template <class FormatContext>
	auto format(rpng::chunk_t const & chunk, FormatContext & ctx) {
		return format_to(
			ctx.out(),
			"{} ({:#x}), {}",
			join(std::span((char*)&chunk.type, 4), ""),
			chunk.type,
			chunk.length == 0 ? "empty" : fmt::format("{} bytes", chunk.length)
		);
	}
};

}
