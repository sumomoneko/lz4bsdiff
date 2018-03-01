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

#include "lz4bspatch.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <functional>

#include <string.h>
#include <assert.h>


int main(int argc, char* argv[])
{
	if (argc != 4) {
        std::cerr << "usage: " << argv[0] << " oldfile newfile patchfile" << std::endl;
        return 1;
    }

    const char* oldname = argv[1];
    const char* newname = argv[2];
    const char* patchname = argv[3];

    /*
      read old file
    */
    std::vector<uint8_t> oldbin;
    {
        std::ifstream oldfs(oldname, std::ios::in | std::ios::binary);

        if (oldfs.fail()) {
            std::cerr << "old file " << oldname << " open failed." << std::endl;
            return 1;
        }

        oldbin.assign(std::istreambuf_iterator<char>(oldfs),
                      std::istreambuf_iterator<char>());
        if (oldfs.fail()) {
            std::cerr << "old file " << oldname << " read failed." << std::endl;
            return 1;
        }
        oldfs.close();
    }

    /*
      read patch file
    */
    std::vector<uint8_t> diff;
    {
        std::ifstream difffs(patchname, std::ios::in | std::ios::binary);

        if (difffs.fail()) {
            std::cerr << "patch file " << patchname << " open failed." << std::endl;
            return 1;
        }

        diff.assign(std::istreambuf_iterator<char>(difffs),
                    std::istreambuf_iterator<char>());
        if (difffs.fail()) {
            std::cerr << "new file " << patchname << " read failed." << std::endl;
            return 1;
        }
        difffs.close();
    }

    /*
      apply patch
     */
    std::vector<uint8_t> patched_newbin;
    auto context = std::make_pair(std::ref(diff), std::ref(patched_newbin));

    lz4bspatch_handler_t handler =
    {
        .context = &context,
        .read = [](void* buf, size_t len, void* c) -> ssize_t {
            auto& ctx = *reinterpret_cast<std::pair<std::vector<uint8_t>&, std::vector<uint8_t>&>*>(c);
            auto& v = ctx.first;
            size_t l = std::min(v.size(), len);
            memcpy(buf, &v[0], l);
            v.erase(v.begin(), v.begin()+l);
            return static_cast<ssize_t>(l);
        },
        .write = [](const void* buf, size_t len, void* c) -> ssize_t {
            auto& ctx = *reinterpret_cast<std::pair<std::vector<uint8_t>&, std::vector<uint8_t>&>*>(c);
            auto& v = ctx.second;
            auto b = reinterpret_cast<const uint8_t*>(buf);
            std::copy(b, b+len, std::back_inserter(v));
            return static_cast<ssize_t>(len);
        }
    };

    int result = lz4bspatch(&oldbin[0], oldbin.size(), &handler);
    if (result != 0) {
        std::cerr << "patch failed." << std::endl;
        return 1;
    }

    /*
      write new file
     */
    std::ofstream newfs(newname, std::ios::out | std::ios::binary);
    if (newfs.fail()) {
        std::cerr << "new file " << newname << " open failed." << std::endl;
        return 1;
    }

    newfs.write(reinterpret_cast<char*>(&patched_newbin[0]), static_cast<std::streamsize>(patched_newbin.size()));

    newfs.close();
    if (newfs.fail()) {
        std::cerr << "new file " << newname << " write failed." << std::endl;
        return 1;
    }

    return 0;
}

