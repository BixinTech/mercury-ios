#pragma once
#include <cstdint>
#include <algorithm>
namespace Conversion
{
union EndianUnion
{
    uint32_t data;
    uint8_t bytes[4];
};
inline uint32_t EndianConversion(uint32_t src)
{
    EndianUnion endian;
    endian.data = src;
    std::reverse(endian.bytes, endian.bytes + 4);
    return endian.data;
}
}

