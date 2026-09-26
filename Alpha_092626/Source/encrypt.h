/**
 * @file encrypt.h
 *
 * Interface of functions for compression and decompressing MPQ data.
 */
#pragma once

#include <cstdint>

#include "utils/stdcompat/cstddef.hpp"

namespace devilution {

struct TDataInfo {
	byte *srcData;
	uint32_t srcOffset;
	byte *destData;
	uint32_t destOffset;
	uint32_t size;
	uint32_t destCapacity;
	bool overflow;
};

void Decrypt(uint32_t *castBlock, uint32_t size, uint32_t key);
void Encrypt(uint32_t *castBlock, uint32_t size, uint32_t key);
uint32_t Hash(const char *s, int type);
uint32_t PkwareCompress(byte *srcData, uint32_t size);
uint32_t PkwareDecompress(byte *inBuff, uint32_t recvSize, uint32_t maxBytes);

} // namespace devilution
