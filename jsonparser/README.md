this is a simple barebones json parser in C. this is my first time ever writing
a parser and first time trying to make code "proper" (more fleshed out builds
etc.).

a showcase of the api can be seen in `main.c`. in general any json file is
turned into a `json_object` struct that has two fields: a `OBJECT_KIND` enum
that indicates what kind of object it is (string, number, array or a key-value
map) and a union that contains the data of the object. strings are
nul-terminated `char` pointers, numbers are `double`s, arrays are dynamic array
structs (api for it is in `src/smol.h`, a smol header libray with helpers
largely based on tsoding's nob.h) and key-value maps are hashmaps implemented by
`stb_ds.h`.

to build a static library into the build directory run `make lib`. to build the
`main.c` example run `make`.

to debug memory leaks i used `valgrind` and from what i've been able to test,
this is memory-safe, which i'm very happy about :)
