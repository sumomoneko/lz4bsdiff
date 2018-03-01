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

#ifndef LZ4BSDIFF_H_INCLUDED_
#define LZ4BSDIFF_H_INCLUDED_

#ifndef __cplusplus
# error "This is C++ header."
#endif

#include <vector>
#include <stdint.h>

#ifndef LZ4BSPATCH_BLOCK_BYTES
#  define LZ4BSPATCH_BLOCK_BYTES (512)
#endif

/** create patch

   @param[in] old_data
   @param[in] new_data
   @param[out] patch

   @retval 0 success
   @retval 1 failed

   @throw std::bad_alloc
 */
int lz4bsdiff(const std::vector<uint8_t>& old_data,
              const std::vector<uint8_t>& new_data,
              std::vector<uint8_t>& patch);

#endif // LZ4BSDIFF_H_INCLUDED_
