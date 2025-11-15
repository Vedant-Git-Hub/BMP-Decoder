#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <windows.h>
#include <io.h>


#define IMAGE_PATH_REL          "../monochrome.bmp"


typedef struct{
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


        return 1;
    }

    return 0;

}

int main()
{
    if( getImageArray(IMAGE_PATH_REL) )
    {
        printf("\nGot Image array");
    }

    closeMap();

    return 0;
}
