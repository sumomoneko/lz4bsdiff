lz4bsdiff/lz4bspatch
====================

lz4bsdiff/lz4bspatch are libraries for building and applying patches
to binary files on tight RAM space environment.

lz4bspatch() function require only about LZ4 compless block size RAM.


License
-------

This project includes LZ4, bspatch/bsdiff project.

This project is governed by the BSD 2-clause license. For details see the file
titled LICENSE in the project root folder.

Configuration macro
--------------------

<dl>
    <dt><code>LZ4BSPATCH_BLOCK_BYTES</code>
    <dd>LZ4 compress blocksize. Default is 512 bytes.
    lz4bspatch require this buffer on the stack.
    There is a trade-off between compression ratio and memory usage.
</dl>

Build
-----

``` shell
cmake .
make
make test
```

<dl>
    <dt>liblz4bsdiff.a
    <dd>patch generating library.
    <dt>lz4bsdiff
    <dd>generate a patch between two binary files.
    <dt>liblz4bspatch.a
    <dd>patch applying library.
    <dt>lz4bspatch
    <dd>apply a patch built with lz4bsdiff.
    <dt>unitTest
    <dd>simple unitTest for liblz4bsdiff.a and liblz4bspatch.a
</dl>
