#include <span>
#include <string>
#include <stdexcept>
#include <memory>
#include <ios>
#include <fstream>
#include <unordered_map>

#include <fmt/format.h>
#include <docopt/docopt.h>

#include "net.h"
#include "constants.h"
#include "chunk.h"
#include "ihdr.h"

using namespace rpng;

const char COMMAND[] =
R"(rpng
Load a file in Portable Network Graphics (PNG) format.

Usage:   rpng load (--file FILE)

Options:
    -f, --file FILE         The path to the PNG file to load.
    -h, --help              Show this screen.
)";

chunk_t read_chunk(std::ifstream & ifs) {
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

	fmt::print("parsed chunk: {}\n", chunk);
	return chunk;
}

void load(std::string const & filepath) {
	std::ifstream ifs(filepath, std::ios::binary);
	if(!ifs)
		throw std::runtime_error(fmt::format("Cannot open file {}", filepath));
	
	// PNG header
	uint64_t filetype = 0xdeadbeefdeadbeef;
	ifs.read((char*)&filetype, 8);
	if(ifs.gcount() != 8 || filetype != PNG_MAGIC)
		throw std::runtime_error(
			fmt::format("Unexpected PNG filetype: {:#x}", filetype)
		);

	// IHDR header
	chunk_t ihdr = read_chunk(ifs);
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

	fmt::print("\n{}\n\n", ihdr_data);

	if(ihdr_data.width == 0 || ihdr_data.height == 0)
		throw std::runtime_error("Neither width nor height may be zero");

	std::unordered_map<uint8_t, uint8_t> combos {
		{ 0, 0b00011111 },
		{ 2, 0b00011000 },
		{ 3, 0b00001111 },
		{ 4, 0b00011000 },
		{ 6, 0b00011000 }
	};

	if(auto it = combos.find(ihdr_data.colour_type); it == combos.end()) {
		throw std::runtime_error(
			fmt::format("Colour type {} is not valid", ihdr_data.colour_type)
		);
	} else if((ihdr_data.bit_depth & it->second) == 0) {
		throw std::runtime_error(fmt::format(
			"Colour type {} and bit depth {} combination is not valid",
			ihdr_data.colour_type,
			ihdr_data.bit_depth
		));
	}

	// Read each chunk
	while(ifs && ifs.peek() != decltype(ifs)::traits_type::eof()) {
		chunk_t chunk = read_chunk(ifs);
		switch(chunk.type) {
		case CHUNK_TYPE_IHDR:
		case CHUNK_TYPE_PLTE:
		case CHUNK_TYPE_IDAT:
		case CHUNK_TYPE_IEND:
			break;
		default:
			if(chunk.type & (1 << 5))
				fmt::print(
					"Ignoring unrecognized ancillary chunk: {}\n",
					chunk
				);
			else
				throw std::runtime_error(fmt::format(
					"Encountered unrecognized critical chunk: {}", chunk
				));
		}
	}
}

int main(int argc, char ** argv) {
	auto args = docopt::docopt(COMMAND, { argv + 1, argv + argc }, true, "");
	if(args["load"].asBool()) {
		load(args["--file"].asString());
	} else {
		throw std::runtime_error(argc > 0
			? fmt::format("Unable to interpret command: {}", argv[0])
			: fmt::format("Unable to interpret command")
		);
	}
	return 0;
}
