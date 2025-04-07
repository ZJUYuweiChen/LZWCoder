# LZWCoder: A Text Compressor

This is a coarse-grained LZW compressor that can easily compress and decompress text and binary files using the LZW algorithm.

**Make:**

Make sure you've installed boost.

```bash
make # make lzwcoder
```

**Use:**

You can compress or decompress txt/lzw files into the same-named file or specify an output file.

```bash
./lzwcoder input.txt # encode txt to lzw
./lzwcoder input.lzw # decode lzw to txt
./lzwcoder input -e output # encode input to output
./lzwcoder input -d output # decode input to output
```

**Test:**

You can test our LZW compressor's performance in exp.ipynb.