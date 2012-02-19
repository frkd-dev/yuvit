#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#if defined(__MINGW__)
//#define __declspec(A) __stdcall
#undef _WIN32
#undef __WIN32__
#endif

//extern "C"{
#define FREEIMAGE_LIB

#include "FreeImage.h"
//}

#include "getopt_pp.h"

#define MAJORVERSION "0"
#define MINORVERSION "1"
#define APPVERSION MAJORVERSION "." MINORVERSION

using namespace std;

#define LOG_MESSAGE(...) {printf(__VA_ARGS__); printf("\n");}
#define LOG_ERROR(...) {printf("ERROR: "); printf(__VA_ARGS__); printf("\n");}

enum YUVFormat {
	YUV_YUV,
	YUV_YVU,
	YUV_YVYU,
	YUV_VYUY,
	YUV_UYVY,
	YUV_YUYV
};

enum YUVScale
{
	SCALE_H2V2,
	SCALE_H2V1,
	SCALE_H1V2,
	SCALE_H1V1,
};

class Config
{
public:
	bool appendMode;			/* if not zero append YUV image(s) to output file */
	bool uvInterleave;			/* if not zero, UV rows in planar images are interleaved*/
	uint32_t yuvFormat;			/* YUV output mode. Default: h2v2 */
	uint32_t uvScale;			/* Defines how UV components are scaled in planar mode */
	uint32_t seqStart;			/* Sequence start for multiple files */
	uint32_t seqEnd;			/* Sequence end for multiple files */
	string inFileNamePattern;
	string outFileNamePattern;

	Config();

	// Returns "false" on errors and "true" if no errors
	bool ParseArgs(char* args[], int count);

private:
	bool ParseSequenceRange(string range);
};

void PrintHelp();
string ExpandPattern(string pattern, int counter);

int main(int argc, char* argv[])
{
	Config cfg;
	string inFileName, outFileName;
	FILE* hOutFile;
	uint8_t errorFlag = 1; // By default we will exiting with error
	FIBITMAP *inImage = 0;
	int lumaWidth, lumaHeight;
	int chromaWidth, chromaHeight;
	int x, y, xMask, yMask;
	unsigned char Rc, Gc, Bc;
	unsigned char* rgbPixels;
	unsigned char *yPixels, *uPixels, *vPixels;
	unsigned char *yPtr, *uPtr, *vPtr;

	if(!cfg.ParseArgs(argv, argc))
		exit(1);

	FreeImage_Initialise();

	LOG_MESSAGE("Processing:");

	/* First expanding of output filename. If append mode - this is only one place
	   where name is expanded */
	outFileName = ExpandPattern(cfg.outFileNamePattern, cfg.seqStart);

	/* Main loop of passing through all images */
	while(cfg.seqStart <= cfg.seqEnd)
	{
		inFileName = ExpandPattern(cfg.inFileNamePattern, cfg.seqStart);

		FREE_IMAGE_FORMAT format = FreeImage_GetFileType(inFileName.c_str());
		if(format == FIF_UNKNOWN)
		{
			LOG_ERROR("Input image format is unknown or unsupported...");
			goto HandleError;
		}

		inImage = FreeImage_Load(format, inFileName.c_str());

		if(inImage == 0)
		{
			LOG_ERROR("Some problem occurred with reading of input image...");
			goto HandleError;
		}

		if(cfg.appendMode == 0)
			outFileName = ExpandPattern(cfg.outFileNamePattern, cfg.seqStart);

		hOutFile = fopen( outFileName.c_str(), "ab" ); /* Append to existing file */

		if(hOutFile == 0)
		{
			LOG_ERROR("Can not open output file...");
			goto HandleError;
		}

		LOG_MESSAGE("\t%s", inFileName.c_str());

		FreeImage_FlipVertical(inImage);

		lumaWidth = FreeImage_GetWidth(inImage);
		lumaHeight = FreeImage_GetHeight(inImage);

		/* Calculate dimensions of destination YUV */
		switch(cfg.uvScale)
		{
		default: /* Default scale h1v1 */
		case SCALE_H1V1:
			chromaWidth = lumaWidth;
			chromaHeight = lumaHeight;
			xMask = 0;
			yMask = 0;
			break;
		case SCALE_H2V2:
			chromaWidth = lumaWidth / 2;
			chromaHeight = lumaHeight / 2;
			yMask = xMask = 1;
			break;
		case SCALE_H2V1:
			chromaWidth = lumaWidth;
			chromaHeight = lumaHeight /2;
			xMask = 0;
			yMask = 1;
			break;
		case SCALE_H1V2:
			chromaWidth = lumaWidth / 2;
			chromaHeight = lumaHeight;
			xMask = 1;
			yMask = 0;
			break;
		}

		yPixels = (unsigned char*)malloc(lumaHeight * lumaWidth);
		uPixels = (unsigned char*)malloc(chromaHeight * chromaWidth);
		vPixels = (unsigned char*)malloc(chromaHeight * chromaWidth);

		yPtr = yPixels;
		uPtr = uPixels;
		vPtr = vPixels;

		/* Main converting cycle */
		for(y = 0; y < lumaHeight; y++)
		{
			rgbPixels = FreeImage_GetScanLine(inImage, y);

			for(x = 0; x < lumaWidth; x++)
			{
				Rc = *rgbPixels++;
				Gc = *rgbPixels++;
				Bc = *rgbPixels++;

				*yPtr++ = (0.257f * Rc) + (0.504f * Gc) + (0.098f * Bc) + 16;
				if((y & yMask) == 0 && (x & xMask) == 0 && (y / 2) < chromaHeight && (x / 2) < chromaWidth)
				{
					*uPtr++ = -(0.148f * Rc) - (0.291f * Gc) + (0.439f * Bc) + 128;
					*vPtr++ = (0.439f * Rc) - (0.368f * Gc) - (0.071f * Bc) + 128;
				}
			}
		}


		fwrite(yPixels, 1, lumaWidth * lumaHeight, hOutFile);
		fwrite(vPixels, 1, chromaWidth * chromaHeight, hOutFile);
		fwrite(uPixels, 1, chromaWidth * chromaHeight, hOutFile);

		FreeImage_Unload(inImage);
		inImage = 0;

		fclose(hOutFile);
		hOutFile = 0;

		free(yPixels);
		free(uPixels);
		free(vPixels);

		cfg.seqStart++;
	}

	LOG_MESSAGE("Done!");

	errorFlag = 0; // We successful passed all stages, so set flag to zero which means - OK

HandleError:
	if(inImage)
	{
		FreeImage_Unload(inImage);
		inImage = 0;
	}

	if(hOutFile)
	{
		fclose(hOutFile);
		hOutFile = 0;
	}

	FreeImage_DeInitialise();

	return errorFlag;
}

