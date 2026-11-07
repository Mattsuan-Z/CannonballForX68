#pragma once
#include <cstdint>
#include <cstddef>
namespace boost { struct crc_32_type { uint32_t c=0xffffffff; void process_bytes(const void* p, size_t n){ const uint8_t* b=(const uint8_t*)p; for(size_t i=0;i<n;i++){ c^=b[i]; for(int k=0;k<8;k++) c = (c>>1) ^ (0xEDB88320u & (-(int32_t)(c&1))); } } uint32_t checksum() const { return c ^ 0xffffffff; } }; }
