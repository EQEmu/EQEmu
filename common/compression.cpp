#include "compression.h"

#include "common/types.h"

#include "zlib.h"
#include <cstring>

namespace EQ
{
	uint32 EstimateDeflateBuffer(uint32 len) {
		z_stream zstream;
		memset(&zstream, 0, sizeof(zstream));

		zstream.zalloc = Z_NULL;
		zstream.zfree = Z_NULL;
		zstream.opaque = Z_NULL;
		if (deflateInit(&zstream, Z_FINISH) != Z_OK)
			return 0;

		return deflateBound(&zstream, len);
	}

	uint32 DeflateData(const char *buffer, uint32 len, char *out_buffer, uint32 out_len_max) {
		z_stream zstream;
		memset(&zstream, 0, sizeof(zstream));
		int zerror;

		zstream.next_in = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(buffer));
		zstream.avail_in = len;
		zstream.zalloc = Z_NULL;
		zstream.zfree = Z_NULL;
		zstream.opaque = Z_NULL;
		deflateInit(&zstream, Z_FINISH);

		zstream.next_out = reinterpret_cast<unsigned char*>(out_buffer);
		zstream.avail_out = out_len_max;
		zerror = deflate(&zstream, Z_FINISH);

		if (zerror == Z_STREAM_END)
		{
			deflateEnd(&zstream);
			return (uint32)zstream.total_out;
		}
		else
		{
			zerror = deflateEnd(&zstream);
			return 0;
		}
	}

	uint32 InflateData(const char* buffer, uint32 len, char* out_buffer, uint32 out_len_max) {
		z_stream zstream;
		int zerror = 0;
		int i;

		zstream.next_in = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(buffer));
		zstream.avail_in = len;
		zstream.next_out = reinterpret_cast<unsigned char*>(out_buffer);;
		zstream.avail_out = out_len_max;
		zstream.zalloc = Z_NULL;
		zstream.zfree = Z_NULL;
		zstream.opaque = Z_NULL;

		i = inflateInit2(&zstream, 15);
		if (i != Z_OK) {
			return 0;
		}

		zerror = inflate(&zstream, Z_FINISH);
		if (zerror == Z_STREAM_END) {
			inflateEnd(&zstream);
			return zstream.total_out;
		}
		else {
			if (zerror == -4 && zstream.msg == 0)
			{
				return 0;
			}

			zerror = inflateEnd(&zstream);
			return 0;
		}
	}
}
