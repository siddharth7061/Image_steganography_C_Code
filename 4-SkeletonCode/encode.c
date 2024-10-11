#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // get .bmp file
    // if strcmp(".bmp", ".bmp")
    /* check if user has provided the original image file name or not */
    if (argv[2] != NULL && strcmp((strstr(argv[2], ".")), ".bmp") == 0)
    {
        /*  Storing the image file name in the encInfo */
        encInfo->src_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }

    // if strcmp(".txt", ".txt")
    /* check if user has provided the secret file */
    if (argv[3] != NULL && strcmp((strstr(argv[3], ".")), ".txt") == 0)
    {
        /*  Storing the secret file name in the encInfo */
        encInfo->secret_fname = argv[3];
    }
    else
    {
        return e_failure;
    }

    /* check if user has given the output image file */
    if (argv[4] != NULL)
    {
        encInfo->stego_image_fname = argv[4];
    }
    else
    {
        // if not then create new one with stego.bmp
        encInfo->stego_image_fname = "stego.bmp";
    }
    return e_success;
}

uint get_file_size(FILE *fptr_secret)
{
    fseek(fptr_secret, 0, SEEK_END);
    return ftell(fptr_secret);
}

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    // comparing size
    if (encInfo->image_capacity > (54 + ((2 + 4 + 4 + 4 + encInfo->size_secret_file) * 8)))
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char header[54];
    // reset pointer to the beginning of the file
    fseek(fptr_src_image, 0, SEEK_SET);

    // copying 54 bytes of data to the string
    fread(header, sizeof(char), 54, fptr_src_image);

    // writing data to the output image file
    fwrite(header, sizeof(char), 54, fptr_dest_image);
    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_data)
{
    unsigned int mask = 0;
    mask = 1 << 7;
    for (int i = 0; i < 8; i++)
    {
        image_data[i] = (image_data[i] & 0xFE) | ((data & mask) >> (7-i));
        mask = mask >> 1;
    }
    return e_success;
}

Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image, EncodeInfo *encInfo)
{
    for (int i = 0; i < size; i++)
    {   
        // read 8bytes of RGB data from source image
        fread(encInfo->image_data, sizeof(char), 8, fptr_src_image);
        // encoding 1 byte of data in 8 bytes of info
        encode_byte_to_lsb(data[i], encInfo->image_data);
        // writing the final encoded data to the output image
        fwrite(encInfo->image_data, sizeof(char), 8, fptr_stego_image);
    }
    return e_success;
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    // every encoding needs to call encode_data_to_image
    encode_data_to_image(magic_string, strlen(magic_string), encInfo->fptr_src_image, encInfo->fptr_stego_image, encInfo);
    return e_success;
}

Status encode_size_to_lsb(char *image_buffer, int size)
{
    unsigned int mask = 0;
    mask = 1 << 31; // 31 because size is int variable 4 x 8
    for (int i = 0; i < 32; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((size & mask) >> (31-i));
        mask = mask >> 1;
    }
    return e_success;
}

Status encode_secret_file_extn_size(int size, FILE *fptr_src, FILE *fptr_stego)
{
    // creating string for 32 bytes since .txt is of 4 characters
    char str[32];
    fread(str, 32, sizeof(char), fptr_src);
    // encode size to lsb of the acquired data from the source image
    encode_size_to_lsb(str, size);
    // write the encoded data to the stego image
    fwrite(str, 32, sizeof(char), fptr_stego);
    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    file_extn = ".txt";
    encode_data_to_image(file_extn, strlen(file_extn), encInfo->fptr_src_image, encInfo->fptr_stego_image, encInfo);
    return e_success;
}

Status encode_secret_file_size(long int file_size, EncodeInfo *encInfo)
{
    // creating string for 32 bytes since secret file size is int 
    char str[32];
    fread(str, 32, sizeof(char), encInfo->fptr_src_image);
    // encode size to lsb of the acquired data from the source image
    encode_size_to_lsb(str, file_size);
    // write the encoded data to the stego image
    fwrite(str, 32, sizeof(char), encInfo->fptr_stego_image);
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;
    
    // bring secret file pointer to the begining size of the file
    fseek(encInfo->fptr_secret, 0, SEEK_SET);
    for (int i = 0; i < encInfo->size_secret_file; i++)
    {
        fread(encInfo->image_data, 8, sizeof(char), encInfo->fptr_src_image);
        fread(&ch, 1, sizeof(char), encInfo->fptr_secret);
        encode_byte_to_lsb(ch, encInfo->image_data);
        fwrite(encInfo->image_data, 8, sizeof(char), encInfo->fptr_stego_image);
    }
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    // fread will return 0 when the pointer reaches the end of file
    while (fread(&ch, 1, 1, fptr_src) > 0) // read until fread returns 0
    {
        fwrite(&ch, 1,1, fptr_dest);
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    /* ENCODING */

    // call rest of the functions
    if (open_files(encInfo) == e_success)
    {
        printf("Opened all the files\n");
        printf("Started Encoding\n");

        // check the capacity
        if (check_capacity(encInfo) == e_success)
        {
            printf("Encoding is possible, please continue\n");

            // copy bmp header as it is
            if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
            {
                printf("Copied header succesfully\n");

                // Encode Magic String
                if (encode_magic_string(MAGIC_STRING, encInfo) == e_success)
                {
                    printf("Magic String encoded successfully\n");

                    // Encode file extension size
                    if (encode_secret_file_extn_size(strlen(".txt"), encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
                    {
                        printf("Successfully encoded secret file extension size\n");

                        // Encode secret file extension
                        if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
                        {
                            // encode .txt
                            printf("Successfully encoded secret file extension\n");

                            // Encode secret file size
                            if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
                            {
                                printf("Successfully encoded secret file size\n");

                                // Encode secret file data
                                if (encode_secret_file_data(encInfo) == e_success)
                                {
                                    printf("Successfully encoded secret file data\n");

                                    // Copy remaining image data to output image file
                                    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
                                    {
                                        printf("Copied remaining bytes successfully\n");
                                        return e_success;
                                    }
                                    else
                                    {
                                        printf("Failed copying remaining bytes\n");
                                        return e_failure;
                                    }
                                    
                                    
                                }
                                else
                                {
                                    printf("Failed encoding secret file data\n");
                                    return e_failure;
                                }
                            }
                            else
                            {
                                printf("Failed encoding secret file size\n");
                                return e_failure;
                            }
                            
                            
                        }
                        else
                        {
                            printf("Failed encoding secret file extension\n");
                            return e_failure;
                        }
                        
                        
                    }
                    else
                    {
                        printf("Failed encoding extension size\n");
                        return e_failure;
                    }
                    
                    
                }
                else
                {
                    printf("Failed encoding magic string\n");
                    return e_failure;
                }
            }
            else
            {
                printf("Failed copying header\n");
                return e_failure;
            }
        }
        else
        {
            printf("Encoding is not possible as source image size is not possible\n");
            return e_failure;
        }
    }
    else
    {
        printf("Failed opening files\n");
        return e_failure;
    }
    return e_success;
}
