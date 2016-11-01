pxtnplay
==========

## Overview

pxtone collage(.ptcol) player for linux.

## Description

[Pxtone](http://pxtone.org/) is original music format, written by [STUDIO PIXEL](http://studiopixel.sakura.ne.jp/).  
**pxtnplay** can play music file(.ptcol) created by using Pxtone Collage. Enjoy!

![pxtnplay-fs8](https://cloud.githubusercontent.com/assets/19767799/19877535/6cfb9178-a023-11e6-9fd4-4008f1de834e.png)

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
Usage: pxtnplay [options] FILE
Play Pxtone Colage music files (.ptcop) in command line.

General Options
  -h --help        show this help and exit
  -v --version     show version and exit
  -q --quiet       no output

Play Option
  -B --buffer-size play buffer size (1-44100) [frame]
  -c --channels    channels         (1,2)
     --dummy       dummy output
  -r --rate        sample rate      (44100,22050,11025) [KHz]
  -b --bit-rate    bit rate         (8,16) [bit]

ALSA Options
  -d --device      target alsa device

Pxtone Options
  -l --loop        enable loop
     --fadein      enable fade in   (0-10000) [ms]
  -V --volume      change volume    (0-100) [%]
```



## License

[MIT](LICENCE.md)

## Author

- pxtnplay
    - [stkchp](https://github.com/stkchp)
- pxtone collage library(thx for release source code!!)
    - [STUDIO PIXEL](http://studiopixel.sakura.ne.jp/)


