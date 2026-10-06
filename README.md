# doctxt

doctxt is a simple, fast docx to txt conversion tool written in C.

### Dependencies

libzip, libxml2 for building

```sh
$ apt install libxml2-dev
$ apt install libzip-dev
```

### Installation

Install dependencies first. 

```sh
$ make clean
$ make
$ make install
```

### Usage

```sh
$ doctxt FILE [-o OUTFILE]
$ doctxt -v
```

If -o is omitted output will be written to out.txt. `-v` prints the version.
