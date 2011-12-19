#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#include "FreeImage.h"

#define MAX_PATH 2048
#define MAJORVERSION "0"
#define MINORVERSION "1"
#define APPVERSION MAJORVERSION "." MINORVERSION

using namespace std;

void ArgParser(char* args[], int count);
int ArgGetIndex(char* argPos, char* args[]);
void PrintHelp();
int ExpandEnumerator(char *inString, char* outString, int enumValue, int enumSize);

int appendMode = 0;	/* if not zero append YUV image(s) to output file */
int yuvMode = 0;	/* YUV output mode. Default: h2v2 */
int multiMode = 0;	/* Flag of multi file mode */
int firstEnum = 0;	/* First enumerator */
int lastEnum = 0;	/* Last enumerator*/
int enumSize = 0;	/* Size of enumerator*/
int cSource = 0;	/* Make output as C/C++ source*/

char *origInFileName = 0, *origOutFileName = 0;
FILE* hOutFile;
char inFileName[MAX_PATH + 1], outFileName[MAX_PATH + 1];
int dollarWarnFlag = 0;

int main(int argc, char* argv[])
{
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

	FreeImage_Initialise();

	if(argc == 1)
	{
		PrintHelp();
		return 0;
	}

	ArgParser(argv, argc);

	if(yuvMode == -1)
	{
		printf("ERROR: Unknown output mode...\n");
		PrintHelp();
		goto HandleError;
	}

	if(origInFileName == 0)
	{
		printf("ERROR: Input filename not specified...\n");
		PrintHelp();
		goto HandleError;
	}

	if(origOutFileName == 0)
	{
		printf("ERROR: Input filename not specified...\n");
		PrintHelp();
		goto HandleError;
	}

	if(multiMode != 0 && firstEnum >= lastEnum)
	{
		printf("ERROR: Wrong enumeration parameters in multifile option...\n");
		goto HandleError;
	}

	while(firstEnum <= lastEnum)
	{	/* Main cycle for converting all images */

		if(!multiMode)
		{ /* If not Multi file mode don't use enumerators expansion */
			strcpy(inFileName, origInFileName);
			strcpy(outFileName, origOutFileName);
		}else{
			/* Expand enumerators in filenames */
			if(ExpandEnumerator(origInFileName, inFileName, firstEnum, enumSize) == 0 &&
				dollarWarnFlag != 1)
			{
				dollarWarnFlag = 1;
				printf("Warning: Input file name not have enumeration char '$'.\n");
			}
			ExpandEnumerator(origOutFileName, outFileName, firstEnum, enumSize);
		}

		inImage = corona::OpenImage(inFileName, corona::PF_R8G8B8);
		if(inImage == 0)
		{
			printf("ERROR: Can not open input file...\n");
			goto handleError;
		}

		if(appendMode == 0)
			hOutFile = fopen(outFileName,"wb"); /* Write in new file */
		else
			hOutFile = fopen(outFileName,"ab"); /* Add to existing file */

		if(hOutFile == 0)
		{
			delete inImage;
			printf("ERROR: Can not open output file...\n");
			goto error;
		}

		printf("%s\n", inFileName);

		rgbPixels = (unsigned char*)inImage->getPixels();
		lumaWidth = inImage->getWidth();
		lumaHeight = inImage->getHeight();

		/* Calculate dimensions */
		switch(yuvMode)
		{
		default:
		case 0: /* H2V2 */
			chromaWidth = lumaWidth / 2;
			chromaHeight = lumaHeight / 2;
			yMask = xMask = 1;
			break;
		case 1: /* H2V1 */
			chromaWidth = lumaWidth;
			chromaHeight = lumaHeight / 2;
			xMask = 0;
			yMask = 1;
			break;
		case 2: /* H1V2 */
			chromaWidth = lumaWidth / 2;
			chromaHeight = lumaHeight;
			xMask = 1;
			yMask = 0;
			break;
		case 3: /* H1V1 */
			chromaWidth = lumaWidth;
			chromaHeight = lumaHeight;
			xMask = 0;
			yMask = 0;
			break;
		}

		yPixels = (unsigned char*)malloc(lumaHeight * lumaWidth);
		uPixels = (unsigned char*)malloc(chromaHeight * chromaWidth);
		vPixels = (unsigned char*)malloc(chromaHeight * chromaWidth);

		/* Converting cycle */
		yPtr = yPixels;
		uPtr = uPixels;
		vPtr = vPixels;

		for(y = 0; y < lumaHeight; y++)
		{
			for(x = 0; x < lumaWidth; x++)
			{
				Rc = *rgbPixels++;
				Gc = *rgbPixels++;
				Bc = *rgbPixels++;

				*yPtr++ = (0.257f * Rc) + (0.504f * Gc) + (0.098f * Bc) + 16;
				if((y & yMask) == 0 && (x & xMask) == 0)
				{
					*uPtr++ = -(0.148f * Rc) - (0.291f * Gc) + (0.439f * Bc) + 128;
					*vPtr++ = (0.439f * Rc) - (0.368f * Gc) - (0.071f * Bc) + 128;
				}
			}
		}

		/* Write converted image to file */
		if(cSource == 1)
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
			fwrite(uPixels, 1, chromaWidth * chromaHeight, hOutFile);
			fwrite(vPixels, 1, chromaWidth * chromaHeight, hOutFile);
		}

		delete inImage;
		fclose(hOutFile);
		free(yPixels);
		free(uPixels);
		free(vPixels);

		firstEnum++;
	} /* while(firstEnum <= lastEnum)*/

	printf("Done.\n");

	errorFlag = 0; // We successful passed all stages, so set flag to zero which means - OK