string toString(int value)
{
	ostringstream oss;
	oss << value;
	return oss.str();
}

/*
 Description: Function searches for '#' symbols and replaces them by integer value with leading zeros
 Returns: Formed string
*/

string ExpandPattern(string pattern, int counter)
{
	string result;
	string::iterator it = pattern.begin();
	uint32_t cntr = 0;
	string counterStr = toString(counter);

	// Copy from input pattern to resulting string until we meet '#' sign
	while( it != pattern.end() && *it != '#') result += *it++;

	// Calculate number of '#' signs in input pattern
	while( it != pattern.end() && *it == '#')
	{
		it++;
		cntr++;
	}

	// If we not found any patterns - just return unmodified input pattern
	if( !cntr )
		return pattern;

	// Determine leading zeros
	if(cntr > counterStr.length())
	{
		// We have leading zeros
		cntr -= counterStr.length();
	}else{
		// Resulting counter bigger than pattern has defined,
		// therefore there are no leading zeros
		cntr = 0;
	}
	// cntr from now contains number of leading zeros

	// Append to result leading zeros
	if(cntr)
		result.append(cntr, '0');

	// Append integer value
	result.append(counterStr);

	// Copy from input pattern to resulting string rest of input pattern
	while( it != pattern.end() ) result += *it++;

	return result;
}

Config::Config()
{
	appendMode = false;	/* if not zero append YUV image(s) to output file */
	yuvFormat = YUV_YUV;	/* YUV output mode. Default: h2v2 */
	uvScale = SCALE_H1V1;	/* UV scaling for planar mode. Default: h1v1 */
	seqStart = 0;	/* Sequence start for multiple files */
	seqEnd = 0;		/* Sequence end for multiple files */
}


