#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

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

void print_all(BITMAPFILEHEADER *fileHeader, BITMAPINFOHEADER *fileInfo);
void print_hist(float blue[], float green[], float red[]);
void free_all(unsigned char **arr, LONG len);

int main(int argc, char *argv[]) {
    if (argc != 2 && argc != 3 && argc != 4) {
        fprintf(stderr, "Usage: %s <PATH-TO-BMP-FILE>\n"
                        "or: %s <PATH-TO-INPUT-BMP-FILE> <PATH-TO-OUTPUT-BMP-FILE>\n"
                        "or: %s <PATH-TO-INPUT-BMP> <PATH-TO-ENCODED-BMP> \"text to be hidden\"\n", argv[0], argv[0], argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error");
        return 1;
    }

    // write file header into the structure
    BITMAPFILEHEADER fileHeader;
    if (fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, file) != 1) {
        fprintf(stderr, "Error reading file header\n");
        fclose(file);
        return 1;
    }

    // write file info header into the structure
    BITMAPINFOHEADER fileInfo;
    if (fread(&fileInfo, sizeof(BITMAPINFOHEADER), 1, file) != 1) {
        fprintf(stderr, "Error reading info header\n");
        fclose(file);
        return 1;
    }

    // Move file pointer to the begining of the pixel array and save some content for the sake of applying filter
    int x = fileInfo.biSize - sizeof(BITMAPINFOHEADER);
    unsigned char *tail = malloc(x * sizeof(unsigned char));
    fread(tail, sizeof(unsigned char), x, file);

    int row_len = (24 * fileInfo.biWidth + 31) / 32 * 4;
    unsigned char row[row_len];

    if (argc == 2) {
        free(tail);
        print_all(&fileHeader, &fileInfo);
        char decode;
        do {
            printf("Decode steganography? [y/n] ");
            scanf(" %c", &decode);
            decode = tolower(decode);
        } while (decode != 'y' && decode != 'n');

        char decode_len = 8;
        unsigned char encd_len = 0;
        int msg_pos = 0;
        char chr_pos = 0;
        unsigned char ch;
        char *msg;

        if (fileInfo.biBitCount == 24 && fileInfo.biCompression == 0) {
            float blue[16] = {0};
            float green[16] = {0};
            float red[16] = {0};

            for (int i=0; i < fileInfo.biHeight; i++) {
                fread(row, sizeof(unsigned char), row_len, file);
                if (decode == 'y') {
                    int j = 0;
                    for (; j < fileInfo.biWidth*3 && decode_len != 0; j++, chr_pos++, decode_len--) {
                        encd_len += (unsigned char)pow(2, chr_pos) * (row[j] & 1);
                        if (chr_pos == 7) {
                            chr_pos = -1;
                            msg = malloc((encd_len+1) * sizeof(char));
                        }
                    }

                    for (; j < fileInfo.biWidth*3 && msg_pos < encd_len; j++, chr_pos++) {
                        if (chr_pos == 0) {
                            ch = 0;
                        }

                        ch += pow(2, chr_pos) * (row[j] & 1);
                        if (chr_pos == 7) {
                            chr_pos = -1;
                            msg[msg_pos] = ch;
                            msg_pos++;
                        }
                    }
                }                

                for (int j=0; j < fileInfo.biWidth*3; j+=3) {
                    blue[row[j] / 16]++;
                    green[row[j+1] / 16]++;
                    red[row[j+2] / 16]++;
                }
            }

            fclose(file);
            int normb = 0, normg = 0, normr = 0;
            for (int i=0; i < 16; i++) {
                normb += blue[i];
                normg += green[i];
                normr += red[i];
            }

            for (int i=0; i < 16; i++) {
                blue[i] /= normb;
                green[i] /= normg;
                red[i] /= normr;
            }

            print_hist(blue, green, red);
            if (decode == 'y') {
                msg[msg_pos] = '\0';
                printf("\n%s\n", msg);
                free(msg);
            }
        } else {
            printf("Unsupported file\n");
        }
    } else if (argc == 3) {
        // ** because it is a pointer to pointers to arrays of uchars
        unsigned char **pixel_arr = malloc(fileInfo.biHeight * sizeof(unsigned char *));
        int avg;
        for (int i=0; i < fileInfo.biHeight; i++) {
            fread(row, sizeof(unsigned char), row_len, file);
            pixel_arr[i] = (unsigned char *)malloc(row_len * sizeof(unsigned char));
            for (int j=0; j < fileInfo.biWidth*3; j+=3) {
                avg = 0;
                avg += row[j];
                avg += row[j+1];
                avg += row[j+2];
                
                avg /= 3;
                
                row[j] = avg;
                row[j+1] = avg;
                row[j+2] = avg;
            }

            memcpy(pixel_arr[i], row, row_len * sizeof(unsigned char));
        }
        
        fclose(file);
        FILE *out = fopen(argv[2], "wb");
        if (out == NULL) {
            fprintf(stderr, "Error opening output file.\n");
            free_all(pixel_arr, fileInfo.biHeight);
            return 1;
        }

        fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, out);
        fwrite(&fileInfo, sizeof(BITMAPINFOHEADER), 1, out);
        fwrite(tail, sizeof(unsigned char), x, out);
        free(tail);
        for (int i=0; i < fileInfo.biHeight; i++) {
            fwrite(pixel_arr[i], row_len * sizeof(unsigned char), 1, out);
        }

        fclose(out);
        free_all(pixel_arr, fileInfo.biHeight);
    } else {
        // ** because it is a pointer to pointers to arrays of uchars
        if (strlen(argv[3]) > 255) {
            printf("The maximum length of message to encrypt is 255 characters\n");
            free(tail);
            return 1;
        }

        unsigned char **pixel_arr = malloc(fileInfo.biHeight * sizeof(unsigned char *));
        unsigned char len = strlen(argv[3]);
        unsigned char bit_len = strlen(argv[3]);
        unsigned char encd_len = 0;
        unsigned char msg_pos = 0;
        char chr_pos = 0;
        unsigned char ch;

        for (int i=0; i < fileInfo.biHeight; i++) {
            fread(row, sizeof(unsigned char), row_len, file);
            pixel_arr[i] = (unsigned char *)malloc(row_len * sizeof(unsigned char));
            // encode length of message
            int j = 0;
            for (; j < fileInfo.biWidth*3 && encd_len < 8; j++, encd_len++) {
                row[j] = (row[j] & 254) | (bit_len & 1);
                bit_len >>= 1;
            }

            // encode message character by character
            for (; j < fileInfo.biWidth*3 && msg_pos < len; j++, chr_pos++) {
                if (chr_pos == 0) {
                    ch = argv[3][msg_pos];
                }

                row[j] = (row[j] & 254) | (ch & 1);
                ch >>= 1;

                if (chr_pos == 7) {
                    chr_pos = -1;
                    msg_pos++;
                }
            }

            memcpy(pixel_arr[i], row, row_len * sizeof(unsigned char));
        }

        fclose(file);
        FILE *out = fopen(argv[2], "wb");
        if (out == NULL) {
            fprintf(stderr, "Error opening output file.\n");
            free_all(pixel_arr, fileInfo.biHeight);
            return 1;
        }

        fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, out);
        fwrite(&fileInfo, sizeof(BITMAPINFOHEADER), 1, out);
        fwrite(tail, sizeof(unsigned char), x, out);
        free(tail);
        for (int i=0; i < fileInfo.biHeight; i++) {
            fwrite(pixel_arr[i], row_len * sizeof(unsigned char), 1, out);
        }

        fclose(out);
        free_all(pixel_arr, fileInfo.biHeight);
    }

    return 0;
}

