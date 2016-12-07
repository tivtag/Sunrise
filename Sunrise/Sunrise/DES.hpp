#pragma once

#include <cstddef>

namespace sr 
{ 
    namespace des
    {
        void decode(unsigned char* buf, std::size_t len, int cycle);
        void decodeFilename(unsigned char* buf, const std::size_t len);
    }
}
