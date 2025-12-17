#pragma once

#include "common/types.h"

namespace EQ
{
	uint32 EstimateDeflateBuffer(uint32 len);
	uint32 DeflateData(const char *buffer, uint32 len, char *out_buffer, uint32 out_len_max);
	uint32 InflateData(const char* buffer, uint32 len, char* out_buffer, uint32 out_len_max);
}