void print_all(BITMAPFILEHEADER *fileHeader, BITMAPINFOHEADER *fileInfo) {
    printf("BITMAPFILEHEADER:\n");
    printf("    bftype:          0x%x (%c%c)\n", fileHeader->bfType, (char)(fileHeader->bfType & 0xFF), (char)((fileHeader->bfType >> 8) & 0xFF));
    printf("    bfsize:          %d\n", fileHeader->bfSize);
    printf("    bfReserved1:     0x%x\n", fileHeader->bfReserved1);
    printf("    bfReserved2:     0x%x\n", fileHeader->bfReserved2);
    printf("    bfOffBits:       %d\n", fileHeader->bfOffBits);
    printf("BITMAPINFOHEADER:\n");
    printf("    biSize:          %d\n", fileInfo->biSize);
    printf("    biWidth:         %d\n", fileInfo->biWidth);
    printf("    biHeight:        %d\n", fileInfo->biHeight);
    printf("    biPlanes:        %d\n", fileInfo->biPlanes);
    printf("    biBitCount:      %d\n", fileInfo->biBitCount);
    printf("    biCompression:   %d\n", fileInfo->biCompression);
    printf("    biSizeImage:     %d\n", fileInfo->biSizeImage);
    printf("    biXPelsPerMeter: %d\n", fileInfo->biXPelsPerMeter);
    printf("    biYPelsPerMeter: %d\n", fileInfo->biYPelsPerMeter);
    printf("    biClrUsed:       %d\n", fileInfo->biClrUsed);
    printf("    biClrImportant:  %d\n\n", fileInfo->biClrImportant);
}

void print_hist(float blue[], float green[], float red[]) {
    printf("Blue:\n");
    unsigned char range = 0;
    for (int i=0; i < 16; i++) {
        printf("  %d-%d: %.2f%%\n", range, range+15, blue[i]*100);
        range += 16;
    }
    
    printf("Green:\n");
    range = 0;
    for (int i=0; i < 16; i++) {
        printf("  %d-%d: %.2f%%\n", range, range+15, green[i]*100);
        range += 16;
    }
    
    printf("Red:\n");
    range = 0;
    for (int i=0; i < 16; i++) {
        printf("  %d-%d: %.2f%%\n", range, range+15, red[i]*100);
        range += 16;
    }
}

void free_all(unsigned char **arr, LONG len) {
    for (int i=0; i < len; i++) {
        free(arr[i]);
    }

    free(arr);
}