bool Config::ParseArgs(char* args[], int count)
{
	using namespace GetOpt;

	vector<string> files;
	string seqRangeOption;
	string yuvFormatOption;
	string uvScaleOption;
	bool error = true;

	GetOpt_pp opt(count, args);

	opt >> OptionPresent('a', appendMode);
	opt >> OptionPresent('i', uvInterleave);
	opt >> Option('m', seqRangeOption);
	opt >> Option('f', yuvFormatOption);
	opt >> Option('s', uvScaleOption);
	opt >> GlobalOption(files);

	if(files.size() > 0)
		inFileNamePattern = files[0];

	if(files.size() >= 1)
		outFileNamePattern = files[1];
	else
	{
		LOG_ERROR("You've not specified files to process...");
		goto HandleError;
	}

	// Scaling could be overridden by format, then select scale first
	if( uvScaleOption == "h2v2")
		uvScale = SCALE_H2V2;
	else if(uvScaleOption == "h1v1")
		uvScale = SCALE_H1V1;
	else if(uvScaleOption == "h1v2")
		uvScale = SCALE_H1V2;
	else if(uvScaleOption == "h2v1")
		uvScale = SCALE_H2V1;
	else if( !uvScaleOption.empty())
	{
		LOG_ERROR("Unknown UV scaling...");
		goto HandleError;
	}

	if( yuvFormatOption == "yuv")
		yuvFormat = YUV_YUV;
	else if(yuvFormatOption == "yvu")
		yuvFormat = YUV_YVU;
	else if(yuvFormatOption == "yuyv")
		yuvFormat = YUV_YUYV, uvScale = SCALE_H2V1; // Packed format always h2v1
	else if(yuvFormatOption == "yvyu")
		yuvFormat = YUV_YVYU, uvScale = SCALE_H2V1; // Packed format always h2v1
	else if(yuvFormatOption == "uyvy")
		yuvFormat = YUV_UYVY, uvScale = SCALE_H2V1; // Packed format always h2v1
	else if(yuvFormatOption == "vyuy")
		yuvFormat = YUV_VYUY, uvScale = SCALE_H2V1; // Packed format always h2v1
	else if( !yuvFormatOption.empty())
	{
		LOG_ERROR("Unknown YUV format...");
		goto HandleError;
	}

	if(!seqRangeOption.empty())
		if(!ParseSequenceRange(seqRangeOption))
		{
			LOG_ERROR("You've specified bad sequence range...");
			goto HandleError;
		}
	error = false;
HandleError:
	if(error)
		PrintHelp();

	return !error;
}

bool Config::ParseSequenceRange(string range)
{
	string::iterator it = range.begin();
	string seqStartOpt;
	string seqEndOpt;

	// Copy from input to seqStartOpt until we will find ':' character
	while(it != range.end() && *it != ':')
		seqStartOpt += *it++;

	if(it == range.end() || *it++ != ':')
		return false;

	// Copy from input to seqEndOpt till the end
	while(it != range.end())
		seqEndOpt += *it++;

	if(seqStartOpt.empty() || seqEndOpt.empty())
		return false;

	seqStart = atoi(seqStartOpt.c_str());
	seqEnd = atoi(seqEndOpt.c_str());

	return true;
}

void PrintHelp()
{
	printf(
		"\nUsage: yuvit [options] [-f format] [-s uvscale] <InFile> <OutFile>\n\n"
		"Options:\n"
		"   -a : Add new image to the end of output file. Don't truncate output file.\n"
		"   -m <start>:<end> : Multiple file input. Where:\n"
		"      start : Sequence start\n"
		"      end  : Sequence end\n"
		"	-i : Interleave UV rows for planar formats"
		"\nFormats (-f option):\n"
		"	yuv		: Planar format [DEFAULT]\n"
		"	yvu		: Planar format, reversed UV order\n"
		"	yuyv	: Packed format\n"
		"	yvyu	: Packed format\n"
		"	uyvy	: Packed format\n"
		"	vyuy	: Packed format\n"
		"\nUV scales (used only with -f and planar formats):\n"
		"	h1v1	: UV not scaled down [DEFAULT]\n"
		"   h2v2	: UV scaled down by 2x horizontally and vertically\n"
		"	h2v1	: UV scaled down by 2x horizontally\n"
		"	h1v2	: UV scaled down by 2x vertically\n"
		"\nNote: Use symbol '#' in file names for enumerators.\n"
		"\nExamples:\n"
		"yuvit -a -r=0:100 test###.bmp out.yuv\n"
		"		Convert images from 'test000.bmp' to 'test100.bmp' into single 'out.yuv' file\n"
		"\nyuvit -r=10:200 test######.jpg out###.yuv\n"
		"		Convert images 'test000010.jpg'...'test000200.jpg' into files 'out010.yuv'...'out200.yuv'\n"
		"\n"
		"Converter from different image types to YUV. v" APPVERSION " (C)2011 Shashkevych Alexander\n"
		);
}
