# LZWCoder: A Text Compressor

This is a coarse-grained LZW compressor that can easily compress and decompress text and binary files using the LZW algorithm.

**Example:**
```bash
make
./lzwcoder input.txt # encode txt to lzw
./lzwcoder input.lzw # decode lzw to txt
./lzwcoder input -e output # encode input to output
./lzwcoder input -d output # decode input to output
```