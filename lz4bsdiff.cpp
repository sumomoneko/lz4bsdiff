/**

   BSD 2-clause license

   Copyright (c) 2018 sumomoneko@gmail.com

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
   COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
   STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
   OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "lz4hc.h"
extern "C" {
#include "bsdiff.h"
}

#include "lz4bsdiff.h"

#include <stdlib.h>
#include <assert.h>
#include <vector>


/** append buffer data to stream->opecue vector.
 */
static int
vec_write(bsdiff_stream* stream, const void* buffer, int size)
{
    auto& vec = *reinterpret_cast<std::vector<uint8_t>*>(stream->opaque);
    const uint8_t* p = reinterpret_cast<const uint8_t*>(buffer);
    std::copy(p, p + size, std::back_inserter(vec));
    return 0;
}


int
lz4bsdiff(const std::vector<uint8_t>& old_data,
          const std::vector<uint8_t>& new_data,
          std::vector<uint8_t>& patch_data)
{
    std::vector<char> plain_patch;
    bsdiff_stream stream = {
        .opaque = &plain_patch,
        .malloc = malloc,
        .free = free,
        .write = vec_write,
    };

    /*
      make patch
     */
    int r = bsdiff(&old_data[0], static_cast<int64_t>(old_data.size()),
                   &new_data[0], static_cast<int64_t>(new_data.size()),
                   &stream);
    if (r != 0) {
        return 1;
    }

    /*
      compress patch with LZ4compress
     */

    patch_data.clear();

    std::vector<char> comp_buf;
    comp_buf.resize(static_cast<size_t>(LZ4_compressBound(LZ4BSPATCH_BLOCK_BYTES)));

    size_t pos = 0;
    size_t remain = plain_patch.size();
    do {
        const size_t len = std::min(remain, static_cast<size_t>(LZ4BSPATCH_BLOCK_BYTES));
        const int comp_bytes = LZ4_compress_HC(&plain_patch[pos], &comp_buf[0],
                                               static_cast<int>(len),
                                               static_cast<int>(comp_buf.size()),
                                               LZ4HC_CLEVEL_MAX);
        if (comp_bytes <= 0) {
            return 1;
        }

        // add uint16_t length field(little endian) for each comporess block.
        patch_data.push_back(static_cast<uint8_t>(comp_bytes & 0xff));
        patch_data.push_back(static_cast<uint8_t>((comp_bytes >> 8) & 0xff));

        // add compressed body
        std::copy(comp_buf.begin(), comp_buf.begin() + comp_bytes, std::back_inserter(patch_data));

        pos += len;
        remain -= len;
        assert( pos + remain == plain_patch.size() );
    } while (remain != 0);

    return 0;
}