HandleError:
	FreeImage_DeInitialise();

	return errorFlag;
}

/*
	
 Description: Function searches '#' chars and replaces it with enumerator
 Returns: Count of found '#' symbols

*/
int ExpandEnumerator(char *inString, char* outString, int enumValue, int enumSize)
{
	int count = 0;
	char formatString[1024];

	sprintf(formatString, "%%0%dd", enumSize);

	while(*inString != 0)
	{
		if(*inString == '#')
		{
			sprintf(outString, formatString, enumValue);
			outString += enumSize;
			inString++;
			count++;
		}else{
			*outString++ = *inString++;
		}
	}
	return count;
}

void PrintHelp()
{
	printf(
		"-=======================================================================-\n"
		"  Any 2 YUV convertor v" APPVERSION " (C)2006 Shashkevych Alexander\n"
		"-=======================================================================-\n"
		"Usage: any2yuv.exe [options] <InFile> <OutFile>\n\n"
		"Options:\n"
		"   -a : Add new image to the end of output file. Don't truncate output file.\n"
		"   -t <h2v2|h2v1|h1v2|h1v1|uvyv|yuyv> : Type of output YUV image(s).\n"
		"   -r <from>:<to> : Multiple input file mode. Where:\n"
		"      first : Start counter value\n"
		"      last  : End counter value\n"
		"   -c : Output as C/C++ source. Default: binary mode\n"
		"\nNote: Use symbol '#' in file names for enumerators.\n"
		"\nExamples:\n"
		"any2yuv.exe -a -r 0:100 test###.bmp out.yuv\n"
		"   Convert images from 'test000.bmp' to 'test100.bmp' into single 'out.yuv' file\n"
		"\nany2yuv.exe -r 10:200 test#####.jpg out###.yuv\n"
		"   Convert images 'test00010.jpg'...'test00200.jpg' into files 'out010.yuv'...'out200.yuv'\n"
		);
}

char *appArgs[] = {
	"-a", /* New converted image will be added to the end of output file */
	"-t", /* output YUV type can be h2v2, h2v1, h1v2, h1v1, uyvy, yuyv */
	"-r", /* Mutifle mode */
	"-c",
	0
};

char *modeArgs [] = {
	"h2v2", "h2v1", "h1v2", "h1v1", "yuvu", "uyvy", 0
};

void ArgParser(char* args[], int count)
{
	int pos = 1, index;

	while(pos < count)
	{
		index = ArgGetIndex(args[pos], appArgs);
		switch(index)
		{
		case 0: /* Add mode */
			appendMode = 1;
			break;
		case 1: /* Output type */
			pos++;
			yuvMode = ArgGetIndex(args[pos], modeArgs);
			break;
		case 2:	/* Mutlifile mode */
			multiMode = 1;
			firstEnum = atoi(args[++pos]);
			lastEnum = atoi(args[++pos]);
			enumSize = atoi(args[++pos]);
			break;
		case 3:
			cSource = 1;
			break;
		case -1: /* filenames */
			if(origInFileName == 0)
			{
				origInFileName = args[pos];
			}else if(origOutFileName == 0){
				origOutFileName = args[pos];
			}else{
				printf("ERROR: Unknown argument '%s'...\n", args[pos]);
			}
			break;
		}
		pos++;
	}
}

int ArgGetIndex(char* arg, char* appArgs[])
{
	int pos = 0;
	
	while(appArgs[pos] != 0)
	{
		if(strcmp(appArgs[pos], arg) == 0)
			return pos;
		pos++;
	}

	return -1;
}
