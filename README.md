pxtnplay
==========

## Overview

pxtone collage(.ptcol /.pptune) player for linux.

## Description

[Pxtone](http://pxtone.org/) is original music format, written by [STUDIO PIXEL](http://studiopixel.sakura.ne.jp/).
**pxtnplay** can play music file(.ptcol, .pptune) in simple CUI.


- TODO: paste screenshot.

## Installation

- Requirement
	- Linux(kernel 2.32 or later)
	- c++11 compiler
		- gcc 4.8.1 or later
		- clang 3.3 or later
	- cmake
	- alsa-lib
	- libvorbis

```bash
 $ cmake .
 $ make && make install
```

## Usage

```
Usage: ./pxtnplay [options] FILE
Play Pxtone Colage music files (.ptcop /.pptune) in command line.

General Options
  -h --help       show this help and exit
  -v --version    show version and exit
  -q --quiet      no output

ALSA Options
  not implemented yet

Pxtone Options
  -l --loop       enable loop
  -c --channel    pxtone channe
```



## License

[MIT](https://github.com/stkchp/pxtnplay/blob/master/LICENCE.md)

## Author

- pxtnplay
    - [stkchp](https://github.com/stkchp)
- pxtone collage library(thx for release source code!!)
    - [STUDIO PIXEL](http://studiopixel.sakura.ne.jp/)


