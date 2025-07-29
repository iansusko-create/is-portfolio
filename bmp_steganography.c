#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(1)
typedef struct {
    unsigned char header[2]; // "BM"
    unsigned int size;       // Size of the file
    unsigned short reserved1; // Reserved
    unsigned short reserved2; // Reserved
    unsigned int offset;     // Offset to pixel data
} BMPHeader;

typedef struct {
    unsigned int size;          // Size of this header
    unsigned int width;         // Width of the image
    unsigned int height;        // Height of the image
    unsigned short planes;      // Number of color planes
    unsigned short bitsPerPixel; // Bits per pixel
    unsigned int compression;    // Compression scheme
    unsigned int imageSize;      // Image size
    unsigned int xResolution;     // Horizontal resolution
    unsigned int yResolution;     // Vertical resolution
    unsigned int colors;         // Number of colors in the palette
    unsigned int importantColors; // Important colors
} DIBHeader;
#pragma pack()

// Function to handle error messages
void printError(const char *message) {
    fprintf(stderr, "ERROR: %s\n", message);
}

void info(const char *filename) {
   FILE *file = fopen(filename, "rb"); // open in read-binary
    if (!file) {
        printError("File not found.");
        return;
    }

    BMPHeader bmpHeader;
    // read BMP header
    if (fread(&bmpHeader, sizeof(BMPHeader), 1, file) != 1) {
        printError("File is not valid or too small.");
        fclose(file);
        return;
    }

    // Check for valid BMP file
    if (bmpHeader.header[0] != 'B' || bmpHeader.header[1] != 'M') {
        printError("The format is not supported.");
        fclose(file);
        return;
    }

    DIBHeader dibHeader;
    // read DIB header
    if (fread(&dibHeader, sizeof(DIBHeader), 1, file) != 1) {
        printError("File is not valid or too small.");
        fclose(file);
        return;
    }

    // check for valid DIB header
    if (dibHeader.size != 40 || dibHeader.bitsPerPixel != 24) {
        printError("The format is not supported.");
        fclose(file);
        return;
    }

    // Print BMP Header
    printf("=== BMP Header ===\n");
    printf("Type: %c%c\n", bmpHeader.header[0], bmpHeader.header[1]);
    printf("Size: %u\n", bmpHeader.size);
    printf("Reserved 1: %u\n", bmpHeader.reserved1);
    printf("Reserved 2: %u\n", bmpHeader.reserved2);
    printf("Image offset: %u\n", bmpHeader.offset);

    // Print DIB Header
    printf("\n=== DIB Header ===\n");
    printf("Size: %u\n", dibHeader.size);
    printf("Width: %u\n", dibHeader.width);
    printf("Height: %u\n", dibHeader.height);
    printf("# color planes: %u\n", dibHeader.planes);
    printf("# bits per pixel: %u\n", dibHeader.bitsPerPixel);
    printf("Compression scheme: %u\n", dibHeader.compression);
    printf("Image size: %u\n", dibHeader.imageSize);
    printf("Horizontal resolution: %u\n", dibHeader.xResolution);
    printf("Vertical resolution: %u\n", dibHeader.yResolution);
    printf("# colors in palette: %u\n", dibHeader.colors);
    printf("# important colors: %u\n", dibHeader.importantColors);

    fclose(file);
}

void revealImage(const char *filename) {
    FILE *file = fopen(filename, "r+b"); //Opens for reading and writing
    if(!file) {
        printError("File not found.");
        return;
    }

    BMPHeader bmpHeader;
    fread(&bmpHeader, sizeof(BMPHeader), 1, file);

    // check BMP format
    if(bmpHeader.header[0] != 'B' || bmpHeader.header[1] != 'M') {
        printError("The format is not supported.");
        fclose(file);
        return;
    }

    DIBHeader dibHeader;
    fread(&dibHeader, sizeof(DIBHeader), 1, file);

    // check DIB format
    if(dibHeader.size != 40 || dibHeader.bitsPerPixel != 24) {
        printError("The format is not supported.");
        fclose(file);
        return;
    }

    fseek(file, bmpHeader.offset, SEEK_SET); // move to start of pixel data

    // calculate padding
    int padding = (4 - (dibHeader.width * 3) % 4) % 4;

    // process each pixel to reveal hidden data
    for(unsigned int row = 0; row < dibHeader.height; row++) {
        for(unsigned int col = 0; col < dibHeader.width; col++) {
            unsigned char pixel[3];
            fread(pixel, sizeof(unsigned char), 3, file);

            // change pixel data to reveal the hidden photo
            for(int i = 0; i < 3; i++) {
                unsigned char lsb = pixel[i] & 0x0F; // extract LSB
                unsigned char msb = pixel[i] & 0xF0; // extract MSB
                pixel[i] = (lsb << 4) | (msb >> 4); // combine them
            }
            

            fseek(file, -3, SEEK_CUR);
            fwrite(pixel, sizeof(unsigned char), 3, file);
        }
        fseek(file, padding, SEEK_CUR); // Skip the padding
    }

    fclose(file);
}

