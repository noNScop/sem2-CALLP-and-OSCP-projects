#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;

// https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapfileheader
typedef struct tagBITMAPFILEHEADER {
  WORD  bfType;
  DWORD bfSize;
  WORD  bfReserved1;
  WORD  bfReserved2;
  DWORD bfOffBits;
} __attribute__((packed)) BITMAPFILEHEADER, *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;

// https://docs.microsoft.com/pl-pl/previous-versions/dd183376(v=vs.85)
typedef struct tagBITMAPINFOHEADER {
  DWORD biSize;
  LONG  biWidth;
  LONG  biHeight;
  WORD  biPlanes;
  WORD  biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG  biXPelsPerMeter;
  LONG  biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} __attribute__((packed)) BITMAPINFOHEADER, *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

int fractal_iter(double x, double y);
void free_all(unsigned char **arr, LONG len);

int MAX_ITERATIONS=100;

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <WIDTH> <HEIGHT> <PATH-TO-BMP-FILE>\n", argv[0]);
        return 1;
    }

    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;
    
    infoHeader.biSize = sizeof(BITMAPINFOHEADER);
    infoHeader.biWidth = atoi(argv[1]);
    infoHeader.biHeight = atoi(argv[2]);
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24;
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = 3 * infoHeader.biWidth * infoHeader.biHeight;
    infoHeader.biXPelsPerMeter = 0;
    infoHeader.biYPelsPerMeter = 0;
    infoHeader.biClrUsed = 0;
    infoHeader.biClrImportant = 0;

    fileHeader.bfType = 0x4D42;
    int pxarr_wd = (infoHeader.biBitCount * infoHeader.biWidth + 31) / 32 * 4;
    int pxarr_sz = infoHeader.biHeight * pxarr_wd;
    fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + pxarr_sz;
    fileHeader.bfReserved1 = 0;
    fileHeader.bfReserved2 = 0;
    fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    unsigned char **pixel_arr = malloc(infoHeader.biHeight * sizeof(unsigned char *));
    for (int i=0; i < infoHeader.biHeight; i++) {
        pixel_arr[i] = malloc(pxarr_wd * sizeof(unsigned char));
        for (int j=0; j < infoHeader.biWidth*3; j+=3) {
            double x = (j/3 + 1)*3 / (double)infoHeader.biWidth - 2;
            double y = (i+1)*2 / (double)infoHeader.biHeight - 1;
            int it = fractal_iter(x, y);
            uint8_t col = (double)it / MAX_ITERATIONS * 255;
            pixel_arr[i][j] = col;
            pixel_arr[i][j+1] = col;
            pixel_arr[i][j+2] = col;
        }
    }

    FILE *out = fopen(argv[3], "wb");
    if (out == NULL) {
        fprintf(stderr, "Error opening output file.\n");
        free_all(pixel_arr, infoHeader.biHeight);
        return 1;
    }

    fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, out);
    fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, out);
    for (int i=0; i < infoHeader.biHeight; i++) {
        fwrite(pixel_arr[i], pxarr_wd * sizeof(unsigned char), 1, out);
    }

    fclose(out);
    free_all(pixel_arr, infoHeader.biHeight);
    return 0;
}

int fractal_iter(double x0, double y0) {
    int iterations = 0;
    double x = 0, y = 0, xtemp;
    while (iterations < MAX_ITERATIONS && x*x + y*y <= 4) {
        xtemp = x*x - y*y + x0;
        y = 2*x*y + y0;
        x = xtemp;
        iterations++;
    }

    return iterations;
}

void free_all(unsigned char **arr, LONG len) {
    for (int i=0; i < len; i++) {
        free(arr[i]);
    }

    free(arr);
}