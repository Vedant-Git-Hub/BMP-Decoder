#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <windows.h>
#include <io.h>

/*Absolute path to the image*/
#define IMAGE_PATH            "D:/C code/BMP-Decoder/BMPDecoder/Images/monochrome.bmp"
#define DISP_BUFF_SIZE        8192



/*Enum for color in which the text should be printed*/
typedef enum
{
    NO_COLOR = 0,
    /*Normal color*/
    BLACK = 30,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYIAN,
    WHITE,
    /*Bold colors*/
    B_BLACK = 90,
    B_RED,
    B_GREEN,
    B_YELLOW,
    B_BLUE,
    B_MAGENTA,
    B_CYIAN,
    B_WHITE,
} E_COLOR;


/*BMP header structure format*/
#pragma pack(1)
typedef struct
{
    char magic_bytes[2];        /*2 bytes of magic bytes "BM"*/
    uint32_t file_size;         /*File size in bytes*/
    uint32_t reserved;          /*4 bytes reserved*/
    uint32_t img_arr_offset;    /*Offset to image array*/
} T_BMP_HEADER;

/*DIB header structure format*/
#pragma pack(1)
typedef struct
{
    uint32_t dib_header_size;   /*DIB header size*/
    uint32_t pixel_x;           /*Number of pixels in X direction*/
    uint32_t pixel_y;           /*Number of pixels in Y direction*/
    uint16_t color_plane_num;   /*Number of color planes*/
    uint16_t bits_per_pixel;    /*Bits used per pixel(always 1 for monochrome bmp images)*/
    uint32_t compression;       /*Compression used*/
    uint32_t raw_bitmap_size;   /*Image array size in bytes (includes padded bytes as well)*/
    uint32_t img_res_x;         /*Resolution in X*/
    uint32_t img_res_y;         /*Resolution in Y*/
    uint32_t color_pal_num;     /*Color pallete number*/
    uint32_t imp_colors;        /*Important colors*/
} T_DIB_HEADER;

/*BMP file format structure*/
#pragma pack(1)
typedef struct
{
    T_BMP_HEADER bmp_header;    /*BMP header*/
    T_DIB_HEADER dib_header;    /*DIB header*/
    uint8_t img_array[];        /*Image array*/
} T_BMP;


/*Holds address for file pointer*/
static FILE *file_ptr = NULL;
/*Handle for mapping the file into memory*/
static HANDLE handle_map;
/*Points to start of the BMP file*/
static T_BMP *image = NULL;



