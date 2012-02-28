YUVIT
=====

YUVIT is a command line tool that can convert images into YUV sequences or streams. This tool has two major benefits: wide formats support for output yuv images and wide formats support for input images. 

This tool was started in far 2005 and I released it on Sourceforge as any2yuv. It was based on open source library named Corona (died in far-far 2003). I didn't maintained it since it was published, therefore it missed functionality, sources were unorganized and so on. But in 2011 I got several requests and bug reports about this tool. After that I decided to rewrite this tool from scratch.

Meet the YUVIT for Mac, Windows and Linux. It's simply better then its predecessor. Now YUVIT uses FreeImage library, which provides support for tens image codecs like jpeg, tiff, png, pcx, bmp, gif and so on. 


USAGE
=====

        yuvit [options] [-f format] [-s uvscale] <InFile> <OutFile>

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
===========

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
        UUUU

AUTHOR
======

(c) 2005-2012 Alexander Shashkevych <alexander()stunpix.com>

LICENSE
=======

YUVIT code is licensed under LGPL. Code of FreeImage and image codecs are distributed under their own licenses.