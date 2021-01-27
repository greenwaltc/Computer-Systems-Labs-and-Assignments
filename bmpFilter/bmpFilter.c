#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE  1
#define FALSE 0

#define BAD_NUMBER_ARGS 1
#define FSEEK_ERROR 2
#define FREAD_ERROR 3
#define MALLOC_ERROR 4
#define FWRITE_ERROR 5

//#define DEBUG
/**
 * Parses the command line.
 *
 * argc:      the number of items on the command line (and length of the
 *            argv array) including the executable
 * argv:      the array of arguments as strings (char* array)
 * grayscale: the integer value is set to TRUE if grayscale output indicated
 *            outherwise FALSE for threshold output
 *
 * returns the input file pointer (FILE*)
 **/
FILE *parseCommandLine(int argc, char **argv, int *isGrayscale) {
  if (argc > 2) {
    printf("Usage: %s [-g]\n", argv[0]);
    exit(BAD_NUMBER_ARGS);
  }
  
  if (argc == 2 && strcmp(argv[1], "-g") == 0) {
    *isGrayscale = TRUE;
  } else {
    *isGrayscale = FALSE;
  }

  return stdin;
}

unsigned getFileSizeInBytes(FILE* stream) {
  unsigned fileSizeInBytes = 0;
  
  rewind(stream);
  if (fseek(stream, 0L, SEEK_END) != 0) {
    exit(FSEEK_ERROR);
  }
  fileSizeInBytes = ftell(stream);

  return fileSizeInBytes;
}

void getBmpFileAsBytes(unsigned char* ptr, unsigned fileSizeInBytes, FILE* stream) {
  rewind(stream);
  if (fread(ptr, fileSizeInBytes, 1, stream) != 1) {
#ifdef DEBUG
    printf("feof() = %x\n", feof(stream));
    printf("ferror() = %x\n", ferror(stream));
#endif
    exit(FREAD_ERROR);
  }
}

unsigned char getAverageIntensity(unsigned char blue, unsigned char green, unsigned char red) {
  //printf("TODO: unsigned char getAverageIntensity(unsigned char blue, unsigned char green, unsigned char red)\n");
  return ((blue + green + red) / 3);
}

void applyGrayscaleToPixel(unsigned char* pixel) {
  //printf("TODO: void applyGrayscaleToPixel(unsigned char* pixel)\n");
  
  unsigned char avg = getAverageIntensity(pixel[0], pixel[1], pixel[2]);
    pixel[0] = avg;
    pixel[1] = avg;
    pixel[2] = avg;
}

void applyThresholdToPixel(unsigned char* pixel) {
  //printf("TODO: void applyThresholdToPixel(unsigned char* pixel)\n");
  unsigned char avg = getAverageIntensity(pixel[0], pixel[1], pixel[2]);
  if (avg >= 128){
    pixel[0] = 0xFF;
    pixel[1] = 0xFF;
    pixel[2] = 0xFF;
  }
  else{
    pixel[0] = 0x00;
    pixel[1] = 0x00;
    pixel[2] = 0x00;
  }
}

void applyFilterToPixel(unsigned char* pixel, int isGrayscale) {
  //printf("TODO: void applyFilterToPixel(unsigned char* pixel, int isGrayscale)\n");
  
  if(isGrayscale){
    applyGrayscaleToPixel(pixel);
  }
  else{
    applyThresholdToPixel(pixel);
  }
}

void applyFilterToRow(unsigned char* row, int width, int isGrayscale) {
  //printf("TODO: void applyFilterToRow(unsigned char* row, int width, int isGrayscale)\n");
  for (int i = 0; i < width; i++){
    int offset = i * 3;
    applyFilterToPixel( (row + offset), isGrayscale);
  }
}

void applyFilterToPixelArray(unsigned char* pixelArray, int width, int height, int isGrayscale) {
  //printf("TODO: void applyFilterToPixelArray(unsigned char* pixelArray, int width, int height, int isGrayscale)\n");
  
  int padding = 4 - ((width * 3) % 4);
  //If there's no padding, padding will end up being 4, which is not what it needs to be
  //so I included this if statement to reassign in that case
  if (padding == 4){
    padding = 0;
  }
  
  int widthInBytesPerRow = (width*3) + padding;
  
  for(int i = 0; i < height; i++){
    int offset = i * widthInBytesPerRow;
    applyFilterToRow( (pixelArray + offset), width, isGrayscale);
  }
}

void parseHeaderAndApplyFilter(unsigned char* bmpFileAsBytes, int isGrayscale) {
  int offsetFirstBytePixelArray = 0;
  int width = 0;
  int height = 0;
  unsigned char* pixelArray = NULL;

  //printf("TODO: set offsetFirstBytePixelArray\n");
  offsetFirstBytePixelArray = *((int*) (bmpFileAsBytes + 10));

  //printf("TODO: set width\n");
  width = *((int*) (bmpFileAsBytes + 18));
  
  //printf("TODO: set height\n");
  height = *((int*) (bmpFileAsBytes + 22));
  
  //printf("TODO: set the pixelArray to the start of the pixel array\n");
  pixelArray = bmpFileAsBytes + offsetFirstBytePixelArray;

#ifdef DEBUG
  printf("offsetFirstBytePixelArray = %u\n", offsetFirstBytePixelArray);
  printf("width = %u\n", width);
  printf("height = %u\n", height);
  printf("pixelArray = %p\n", pixelArray);
#endif

  applyFilterToPixelArray(pixelArray, width, height, isGrayscale);
}

int main(int argc, char **argv) {
  int grayscale = FALSE;
  unsigned fileSizeInBytes = 0;
  unsigned char* bmpFileAsBytes = NULL;
  FILE *stream = NULL;
  
  stream = parseCommandLine(argc, argv, &grayscale);
  fileSizeInBytes = getFileSizeInBytes(stream);

#ifdef DEBUG
  printf("fileSizeInBytes = %u\n", fileSizeInBytes);
#endif

  bmpFileAsBytes = (unsigned char *)malloc(fileSizeInBytes);
  if (bmpFileAsBytes == NULL) {
    exit(MALLOC_ERROR);
  }
  getBmpFileAsBytes(bmpFileAsBytes, fileSizeInBytes, stream);

  parseHeaderAndApplyFilter(bmpFileAsBytes, grayscale);


  if (fwrite(bmpFileAsBytes, fileSizeInBytes, 1, stdout) != 1) {
    printf("Exiting - unable to write");
    exit(FWRITE_ERROR);
  }

  free(bmpFileAsBytes);
  return 0;
}