/*Gets the pointer to the BMP file in memory*/
bool getFilePointer(char *bmp_img)
{
    /*Opens the BMP image file in read only and binary mode*/
    file_ptr = fopen(bmp_img, "r+b");
    /*Checks if the file was opened successfully*/
    if(file_ptr == NULL)
    {
        printf("\nError: Unable to open file!");
        goto Error;
    }

    /*Gets the file descriptor out of file pointer*/
    int file_des = _fileno(file_ptr);

    /*Gets the file handle out of file descripter for windows*/
    HANDLE handle_File = (HANDLE)_get_osfhandle(file_des);
    /*Handles the invalid file handle value case*/
    if(handle_File == INVALID_HANDLE_VALUE)
    {
        printf("\nError: Invalid file handle");
        goto Error;
    }

    /*Creates a map of file in memory*/
    handle_map = CreateFileMapping(handle_File, NULL, PAGE_READWRITE, 0, 0, NULL);
    /*Checks if the map handle was created successfully or not*/
    if( !handle_map )
    {
        printf("Error: File map creation failed");
        goto Error;
    }

    /*Gets the pointer to the map of file created in memory into T_BMP structure pointer variable*/
    image = (T_BMP *)MapViewOfFile(handle_map, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    /*Checks if the file is not pointed to a NULL*/
    if(image == NULL)
    {
        printf("Error: Map view of file failed");
        goto Error;
    }

    return 1;

    /*Lable to handle errors for all ghe above cases*/
Error:
    return 0;

}

/*Housekeeping function to close opened files, maps and handles*/
void closeMap()
{
    /*Unmaps the file from memory*/
    UnmapViewOfFile((uint8_t *)image);
    /*Closes the map handle*/
    CloseHandle(handle_map);
    /*Closes the opened BMP image file*/
    fclose(file_ptr);
}

/*Validates the opened file in memory view*/
bool validateFile(char *bmp_img)
{
    /*Checks if the map view of file was created successfully*/
    if( getFilePointer(bmp_img) )
    {
        /*Checks if the magic bytes are present in the file as expected*/
        if(image->bmp_header.magic_bytes[0] != 'B' || image->bmp_header.magic_bytes[1] != 'M' )
        {
            printf("\nError: Magic bytes not matched!");
            goto Error;
        }

        /*Checks if file size is not zero*/
        if(image->bmp_header.file_size <= 0)
        {
            printf("\nError: File size is zero!");
            goto Error;
        }

        /*checks if image array size is not zero*/
        if(image->dib_header.raw_bitmap_size == 0)
        {
            printf("\nError: Raw bitmap size is zero!");
            goto Error;
        }

        return 1;
    }

    /*Label to handle errors for above cases*/
Error:
    return 0;

}

bool printDisplayBuffer(uint8_t color)
{
    /*Variable to hold pixels in x direction*/
    uint32_t hor = image->dib_header.pixel_x;
    /*Variable to hold pixels in Y direction*/
    uint32_t ver = image->dib_header.pixel_y;
    /*Gets the total bytes required to represent pixels in x direction or total bytes in a row*/
    uint32_t row_bytes = image->dib_header.raw_bitmap_size / image->dib_header.pixel_y;
    char *display_buffer = NULL;

    display_buffer = (char *)malloc(DISP_BUFF_SIZE);
    uint32_t disp_idx = 0;

    if(!display_buffer)
    {
        printf("\nError: Display buffer null pointer!");
        return 0;
    }

    /*Loop to traverse through the array rows, in reserve as the image array in inverted inside a BMP file*/
    for(int32_t row = ver; row > 0; row--)
    {
        /*Loop to traverse through the columns*/
        for(uint32_t col = 0; col < hor; col++)
        {
            /*Gets the byte from the image array which represents group of 8 pixels*/
            uint8_t temp = image->img_array[((row - 1) * row_bytes) + (col / 8)];

            if(disp_idx >= (DISP_BUFF_SIZE - 2))
            {
                display_buffer[disp_idx++] = '\0';
                printf("\033[%dm%s\033[0m", color, display_buffer);
                disp_idx = 0;
                memset(display_buffer, 0, DISP_BUFF_SIZE);
            }

            /*Checks if the required pixel is ON by locating the bit representing the pixel*/
            if(((temp >> (7 - (col & 0x00000007))) & 0x01) == 1)
            {
                display_buffer[disp_idx++] = '#';
            }
            else
            {
                display_buffer[disp_idx++] = ' ';
            }

        }

        display_buffer[disp_idx++] = '\n';
    }

    if (disp_idx > 0)
    {
        display_buffer[disp_idx] = '\0';
        printf("\033[%dm%s\033[0m", color, display_buffer);
    }

    free(display_buffer);

    return 1;
}

/*Function to print the image array on console in user selectable colors*/
void printImageScrn(char *img_path, uint8_t color)
{
    /*Validates the BMP image file*/
    if( validateFile(img_path) )
    {
        /*Uncomment this block to view the headers in the file*/
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

        /*Handles the buffering logic and prints image on screen*/
        printDisplayBuffer(color);

    }

    /*Close the opened map and file*/
    closeMap();

}


int main()
{
    /*Imp call to set the encoding to UTF-8 inorder to use block character '█'*/
    SetConsoleOutputCP(CP_UTF8);
    /*Prints the image on screen with desired color, if all the checks are passed*/
    printImageScrn(IMAGE_PATH, RED);
//    printImageScrn(IMAGE_PATH, GREEN);
//    printImageScrn(IMAGE_PATH, BLUE);


    while(1);

    return 0;
}
