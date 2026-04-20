implementation of the byte-pair encoding algorithm, both encoding and decoding.
the input is treated as a stream of bytes, so it _should_ work regardless of the
encoding or file format.

uses the [stb library](https://github.com/nothings/stb) for hash maps, and uses
dynamic arrays from the [nob library](https://github.com/tsoding/nob.h).

in the implementation, pairs of bytes are replaced with 16-bit integer "tokens".
this is not strictly a compression algorithm, but it can compress inputs that
contain a lot of repetitions (mostly natural-language inputs), as the tokens
merge together and thus a single token may contain more information than just
the two bytes it replaces. on a test input of 1m (log outputs) with a bunch of
repetitions, the output `.bpe` file (the encoded one) sits at 31k and the `.tkn`
file (file with token -- byte-pair pairs) at 38k. on binary files there's no
compression at all, but i guess this algorithm was not meant for this type of
data.
