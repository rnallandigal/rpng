#pragma once

#include <span>
#include <string>
#include <stdexcept>
#include <memory>
#include <ios>
#include <filesystem>
#include <unordered_map>
#include <vector>

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <zlib.h>

#include "net.h"
#include "constants.h"
#include "chunk.h"
#include "inflate.h"
#include "reconstruct.h"
#include "interlace.h"
#include "netpbm.h"
#include "chunk/ihdr.h"

namespace rpng {

void parse_png_header(std::ifstream & ifs) {
	uint64_t filetype = 0xdeadbeefdeadbeef;
	ifs.read((char*)&filetype, 8);
	if(ifs.gcount() != 8 || filetype != PNG_MAGIC)
		throw std::runtime_error(
			fmt::format("Unexpected PNG filetype: {:#x}", filetype)
		);
}

chunk_t parse_chunk(std::ifstream & ifs) {
	chunk_t chunk{};
	uint32_t data;

	ifs.read((char*)&data, 4);
	chunk.length = ntohl(data);
	if(ifs.gcount() != 4)
		throw std::runtime_error("Unexpected end of file @ chunk length");

	ifs.read((char*)&chunk.type, 4);
	if(ifs.gcount() != 4)
		throw std::runtime_error("Unexpected end of file @ chunk type");

	if(chunk.length > 0) {
		chunk.data.resize(chunk.length);
		ifs.read((char*)chunk.data.data(), chunk.length);
		if(ifs.gcount() != chunk.length)
			throw std::runtime_error("Unexpected end of file @ chunk data");
	}

	ifs.read((char*)&data, 4);
	chunk.crc = ntohl(data);
	if(ifs.gcount() != 4)
		throw std::runtime_error("Unexpected end of file @ chunk CRC");

	SPDLOG_DEBUG("parsed chunk: {}", chunk);
	return chunk;
}

std::pair<chunk_ihdr_data_t, colour_properties_t>
parse_ihdr(std::ifstream & ifs) {
	chunk_t ihdr = parse_chunk(ifs);
	if(ihdr.type != CHUNK_TYPE_IHDR) {
		throw std::runtime_error(fmt::format(
			"First chunk type is not IHDR: {}", ihdr
		));
	}

	if(ihdr.data.size() != sizeof(chunk_ihdr_data_t))
		throw std::runtime_error(fmt::format(
			"IHDR content size mismatch. Expecting {}, found {}",
			sizeof(chunk_ihdr_data_t),
			ihdr.data.size()
		));

	chunk_ihdr_data_t ihdr_data{};
	memcpy(&ihdr_data, ihdr.data.data(), sizeof(chunk_ihdr_data_t));
	ihdr_data.width = ntohl(ihdr_data.width);
	ihdr_data.height = ntohl(ihdr_data.height);

	if(ihdr_data.width == 0 || ihdr_data.height == 0)
		throw std::runtime_error("Neither width nor height may be zero");

	colour_properties_t colours = colour_properties[
		std::min<uint8_t>(ihdr_data.colour_type, colour_properties.size() - 1)
	];

	if(colours.colour_type == COLOUR_TYPE_INVALID) {
		throw std::runtime_error(
			fmt::format("Colour type {} is not valid", ihdr_data.colour_type)
		);
	} else if((ihdr_data.bit_depth & colours.valid_bit_depths) == 0) {
		throw std::runtime_error(fmt::format(
			"Colour type {} and bit depth {} combination is not valid",
			ihdr_data.colour_type,
			ihdr_data.bit_depth
		));
	}
	return { ihdr_data, colours };
}

std::vector<uint8_t> pack_idat_chunks(std::ifstream & ifs) {
	std::unordered_map<uint32_t, std::vector<chunk_t>> chunks;
	while(ifs && ifs.peek() != std::ifstream::traits_type::eof()) {
		chunk_t & chunk = [&]() -> chunk_t & {
			chunk_t tmp = parse_chunk(ifs);
			return chunks[tmp.type].emplace_back(std::move(tmp));
		}();

		switch(chunk.type) {
		case CHUNK_TYPE_IHDR:
		case CHUNK_TYPE_PLTE:
		case CHUNK_TYPE_IDAT:
		case CHUNK_TYPE_IEND:
			break;
		default:
			if(chunk.type & (1 << 5))
				SPDLOG_WARN(
					"Ignoring unrecognized ancillary chunk: {}",
					chunk
				);
			else
				throw std::runtime_error(fmt::format(
					"Encountered unrecognized critical chunk: {}", chunk
				));
		}
	}

	std::vector<uint8_t> packed;
	for(auto const & chunk : chunks[CHUNK_TYPE_IDAT]) {
		packed.insert(packed.end(), chunk.data.begin(), chunk.data.end());
	}
	return packed;
}

std::vector<uint8_t> load(std::string const & filepath) {
	SPDLOG_DEBUG("{}", filepath);
	std::ifstream ifs(filepath, std::ios::binary);
	if(!ifs)
		throw std::runtime_error(fmt::format("Cannot open file {}", filepath));

	parse_png_header(ifs);

	auto [ihdr_data, colours] = parse_ihdr(ifs);
	SPDLOG_DEBUG("\n{}\n", ihdr_data);

	std::vector<uint8_t> packed = pack_idat_chunks(ifs); 
	SPDLOG_DEBUG("packed idat size: {}", packed.size());

	std::vector<uint8_t> filtered = inflate(packed);
	SPDLOG_DEBUG("inflated size: {}", filtered.size());

	std::vector<uint8_t> reconstructed
		= reconstruct(filtered, ihdr_data, colours);
	SPDLOG_DEBUG("reconstructed size: {}", filtered.size());

	std::vector<uint8_t> raw;
	if(ihdr_data.interlace) {
		raw = deinterlace(reconstructed, ihdr_data, colours);
	} else {
		raw = reconstructed;
	}
	SPDLOG_DEBUG("raw size: {}", raw.size());

	return raw;

	/*
	std::filesystem::create_directory(
		std::filesystem::path("./out"),
		std::filesystem::path(".")
	);

	save_greyscale(
		fmt::format(
			"out/{}.pgm",
			std::filesystem::path(filepath).stem().string()
		),
		raw,
		ihdr_data
	);
	*/
}

}
