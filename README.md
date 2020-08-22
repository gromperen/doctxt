# doctxt

doctxt is a simple, fast docx to txt conversion tool written in C.

### Dependecies

libzip, libxml2 for building

```sh
$ apt install libxml2-dev
$ apt install libzip-dev
```

### Installation

Install dependecies first. 

```sh
$ make clean
$ make
$ make install
```

### Usage

```sh
$ doctxt [FILE] [-o OUTFILE]
```

If -o is omitted output will be written to out.txt
