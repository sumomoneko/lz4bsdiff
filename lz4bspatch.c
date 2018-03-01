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

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#include "lz4.h"
#include "bspatch.h"

#include "lz4bspatch.h"


typedef struct {
    void* context;
    ssize_t (*reader)(void* buf, size_t len, void* context);
    ssize_t (*writer)(const void* buf, size_t len, void* context);

    char decomp_buf[LZ4BSPATCH_BLOCK_BYTES];
    char* decomp_bufp;
    size_t remain;
} proxy_context_t;


static bool
is_little_endian(void)
{
    const union { uint32_t u; uint8_t c[4]; } one = { 1 };
    return one.c[0];
}

static bool
fill_buf(proxy_context_t* ctx)
{
    assert(ctx->remain == 0);

    uint16_t comp_len = 0;
    int l = ctx->reader(&comp_len, sizeof(comp_len), ctx->context);
    if (l < 0) {
        return false;
    }

    if (! is_little_endian()) {
        comp_len = (comp_len<< 8) | (comp_len >> 8);
    }

    int decomp_bytes = LZ4_decomplress_onthefly(ctx->reader, ctx->context,
                                                comp_len, ctx->decomp_buf,
                                                LZ4BSPATCH_BLOCK_BYTES);
    if(decomp_bytes <= 0) {
        return false;
    }

    ctx->remain = (size_t)decomp_bytes;
    ctx->decomp_bufp = ctx->decomp_buf;

    return true;
}


static int
read_proxy(const struct bspatch_stream_ex* stream, void* buffer, int length)
{
    proxy_context_t* ctx = (proxy_context_t*)(stream->opaque);

    uint8_t* dst = (uint8_t*)buffer;
    while (length > 0) {
        if (ctx->remain == 0) {
            if (!fill_buf(ctx)) {
                break;
            }
            if (ctx->remain == 0) {
                // no more data.
                break;
            }
        }
        const size_t l = (size_t)length < ctx->remain ? (size_t)length : ctx->remain;
        memcpy(dst, ctx->decomp_bufp, l);

        ctx->decomp_bufp += l;
        ctx->remain -= l;

        dst += l;
        length -= l;
    }

    return length == 0 ? 0 : -1;
}


static int
peep_proxy(const struct bspatch_stream_ex* stream, void** buffer, int length)
{
    proxy_context_t* ctx = (proxy_context_t*)(stream->opaque);
    if (ctx->remain == 0) {
        if (!fill_buf(ctx)) {
            return -1;
        }
    }

    const size_t provide_len = (size_t)length < ctx->remain ? (size_t)length : ctx->remain;

    *buffer = ctx->decomp_bufp;

    ctx->remain -= provide_len;
    ctx->decomp_bufp += provide_len;

    return (int)provide_len;
}


static int
write_proxy(const struct bspatch_stream_ex* stream, const void* buffer, int length)
{
    proxy_context_t* ctx = (proxy_context_t*)(stream->opaque);
    return ctx->writer(buffer, (size_t)length, ctx->context);
}


int
lz4bspatch(const void* old_data, size_t old_data_size, lz4bspatch_handler_t* handler)
{
    proxy_context_t pc = {
        .context = handler->context,
        .reader = handler->read,
        .writer = handler->write,

        .decomp_bufp = NULL,
        .remain = 0,
    };

    struct bspatch_stream_ex stream = {
        .opaque = &pc,
        .read = read_proxy,
        .peep = peep_proxy,
        .write = write_proxy,
    };

    return bspatch_with_stream((const uint8_t*)old_data, (int64_t)old_data_size, &stream);
}



