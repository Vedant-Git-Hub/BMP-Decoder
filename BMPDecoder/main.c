#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <windows.h>
#include <io.h>


#define IMAGE_PATH_REL          "../monochrome.bmp"

#pragma pack(1)
typedef struct{
    char magic_bytes[2];
    uint32_t file_size;
    uint32_t reserved;
    uint32_t img_arr_offset;
}T_BMP_HEADER;

#pragma pack(1)
typedef struct{
    uint32_t dib_header_size;
    uint32_t pixel_x;
    uint32_t pixel_y;
    uint16_t color_plane_num;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t raw_bitmap_size;
    uint32_t img_res_x;
    uint32_t img_res_y;
    uint32_t color_pal_num;
    uint32_t imp_colors;
}T_DIB_HEADER;

#pragma pack(1)
typedef struct{
    T_BMP_HEADER bmp_header;
    T_DIB_HEADER dib_header;
    uint8_t img_array[];
}T_BMP;


static FILE *file_ptr;
static HANDLE handle_map;
static uint8_t *file_mem_ptr = NULL;
static uint8_t *image_array = NULL;
static T_BMP *image;




bool getFilePointer(char *bmp_img)
{
    file_ptr = fopen(bmp_img, "r+b");
    if(file_ptr == NULL)
    {
        printf("\nError: File was not openned successfully...");
        goto Error;
    }

    int file_des = _fileno(file_ptr);

    HANDLE handle_File = (HANDLE)_get_osfhandle(file_des);
    if(handle_File == INVALID_HANDLE_VALUE)
    {
        printf("\nError: Invalid file handle");
        goto Error;
    }

    handle_map = CreateFileMapping(handle_File, NULL, PAGE_READWRITE, 0, 0, NULL);
    if( !handle_map )
    {
        printf("Error: File map creation failed");
        goto Error;
    }

    file_mem_ptr = MapViewOfFile(handle_map, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if(file_mem_ptr == NULL)
    {
        printf("Error: Map view of file failed");
        goto Error;
    }

    return 1;

Error:
    return 0;

}

void closeMap()
{
    UnmapViewOfFile(file_mem_ptr);
    CloseHandle(handle_map);
    fclose(file_ptr);
}

bool getImageArray(char *bmp_img)
{
    if( getFilePointer(bmp_img) )
    {
        image = (T_BMP *) file_mem_ptr;

        if(image->bmp_header.magic_bytes[0] != 'B' || image->bmp_header.magic_bytes[1] != 'M' )
        {
            goto Error;
        }

        if(image->bmp_header.file_size == 0)
        {
            goto Error;
        }

        if(image->dib_header.raw_bitmap_size == 0)
        {
            goto Error;
        }

        return 1;
    }

    Error:
    return 0;

}

void printImageScrn()
{
    uint32_t hor = image->dib_header.pixel_x;
    uint32_t ver = image->dib_header.pixel_y;
    uint32_t row_bytes = image->dib_header.raw_bitmap_size / image->dib_header.pixel_y;


    for(int32_t row = ver; row > 0; row--)
    {
        for(uint32_t col = 0; col < row_bytes; col++)
        {
            uint8_t temp = image->img_array[(row * row_bytes) + col];

            for(int8_t shift = 7; shift >= 0; shift--)
            {
                if(((temp >> (shift)) & 0x01) == 1)
                {
                    printf("█");
                }
                else
                {
                    printf(" ");
                }
            }
        }

        printf("\n");
    }
}


int main()
{
    SetConsoleOutputCP(CP_UTF8);

    if( getImageArray(IMAGE_PATH_REL) )
    {
//        printf("\n Sizeof header = %d, size of dib header = %d", sizeof(T_BMP_HEADER), sizeof(T_DIB_HEADER));
//        printf("\n Magic bytes = %c%c", image->bmp_header.magic_bytes[0], image->bmp_header.magic_bytes[1]);
//        printf("\n File size = %d", image->bmp_header.file_size);
//        printf("\n Image array offset = %d", image->bmp_header.img_arr_offset);
//        printf("\n");
//        printf("\n DIB header size = %d", image->dib_header.dib_header_size);
//        printf("\n Pixel X = %d", image->dib_header.pixel_x);
//        printf("\n Pixel Y = %d", image->dib_header.pixel_y);
//        printf("\n Color plane number = %d", image->dib_header.color_plane_num);
//        printf("\n Bits per pixel = %d", image->dib_header.bits_per_pixel);
//        printf("\n Compression = %d", image->dib_header.compression);
//        printf("\n Raw bitmap size = %d", image->dib_header.raw_bitmap_size);
//        printf("\n Res X = %d", image->dib_header.img_res_x);
//        printf("\n Res Y = %d", image->dib_header.img_res_y);
//        printf("\n Color pallate number = %d", image->dib_header.color_pal_num);
//        printf("\n Important colors = %d", image->dib_header.imp_colors);
//        printf("\n\n");

        printImageScrn();

    }

    closeMap();

    while(1);

    return 0;
}
