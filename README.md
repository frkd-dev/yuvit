YUVIT
=====

[![Build Status](https://travis-ci.org/stunpix/yuvit.svg)](https://travis-ci.org/stunpix/yuvit)

Command line tool to convert images from popular formats (png, jpeg, etc) into YUV images or even sequences. It can read all image formats which [FreeImage][1] library [supports][2]. 

[1]: http://freeimage.sourceforge.net
[2]: http://freeimage.sourceforge.net/features.html

Build
-----

You'll need cmake, make, g++ and libfreeimage.

On Debian/Ubuntu:

    sudo apt-get install g++ cmake libfreeimage-dev

On RadHat/Fedora:

    sudo yum install gcc-c++ cmake freeimage-devel

For OS X you'll need a [Brew][1] and [Apple Command-line Tools][2]. Install them, then continue:

[1]: http://brew.sh/
[2]: https://developer.apple.com/downloads/

    brew install cmake make freeimage

Clone sources and build:

    git clone https://github.com/stunpix/yuvit.git
    mkdir yuvit/build && cd yuvit/build
    cmake ..
    make

For OS X there is only way to install tool using:

    make install

This installs tool under /usr/local. You can also use this approach for Linux distros, but we recommend to use your system's package manager for installs. To get .deb and .rpm packages, run:

    cpack

and install package using your package manager. With this approach your system will be not polluted with files that aren't controlled by package manager.

Usage
-----

    Usage: yuvit [options] <file>

    Options:
        -h
            This help
        -o
            Output filename. Could be a pattern for read multifile sequences.
        -a
           Append mode. Images will be append to output file. Doesn't truncate output file.
        -m <start>:<end>
           Start and end numbers for multifile sequences.
        -i
           Interleave UV rows for planar formats
        -w
           Swap UV components order
        -x <jpeg|sdtv|hdtv>
           Use YUV conversion matrix. Default: jpeg
                jpeg
                    JFIF specification matrix:
                    |Y|   | 0.299     0.587     0.114|   |R|
                    |U| = |-0.168736 -0.331264  0.5  | x |G|
                    |V|   | 0.5      -0.418688 -0.081|   |B|
                sdtv
                    ITU-R BT.601 for SDTV specification matrix:
                    |Y|   | 0.299    0.587    0.114  |   |R|
                    |U| = |-0.14713 -0.28886  0.436  | x |G|
                    |V|   | 0.615   -0.51499 -0.10001|   |B|
                hdtv
                    ITU-R BT.709 for HDTV specification matrix:
                    |Y|   | 0.2126   0.7152   0.0722 |   |R|
                    |U| = |-0.09991 -0.33609  0.436  | x |G|
                    |V|   | 0.615   -0.55861 -0.05639|   |B|

        -f <yuv|yuyv|uyuv|yyuv>
            Output YUV format. Default: yuv"
                yuv
                    Planar format
                yuyv
                    Packed format
                uyvy
                    Packed format
                yyuv
                    Planar packed chroma format
        -s <h1v1|h2v2|h2v1|h1v2>
            Chroma scaling. Used only for planar formats. Default: h1v1
                h1v1
                    UV not scaled down [DEFAULT]
                h2v2
                    UV scaled down by 2x horizontally and vertically
                h2v1
                    UV scaled down by 2x horizontally
                h1v2
                    UV scaled down by 2x vertically
    Multifile sequences:
        Use '#' in file names, so they will be replaced with numbers.
        Examples:
            yuvit -a -m 0:100 -o out.yuv test###.bmp
                Converts: 'test000.bmp'...'test100.bmp' -> 'out.yuv'
            yuvit -m 10:200 -o out###.yuv test######.jpg
                Converts: 'test000010.jpg'...'test000200.jpg' -> 'out010.yuv'...'out200.yuv'

YUV Formats
-----------

Some option examples and resulting 4x4 pixel YUV images:
 
1) -f yuv -s h1v1 [Default]

        YYYY
        YYYY
        UUUU
        UUUU
        VVVV
        VVVV

2) -f yuv -s h2v1

        YYYY
        YYYY
        UU
        UU
        VV
        VV

3) -f yuv -s h1v2

        YYYY
        YYYY
        UUUU
        VVVV

4) -f yuv -s h2v1 -i

        YYYY
        YYYY
        UUVV
        UUVV

5) -f yuyv

        YUYVYUYV
        YUYVYUYV

6) -f uyvy

        UYVYUYVY
        UYVYUYVY

7) -f yuyv -w

        YVYUYVYU
        YVYUYVYU

8) -f yuv -s h2v1 -i -w

        YYYY
        YYYY
        VVUU
        VVUU

9) -f yuv -s h1v1

        YYYY
        YYYY
        UVUV


Author
------

2005-2015 Alexander Shashkevich <stunpix_gmail.com>

License
-------

Source code is licensed under LGPLv3.
