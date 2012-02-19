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

enum YUVMode {
	YUV_H2V2,
	YUV_H2V1,
	YUV_H1V2,
	YUV_H1V1,
	YUV_UYVY,
	YUV_YUVU
};

class Config
{
public:
	bool appendMode;			/* if not zero append YUV image(s) to output file */
	bool cSource;				/* Make output as C/C++ source */
	uint32_t yuvType;			/* YUV output mode. Default: h2v2 */
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
	int size;
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
		switch(cfg.yuvType)
		{
		default:
		case YUV_H2V2:
			chromaWidth = lumaWidth / 2;
			chromaHeight = lumaHeight / 2;
			yMask = xMask = 1;
			break;
		case YUV_H2V1:
			chromaWidth = lumaWidth;
			chromaHeight = lumaHeight /2;
			xMask = 0;
			yMask = 1;
			break;
		case YUV_H1V2:
			chromaWidth = lumaWidth / 2;
			chromaHeight = lumaHeight;
			xMask = 1;
			yMask = 0;
			break;
		case YUV_H1V1:
			chromaWidth = lumaWidth;
			chromaHeight = lumaHeight;
			xMask = 0;
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

		/* Write converted image to file */
		if(cfg.cSource == 1)
		{
			fprintf(hOutFile, "unsigned char yuvImage[%d] = {\n\t", lumaWidth * lumaHeight + chromaWidth * chromaHeight * 2);

			yPtr = yPixels;
			uPtr = uPixels;
			vPtr = vPixels;
			x = 0;
			size = lumaWidth * lumaHeight;

			while(size--)
			{
				if(x >= 16)
				{
					x = 0;
					fprintf(hOutFile, "\n\t");
				}
				fprintf(hOutFile, "0x%02x, ", *yPtr++);
				x++;
			}

			size = chromaWidth * chromaHeight;
			while(size--)
			{
				if(x >= 16)
				{
					x = 0;
					fprintf(hOutFile, "\n\t");
				}
				fprintf(hOutFile, "0x%02x, ", *uPtr++);
				x++;
			}

			size = chromaWidth * chromaHeight;
			while(size--)
			{
				if(x >= 16)
				{
					x = 0;
					fprintf(hOutFile, "\n\t");
				}
				if(size)
					fprintf(hOutFile, "0x%02x, ", *vPtr++);
				else
					fprintf(hOutFile, "0x%02x", *vPtr++);
				x++;
			}
			fprintf(hOutFile, "\n};\n");
		}else{
			fwrite(yPixels, 1, lumaWidth * lumaHeight, hOutFile);

			fwrite(vPixels, 1, chromaWidth * chromaHeight, hOutFile);
			fwrite(uPixels, 1, chromaWidth * chromaHeight, hOutFile);
		}

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
	yuvType = YUV_H2V2;	/* YUV output mode. Default: h2v2 */
	cSource = false;	/* Make output as C/C++ source */
	seqStart = 0;	/* Sequence start for multiple files */
	seqEnd = 0;		/* Sequence end for multiple files */
}


bool Config::ParseArgs(char* args[], int count)
{
	using namespace GetOpt;

	vector<string> files;
	string seqRangeOption;
	string yuvTypeOption;
	bool error = true;

	GetOpt_pp opt(count, args);

	opt >> OptionPresent('a', appendMode);
	opt >> OptionPresent('c', cSource);
	opt >> Option('r', seqRangeOption);
	opt >> Option('t', yuvTypeOption);
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

	if( yuvTypeOption == "h2v2")
		yuvType = YUV_H2V2;
	else if(yuvTypeOption == "h1v1")
		yuvType = YUV_H1V1;
	else if(yuvTypeOption == "h1v2")
		yuvType = YUV_H1V2;
	else if(yuvTypeOption == "h2v1")
		yuvType = YUV_H2V1;
	else if( !yuvTypeOption.empty())
	{
		LOG_ERROR("Unknown YUV type...");
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
		"\nUsage: yuvit [options] <InFile> <OutFile>\n\n"
		"Options:\n"
		"   -a : Add new image to the end of output file. Don't truncate output file.\n"
		"   -t <h2v2|h2v1|h1v2|h1v1|uvyv|yuyv> : Type of output YUV image(s).\n"
		"   -r <start>:<end> : Process multiple input files. Where:\n"
		"      start : Sequence start\n"
		"      end  : Sequence end\n"
		"   -c : Output as C/C++ source. Default: binary mode\n"
		"\nNote: Use symbol '#' in file names for enumerators.\n"
		"\nExamples:\n"
		"yuvit -a -r=0:100 test###.bmp out.yuv\n"
		"   Convert images from 'test000.bmp' to 'test100.bmp' into single 'out.yuv' file\n"
		"\nyuvit.exe -r=10:200 test######.jpg out###.yuv\n"
		"   Convert images 'test000010.jpg'...'test000200.jpg' into files 'out010.yuv'...'out200.yuv'\n"
		"\n"
		"Converter from different image types to YUV. v" APPVERSION " (C)2011 Shashkevych Alexander\n"
		);
}
