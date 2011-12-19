/* Disable Visual Studio 2oo5 warnings */
#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "corona.h"

#define MAJORVERSION "1"
#define MINORVERSION "0"
#define APPVERSION MAJORVERSION "." MINORVERSION

#pragma comment(lib, "corona_static.lib")

void ArgParser(char* args[], int count);
int ArgGetIndex(char* argPos, char* args[]);
void PrintHelp();
int ExpandEnumerator(char *inString, char* outString, int enumValue, int enumSize);

int addMode = 0;	/* if not zero add new YUV image to existing file */
int yuvMode = 0;	/* YUV output mode. Default: h2v2 */
int multiMode = 0;	/* Flag of multi file mode */
int firstEnum = 0;	/* First enumerator */
int lastEnum = 0;	/* Last enumerator*/
int enumSize = 0;	/* Size of enumerator*/
int cSource = 0;	/* Make output as C/C++ source*/

char *origInFileName = 0, *origOutFileName = 0;
FILE* hOutFile;
char inFileName[1024], outFileName[1024];
int dollarWarnFlag = 0;

int main(int argc, char* argv[])
{
	corona::Image *inImage;
	int lumaWidth, lumaHeight;
	int chromaWidth, chromaHeight;
	int size;
	int x, y, xMask, yMask;
	unsigned char Rc, Gc, Bc;
	unsigned char* rgbPixels;
	unsigned char *yPixels, *uPixels, *vPixels;
	unsigned char *yPtr, *uPtr, *vPtr;

	if(argc == 1)
	{
		PrintHelp();
		return 0;
	}

	ArgParser(argv, argc);

	if(yuvMode == -1)
	{
		printf("Error: Unknown output mode...\n");
		PrintHelp();
		return 0;
	}

	if(origInFileName == 0)
	{
		printf("Error: Input filename not specified...\n");
		PrintHelp();
		return 0;
	}

	if(origOutFileName == 0)
	{
		printf("Error: Input filename not specified...\n");
		PrintHelp();
		return 0;
	}

	if(multiMode != 0 && firstEnum >= lastEnum)
	{
		printf("Error: Wrong enumeration parameters in multifile option...\n");
		return 0;
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
			printf("Error: Can not open input file...\n");
			return 0;
		}

		if(addMode == 0)
			hOutFile = fopen(outFileName,"wb"); /* Write in new file */
		else
			hOutFile = fopen(outFileName,"ab"); /* Add to existing file */

		if(hOutFile == 0)
		{
			delete inImage;
			printf("Error: Can not open output file...\n");
			return 0;
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
			chromaHeight = lumaHeight / 2;
			xMask = 0;
			yMask = 0;
			break;
		}

		yPixels = (unsigned char*)malloc(lumaHeight * lumaWidth);
		uPixels = (unsigned char*)malloc(chromaHeight * chromaWidth);
		vPixels = (unsigned char*)malloc(chromaHeight * chromaWidth);

		/* Do convert */
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
	return 0;
}

/*
	
 Description: Function searches '$' chars and replaces it with enumerator 
 Returns: Quantity of finded '$' chars

*/
int ExpandEnumerator(char *inString, char* outString, int enumValue, int enumSize)
{
	int count = 0;
	char formatString[1024];

	sprintf(formatString, "%%0%dd", enumSize);

	while(*inString != 0)
	{
		if(*inString == '$')
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
		"   -m <first> <last> <n> : Multiple input file mode. Where:\n"
		"      first : first enumerator\n"
		"      last  : last enumerator\n"
		"      n     : size of enumerator (3 for 001, 5 for 00001)\n"
		"   -c : Output as C/C++ source. Default: binary mode\n"
		"\nNote: Use symbol '$' in file names for enumerators.\n"
		"\nExamples:\n"
		"any2yuv.exe -a -m 0 100 3 test$.bmp out.yuv\n"
		"   Convert images from 'test000.bmp' to 'test100.bmp' into single 'out.yuv' file\n"
		"\nany2yuv.exe -m 10 200 5 test$.jpg out$.yuv\n"
		"   Convert images 'test00010.jpg'...'test00200.jpg' into files 'out00010.yuv'...'out00200.yuv'\n"
		);
}

char *appArgs[] = {
	"-a", /* New converted image will be added to the end of output file */
	"-t", /* output YUV type can be h2v2, h2v1, h1v2, h1v1, uyvy, yuyv */
	"-m", /* Mutifle mode */
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
			addMode = 1;
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
				printf("Error: Unknown argument '%s'...\n", args[pos]);
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
