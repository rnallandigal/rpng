#pragma once

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define htonll(x)	__builtin_bswap64(x)
#define htonl(x)	__builtin_bswap32(x)
#define htons(x)	__builtin_bswap16(x)
#define ntohll(x)	__builtin_bswap64(x)
#define ntohl(x)	__builtin_bswap32(x)
#define ntohs(x)	__builtin_bswap16(x)
#else
#define htonll(x)	(x)
#define htonl(x)	(x)
#define htons(x)	(x)
#define ntohll(x)	(x)
#define ntohl(x)	(x)
#define ntohs(x)	(x)
#endif
