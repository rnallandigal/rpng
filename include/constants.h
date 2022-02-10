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

// Ancillary chunk types
constexpr uint32_t CHUNK_TYPE_CHRM	= htonl(0x6348524d);
constexpr uint32_t CHUNK_TYPE_GAMA	= htonl(0x67414d41);
constexpr uint32_t CHUNK_TYPE_ICCP	= htonl(0x69434350);
constexpr uint32_t CHUNK_TYPE_SBIT	= htonl(0x73424954);
constexpr uint32_t CHUNK_TYPE_SRGB	= htonl(0x73524742);
constexpr uint32_t CHUNK_TYPE_BKGD	= htonl(0x624b4744);
constexpr uint32_t CHUNK_TYPE_HIST	= htonl(0x68495354);
constexpr uint32_t CHUNK_TYPE_TRNS	= htonl(0x74524e53);
constexpr uint32_t CHUNK_TYPE_PHYS	= htonl(0x70485973);
constexpr uint32_t CHUNK_TYPE_SPLT	= htonl(0x73504c54);
constexpr uint32_t CHUNK_TYPE_TIME	= htonl(0x74494d45);
constexpr uint32_t CHUNK_TYPE_ITXT	= htonl(0x69545874);
constexpr uint32_t CHUNK_TYPE_TEXT	= htonl(0x74455874);
constexpr uint32_t CHUNK_TYPE_ZTXT	= htonl(0x7a545874);

}
