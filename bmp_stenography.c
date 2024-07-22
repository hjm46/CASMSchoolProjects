#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "header.h"

int main (int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("ERROR: NO FILE OR COMMAND GIVEN\n");
        return -1;
    }

    //checks if file exists
    FILE *f = fopen(argv[2], "r+");
    if (f == NULL)
    {
        fprintf(stderr, "ERROR: CANNOT OPEN FILE %s \n", argv[2]);
        return -1;
    }

    //allocates memory for image metadata
    BMP* bmp = (BMP*)malloc(sizeof(BMP));
    DIB* dib = (DIB*)malloc(sizeof(DIB));
    if(bmp == NULL || dib == NULL)
    {
        printf("ERROR: NOT ENOUGH MEMORY\n");
        return -1;
    }

    //reads in BMP metadata
    fread(&bmp->format_type, 2, 1, f);
    if(strcmp(bmp->format_type, "BM") != 0)
    {
        printf("The format is not supported.\n");
        return -1;
    }

    fread(&bmp->file_size, 4, 1, f);
    fread(&bmp->res1, 2, 1, f);
    fread(&bmp->res2, 2, 1, f);
    fread(&bmp->offset, 4, 1,f);

    fread(&dib->header_size, 4, 1, f);

    //checks if the file type is supported
    if(dib->header_size != 40)
    {
        printf("The format is not supported.\n");
        return -1;
    }

    //reads in DIB metadata
    fread(&dib->width, 4, 1, f);
    fread(&dib->height, 4, 1, f);
    fread(&dib->num_color_planes, 2, 1, f);
    fread(&dib->bits_per_pixel, 2, 1, f);
    fread(&dib->compression, 4, 1, f);
    fread(&dib->image_size, 4, 1, f);
    fread(&dib->h_res, 4, 1, f);
    fread(&dib->v_res, 4, 1, f);
    fread(&dib->num_colors, 4, 1, f);
    fread(&dib->important_colors, 4, 1, f);

    if (strcmp(argv[1], "--info") == 0)
    {
        printf("=== BMP HEADER ===\nType: %.2s\nSize: %d\nReserved 1: %hd\n", bmp->format_type, bmp->file_size, bmp->res1);
        printf("Reserved 2: %hd\nImage offset: %d\n\n", bmp->res2, bmp->offset);
        printf("=== DIB HEADER ===\nSize: %d\nWidth: %d\nHeight: %d\n", dib->header_size, dib->width, dib->height);
        printf("# color planes: %hd\n# bits per pixel: %hd\nCompression scheme: %d\n", dib->num_color_planes, dib->bits_per_pixel, dib->compression);
        printf("Image size: %d\nHorizontal resolution: %d\nVertical resolution: %d\n", dib->image_size, dib->h_res, dib->v_res);
        printf("# colors in palette: %d\n# important colors: %d\n\n", dib->num_colors, dib->important_colors);
    }

    //start of image processing
    fseek(f, bmp->offset, SEEK_SET);

    Pixels* pixels = (Pixels*)malloc(sizeof(Pixels));
    if(pixels == NULL)
    {
        printf("ERROR: NOT ENOUGH MEMORY\n");
        return -1;
    }
    
    //reveal message
    if (strcmp(argv[1], "--reveal") == 0)
    {
        char mask = 0x0F;
        char letter;
        for(int row = 1; row<=dib->height; row++)
        {            
            for(int col = 1; col<=dib->width; col++)
            {
                fread(&pixels->blue, 1, 1, f);
                fread(&pixels->green, 1, 1, f);
                char lsb_green = pixels->green & mask;
                char lsb_blue = pixels->blue & mask;
                char msb = lsb_green^lsb_blue;
                fread(&pixels->red, 1, 1, f);
                char lsb_red = pixels->red & mask;
                char lsb = lsb_red^lsb_blue;
                letter = (msb<<4) | lsb;
                if(letter == '\0')
                    break;
                printf("%c", letter);
            }
            if(letter == '\0')
                break;
            if ((dib->width)%4 != 0)
                fseek(f, (dib->width)%4, SEEK_CUR);
        }
        printf("\n");
    }

    //hide message
    if (strcmp(argv[1], "--hide") == 0)
    {
        if (argc < 4)
        {
            printf("ERROR: NO SECOND FILE\n");
            return -1;
        }

        FILE *f2 = fopen(argv[3], "r");
        if (f2 == NULL)
        {
            fprintf(stderr, "ERROR: CANNOT OPEN FILE %s \n", argv[3]);
            return -1;
        }

        char mask = 0x0F;
        char msb_mask = 0xF0;
        int row = 1;
        int col = 1;
        while(!feof(f2))
        {
            if(row==dib->height)
            {
                printf("ERROR: Text too large");
                return -1;
            }

            if((col==dib->width) && ((dib->width)%4 != 0))
            {
                fseek(f, (dib->width*3)%4, SEEK_CUR);
                row++;
            }
                char letter;
                fread(&letter, 1, 1, f2);
                char msb = letter >> 4;
                char lsb = letter & mask;

                fread(&pixels->blue, 1, 1, f);
                fread(&pixels->green, 1, 1, f);
                char lsb_blue = pixels->blue & mask;
                char new_green = msb ^ lsb_blue;
                pixels->green = pixels->green & msb_mask;
                pixels->green = pixels->green | new_green;
                fseek(f, -1, SEEK_CUR);
                fprintf(f, "%c", pixels->green);

                fread(&pixels->red, 1, 1, f);
                char new_red = lsb ^ lsb_blue;
                pixels->red = pixels->red & msb_mask;
                pixels->red = pixels->red | new_red;
                fseek(f, -1, SEEK_CUR);
                fprintf(f, "%c", pixels->red);
                
                col++;
        }
        if(fread(&pixels->blue, 1, 3, f) != '\0')
            {
                fseek(f, -3, SEEK_CUR);
                fread(&pixels->blue, 1, 1, f);
                fread(&pixels->green, 1, 1, f);
                char lsb_blue = pixels->blue & mask;
                char new_green = 0x00 ^ lsb_blue;
                pixels->green = pixels->green & msb_mask;
                pixels->green = pixels->green | new_green;
                fseek(f, -1, SEEK_CUR);
                fprintf(f, "%c", pixels->green);

                fread(&pixels->red, 1, 1, f);
                char new_red = 0x00 ^ lsb_blue;
                pixels->red = pixels->red & msb_mask;
                pixels->red = pixels->red | new_red;
                fseek(f, -1, SEEK_CUR);
                fprintf(f, "%c", pixels->red);
            }
        fclose(f2);
    }
    
    free(pixels);
    free(dib);
    free(bmp);
    fclose(f);
    return 0;
}