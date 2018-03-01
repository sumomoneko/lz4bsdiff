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


#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include "lz4bsdiff.h"
#include "lz4bspatch.h"

#include <iostream>
#include <iterator>
#include <sstream>
#include <memory>
#include <vector>
#include <random>
#include <algorithm>


template<typename T>
std::vector<T>
make_data(size_t len, int seed)
{
    std::mt19937 mt(seed);
    std::uniform_int_distribution<> rand_uint8(std::numeric_limits<T>::min(),
                                               std::numeric_limits<T>::max());
    std::vector<T> data(len);
    std::generate( data.begin(), data.end(), [&](){return rand_uint8(mt);} ) ;

    return data;
}


static void
check(const std::vector<uint8_t>&old_data, const std::vector<uint8_t>&new_data)
{
    // create patch data
    std::vector<uint8_t> diff;
    int result = lz4bsdiff(old_data, new_data, diff);
    REQUIRE(result == 0);

    INFO("patch size is: " << diff.size());

    // apply patch
    std::vector<uint8_t> patched_data;
    auto context = std::make_pair(std::ref(diff), std::ref(patched_data));
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
            const char* b = reinterpret_cast<const char*>(buf);
            std::copy(b, b+len, std::back_inserter(v));
            return static_cast<ssize_t>(len);
        }
    };
    result = lz4bspatch(&old_data[0], old_data.size(), &handler);
    REQUIRE(result == 0);

    // check
    REQUIRE(new_data == patched_data);
}


TEST_CASE() {
    // same data
    check(make_data<uint8_t>(1024, 100), make_data<uint8_t>(1024, 100));

    // differ data
    check(make_data<uint8_t>(1024, 100), make_data<uint8_t>(1024, 99));

    // differ length, differ data
    check(make_data<uint8_t>(1024, 100), make_data<uint8_t>(10, 99));
    check(make_data<uint8_t>(10, 100), make_data<uint8_t>(1024, 99));

    // differ length, same data
    check(make_data<uint8_t>(1024, 100), make_data<uint8_t>(1023, 100));
    check(make_data<uint8_t>(1024, 100), make_data<uint8_t>(1025, 100));

    // short data
    check(make_data<uint8_t>(1, 100), make_data<uint8_t>(2, 100));
    check(make_data<uint8_t>(2, 100), make_data<uint8_t>(1, 100));

    // valious len, differ data
    for (size_t i = 1; i < 1024; ++i) {
        check(make_data<uint8_t>(i, i), make_data<uint8_t>(i, i+1));
        check(make_data<uint8_t>(i, i), make_data<uint8_t>(i+1, i+1));
    }

    // valious len, differ data
    for (size_t i = 5; i < 10240; i += 10) {
        check(make_data<uint8_t>(i, i), make_data<uint8_t>(i, i+1));
        check(make_data<uint8_t>(i, i), make_data<uint8_t>(i+1, i+1));
    }

    // differ one byte.
    auto a = make_data<uint8_t>(10000, 5);
    auto b = a;
    b[100] += 1;
    check(a, b);


    // malformed patch not tested...
}
