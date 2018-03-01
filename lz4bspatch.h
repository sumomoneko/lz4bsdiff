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

#ifndef LZ4BSPATCH_H_INCLUDED_
#define LZ4BSPATCH_H_INCLUDED_

#include <stddef.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifndef LZ4BSPATCH_BLOCK_BYTES
#  define LZ4BSPATCH_BLOCK_BYTES (512)
#endif


typedef struct {

    /** context

        pass throuch to read/write function parameter.
     */
    void* context;

    /** read function

        Called when patch data required.

        @param[in] buf buffer to be filled.
        @param[in] len required data length.
        @param[in] context

        @retuen read length.
    */
    ssize_t (*read)(void* buf, size_t len, void* context);

    /** write function

        Called when new data has made.

        @param[in] buf     conteins data to write.
        @param[in] len     valid buf length in bytes.
        @param[in] context

        @retval <0 stop patching.
    */
    ssize_t (*write)(const void* buf, size_t len, void* context);

} lz4bspatch_handler_t;


/** apply a patch

    @param[in] old_data         old data buffer pointer.
    @param[in] old_data_size    old_data buffer size in bytes.
    @param[in] handler          callback functions for read patch and write new data.

    @retval 0 success
    @retval -1 failed
 */
int lz4bspatch(const void* old_data, size_t old_data_size, lz4bspatch_handler_t* handler);

#ifdef __cplusplus
}
#endif //  __cplusplus

#endif // LZ4BSPATCH_H_INCLUDED_
