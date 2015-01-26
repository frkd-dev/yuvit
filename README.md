YUVIT
=====

YUVIT is open source command line tool for convertinng jpeg, bmp, png, tiff images into YUV images or sequences and vice versa. Tool has wide support of YUV formats. Wide support of common input/output formats (png, jpeg, tiff, etc) provided by [FreeImage][1] library. 

[1]: http://freeimage.sf.net

Build
-----

First, install build tools.

Debian based distros (Ubuntu, Mint):

    sudo apt-get install g++ cmake libfreeimage-dev

RadHat based distros (Fedora, CentOS):

    sudo yum install gcc-c++ cmake freeimage-devel

Get sources and build:

    git clone https://github.com/stunpix/yuvit.git
    mkdir yuvit/build && cd yuvit/build
    cmake ..
    make
    cpack

You'll get .deb and .rpm packages in current directory.

Usage
-----

        yuvit [options] [-f format] [-s uvscale] <InFile> [OutFile]

        Options:
                -a : Add new image to the end of output file. Don't truncate output file.
                -m <start>:<end> : Multiple file input. Where:
                    start : Sequence start
                    end  : Sequence end
                -i : Interleave UV rows for planar formats
                -w : Swap UV components order

        Formats (-f option):
                yuv  : Planar format [DEFAULT]
                yuyv : Packed format
                uyvy : Packed format
                yyuv : Planar packed chroma format

        UV scales (-s option. Used only with -f and planar formats):
                h1v1 : UV not scaled down [DEFAULT]
                h2v2 : UV scaled down by 2x horizontally and vertically
                h2v1 : UV scaled down by 2x horizontally
                h1v2 : UV scaled down by 2x vertically

        Note: Use symbol '#' in file names for enumerators.

        Examples:
            yuvit -a -m 0:100 test###.bmp out.yuv
                    Convert images from 'test000.bmp' to 'test100.bmp' into single 'out.yuv' file

            yuvit -m 10:200 test######.jpg out###.yuv
                    Convert images 'test000010.jpg'...'test000200.jpg' into files 'out010.yuv'...'out200.yuv'

YUV Formats
-----------

Here are examples of how 4x4 pixels image can be stored in YUV format depending on options (not all possible cases are shown).
 
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

History
-------

Initially tool has been released in 2006 on Sourceforge and named as `any2yuv`. In 2011 after several requests and bug reports, it was rewritten from the ground, renamed to `yuvit` and moved to github.


Author
------

2005-2015 Alexander Shashkevych <stunpix_gmail.com>

License
-------

Source code licensed under LGPLv3.
