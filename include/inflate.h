#pragma once

#include <vector>
#include <stdexcept>

#include <zlib.h>

std::vector<uint8_t> inflate(std::vector<uint8_t> & in) {
	z_stream stream { .next_in = in.data(), .avail_in = (uInt)in.size() };
	if(inflateInit(&stream) != Z_OK) {
		throw std::runtime_error(fmt::format(
			"Could not initialize the inflate procedure: {}", stream.msg
		));
	}

	std::vector<uint8_t> out(in.size() * 2);
	stream.next_out = out.data();
	stream.avail_out = out.size();

	int res;
	while((res = inflate(&stream, Z_FINISH)) != Z_STREAM_END) {
		if(stream.avail_out == 0 && (res == Z_OK || res == Z_BUF_ERROR)) {
			out.resize(out.size() * 2);
			stream.next_out = out.data() + stream.total_out;
			stream.avail_out = stream.total_out;
		} else {
			std::string msg;

			if(stream.avail_in == 0) msg = "Ran out of input to decompress";
			else if(res == Z_NEED_DICT) msg = "Z_NEED_DICT: unhandled error";
			else msg = fmt::format("Error inflating stream: {}", stream.msg);

			if(inflateEnd(&stream) != Z_OK) {
				msg = fmt::format("{}\nAND\n{}", msg, stream.msg);
			}
			throw std::runtime_error(msg);
		}
	}

	out.resize(stream.total_out);
	if(inflateEnd(&stream) != Z_OK) {
		throw std::runtime_error(fmt::format(
			"Could not cleanup inflate procedure: {}", stream.msg
		));
	}
	return out;
}
