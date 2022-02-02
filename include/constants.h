#pragma once

#include "net.h"

namespace rpng {

// PNG magic constant
constexpr uint64_t PNG_MAGIC	= htonll(0x89504e470d0a1a0a);

// Critical chunk types
constexpr uint32_t CHUNK_TYPE_IHDR	= htonl(0x49484452);
constexpr uint32_t CHUNK_TYPE_PLTE	= htonl(0x504c5445);
constexpr uint32_t CHUNK_TYPE_IDAT	= htonl(0x49444154);
constexpr uint32_t CHUNK_TYPE_IEND	= htonl(0x49454e44);

// TODO: Ancillary chunk types

}
