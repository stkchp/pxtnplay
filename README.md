pxtnplay
==========

## Overview

pxtone collage(.ptcop) player for linux or MacOS.

## Description

[Pxtone](http://pxtone.org/) is original music format, written by [STUDIO PIXEL](http://studiopixel.sakura.ne.jp/).  
**pxtnplay** can play music file(.ptcop) created by using Pxtone Collage. Enjoy!

![pxtnplay-fs8](https://cloud.githubusercontent.com/assets/19767799/19877535/6cfb9178-a023-11e6-9fd4-4008f1de834e.png)

## Installation

- Requirement
	- Linux(kernel 2.32 or later) or MacOSX(confirmed 10.12 only)
	- c++11 compiler
		- gcc 4.8.1 or later
		- clang 3.3 or later
	- cmake
	- libao
	- libvorbis

```bash
 $ cmake .
 $ make
```

### Gentoo Linux

```bash
 $ emerge cmake libao libvorbis
 $ git clone https://github.com/stkchp/pxtnplay.git
 $ cd pxtnplay
 $ cmake .
 $ make
```

### MacOS

I use [Homebrew](http://brew.sh/) in MacOS. Please install Homebrew and run below command.

```bash
 $ brew install cmake libao libvorbis
 $ git clone https://github.com/stkchp/pxtnplay.git
 $ cd pxtnplay
 $ cmake .
 $ make
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

Pxtone Options
  -l --loop        enable loop
     --fadein      enable fade in   (0-10000) [ms]
  -V --volume      change volume    (0-100) [%]

```



## License

[MIT](LICENSE.md)

## Author

- pxtnplay
    - [stkchp](https://github.com/stkchp)
- pxtone collage library(thx for release source code!!)
    - [STUDIO PIXEL](http://studiopixel.sakura.ne.jp/)


