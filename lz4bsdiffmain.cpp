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

#include "lz4bsdiff.h"

#include <iostream>
#include <fstream>
#include <vector>

#include <assert.h>


int main(int argc, char* argv[])
{
	if( argc != 4) {
        std::cerr << "usage: " << argv[0] << " oldfile newfile patchfile" << std::endl;
        return 1;
    }

    const char* oldname = argv[1];
    const char* newname = argv[2];
    const char* patchname = argv[3];

    /*
      read old data from file.
    */
    std::vector<uint8_t> oldBin;
    {
        std::ifstream oldfs(oldname, std::ios::in | std::ios::binary);

        if (oldfs.fail()) {
            std::cerr << "old file " << oldname << " open failed." << std::endl;
            return 1;
        }

        oldBin.assign(std::istreambuf_iterator<char>(oldfs),
                      std::istreambuf_iterator<char>());
        if (oldfs.fail()) {
            std::cerr << "old file " << oldname << " read failed." << std::endl;
            return 1;
        }
        oldfs.close();
    }

    /*
      read new data from file.
    */
    std::vector<uint8_t> newBin;
    {
        std::ifstream newfs(newname, std::ios::in | std::ios::binary);

        if (newfs.fail()) {
            std::cerr << "new file " << newname << " open failed." << std::endl;
            return 1;
        }

        newBin.assign(std::istreambuf_iterator<char>(newfs),
                      std::istreambuf_iterator<char>());
        if (newfs.fail()) {
            std::cerr << "new file " << newname << " read failed." << std::endl;
            return 1;
        }
        newfs.close();
    }

    /*
      make patch
     */
    std::vector<uint8_t> diff;
    int result = lz4bsdiff(oldBin, newBin, diff);
    if (result != 0) {
        std::cerr << "diff/compression fails" << std::endl;
        return 1;
    }

    /*
      write to file
     */
    std::ofstream patchfs(patchname, std::ios::out | std::ios::binary);
    if (patchfs.fail()) {
        std::cerr << "patch file " << patchname << " open failed." << std::endl;
        return 1;
    }

    patchfs.write(reinterpret_cast<char*>(&diff[0]), static_cast<std::streamsize>(diff.size()));

    patchfs.close();
    if (patchfs.fail()) {
        std::cerr << "patch file " << patchname << " write failed." << std::endl;
        return 1;
    }

    return 0;
}

