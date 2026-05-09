#include "Compression.h"

void decompress(const char* in, size_t in_len, char* out, size_t out_len) {
    int status;
    z_stream d_stream;
    
    d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;
    
    d_stream.next_in  = (Bytef*)in;
    d_stream.avail_in = in_len;
    d_stream.next_out = (Bytef*)out;
    d_stream.avail_out = out_len;
    
    inflateInit(&d_stream);
    status = inflate(&d_stream, Z_NO_FLUSH);
    inflateEnd(&d_stream);
}
