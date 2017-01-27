/**
 * Copies a BMP piece by piece, just because.
 */
       
#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./resize n infile outfile\n");
        return 1;
    }

    // remember filenames
    char *infile = argv[2];
    char *outfile = argv[3];
    
    int multiplier = atoi(argv[1]);
    printf("multiplier: %d\n", multiplier);
    
    if (multiplier < 1 || multiplier > 100) {
        printf("Invailed Zoom Factor. \"%i\" Zoom Factor MUST be 1-100\n", multiplier);
        return 1;
    }

    // open input file 
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf, bfOut;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);
    bfOut = bf;
    
    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi, biOut;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);
    biOut = bi;

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 || 
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }
    
    // multiply image size
    biOut.biWidth = biOut.biWidth * multiplier;
    biOut.biHeight = abs(biOut.biHeight * multiplier);
    printf("biWidth:  %d\n", biOut.biWidth);
    printf("biHeight: %d\n", biOut.biHeight);

    // determine padding for scanlines
    int paddingIn = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    int paddingOut = (4 - (biOut.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    printf("paddingIn:  %d | width: %d\n", paddingIn, bi.biWidth);
    printf("paddingOut: %d\n", paddingOut);
    
    // determine file size
    biOut.biSizeImage = ((biOut.biWidth * sizeof(RGBTRIPLE)) + paddingOut) * biOut.biHeight;
    bfOut.bfSize = bf.bfSize - bi.biSizeImage + biOut.biSizeImage;
	
    
    // write outfile's BITMAPFILEHEADER
    fwrite(&bfOut, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&biOut, sizeof(BITMAPINFOHEADER), 1, outptr);

    // itterate over height of file
    for (int i = 0; i < bi.biHeight; i++)
    {
        // iterate over infile's scanlines
        for (int scanLine = 0; scanLine < multiplier; scanLine++) 
        {
            // iterate over pixels in scanline
            for (int j = 0; j < bi.biWidth; j++)
            {
                // temporary storage
                RGBTRIPLE triple;
    
                // read RGB triple from infile
                fread(&triple, sizeof(RGBTRIPLE), 1, inptr);
                
                // write RGB triple to outfile
                for (int w = 0; w < multiplier; w++) {
                    fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
                }
            }
        }
        
        // Skip over original file padding, if any
        fseek(inptr, paddingIn, SEEK_CUR);
        
        // then add it back (to demonstrate how)
        for (int k = 0; k < paddingOut; k++)
        {
            fputc(0x00, outptr);
        }
        
        if (i % multiplier != 0) fseek(inptr, (-bi.biWidth * sizeof(RGBTRIPLE)) - paddingIn, SEEK_CUR);
    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}