// does not work, would love to discuss 
void hideImage(const char *filename1, const char *filename2) {
    FILE *file1 = fopen(filename1, "r+b");
    if(!file1) {
        printError("File not found.");
        return;
    }

    FILE *file2 = fopen(filename2, "rb");
    if(!file2) {
        printError("Secret file not found.");
        fclose(file1);
        return;
    }

    BMPHeader bmpHeader1, bmpHeader2;
    fread(&bmpHeader1, sizeof(BMPHeader), 1, file1);
    fread(&bmpHeader2, sizeof(BMPHeader), 1, file2);

    if (bmpHeader1.header[0] != 'B' || bmpHeader1.header[1] != 'M' || bmpHeader2.header[0] != 'B' || bmpHeader2.header[1] != 'M') {
        printError("The format is not supported.");
        fclose(file1);
        fclose(file2);
        return;
    }

    DIBHeader dibHeader1, dibHeader2;
    fread(&dibHeader1, sizeof(DIBHeader), 1, file1);
    fread(&dibHeader2, sizeof(DIBHeader), 1, file2);
    if (dibHeader1.size != 40 || dibHeader1.bitsPerPixel != 24 || dibHeader2.size != 40 || dibHeader2.bitsPerPixel != 24) {
        printError("The format is not supported.");
        fclose(file1);
        fclose(file2);
        return;
    }

    if (dibHeader1.width != dibHeader2.width || dibHeader1.height != dibHeader2.height) {
        printError("Images must have the same dimensions.");
        fclose(file1);
        fclose(file2);
        return;
    }

    fseek(file1, bmpHeader1.offset, SEEK_SET); 
    fseek(file2, bmpHeader2.offset, SEEK_SET); 

    int padding1 = (4 - (dibHeader1.width * 3) % 4) % 4; 
  
    // Process each pixel
    for (unsigned int row = 0; row < dibHeader1.height; row++) {
        for (unsigned int col = 0; col < dibHeader1.width; col++) {
            unsigned char pixel1[3], pixel2[3];
            fread(pixel1, sizeof(unsigned char), 3, file1); 
            fread(pixel2, sizeof(unsigned char), 3, file2); 

            // Merge pixels
            for (int i = 0; i < 3; i++) {
                pixel1[i] = (pixel1[i] & 0xF0) | (pixel2[i] & 0x0F); 
            }

            fseek(file1, -3, SEEK_CUR); 
            fwrite(pixel1, sizeof(unsigned char), 3, file1); 
        }

        fseek(file1, padding1, SEEK_CUR); 

        int padding2 = (4 - (dibHeader2.width * 3) % 4) % 4;
        fseek(file2, padding2, SEEK_CUR); 
    }

    fclose(file1);
    fclose(file2);

}

int main(int argc, char *argv[]) {
    // check num of arguments 
    if(argc < 3) {
        printError("Missing arguments.");
        return 1;
    }

    // check what command to run
    if(strcmp(argv[1], "--info") == 0) {
        info(argv[2]);
    } else if(strcmp(argv[1], "--reveal") == 0) {
        revealImage(argv[2]);
    } else if(strcmp(argv[1], "--hide") == 0) {
        if(argc != 4) {
            printError("Invalid arguments for hide command.");
            return 1;
        }
        hideImage(argv[2], argv[3]);
    } else {
        printError("Invalid command.");
    }

    return 0; // end the program
}
