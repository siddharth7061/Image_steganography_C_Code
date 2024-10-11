#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

Status open_files_for_decoding(DecodeInfo *deinfo)
{
    // open stego image file
    deinfo->fptr_stego_img = fopen(deinfo->file_name_stego, "r");

    // Do error handling
    if (deinfo->fptr_stego_img == NULL)
    {
        perror("fopen\n");
        fprintf(stderr, "Unable to open file %s\n", deinfo->file_name_stego);
        return d_failure;
    }

    return d_success;
}

Status read_validate_decode_args(char *argv[], DecodeInfo *deInfo)
{
    // get .bmp file
    // if strcmp(".bmp", ".bmp")
    /* check if user has provided the stego image file name or not */
    if (argv[2] != NULL && strcmp((strstr(argv[2], ".")), ".bmp") == 0)
    {
        /*  Storing the image file name in the deInfo */
        deInfo->file_name_stego = argv[2];
    }
    else
    {
        return d_failure;
    }

    /* check if user has given the output image file */
    if (argv[3] != NULL && strcmp((strstr(argv[3], ".")), ".txt") == 0)
    {
        deInfo->file_name_decode = argv[3];
    }
    else
    {
        // if not then create new one with stego.bmp
        deInfo->file_name_decode = "decode.txt";
    }

    return d_success;
}

Status decode_lsb(DecodeInfo *deInfo)
{
    char mask = 1;
    deInfo->ms_char = 0; // char where each byte info will be store, so clearing it
    for (int i = 0; i < 8; i++)
    {
        // bitwise operation to decode lsb and store in char variable
        deInfo->ms_char = deInfo->ms_char | ((deInfo->decode_image_data[i] & mask) << (7-i));
    }
    return d_success;
}

Status decode_data_to_text_file(DecodeInfo *deInfo, int size, const char *image_data, FILE *fptr_stego_img)
{
    fseek(deInfo->fptr_stego_img, 54, SEEK_SET);
    //printf("Position of file pointer before reading: %ld\n", ftell(deInfo->fptr_stego_img));
    size_t f_size;
    for (int i = 0; i < size; i++)
    {
        // read 8 bytes of data from stego image
        f_size = fread(deInfo->decode_image_data, sizeof(char), 8, fptr_stego_img);
        if(f_size != 8)
        {
            printf("Failed to read 8 bytes from stego image: %ld byte read at %ld position\nData: %s\n", f_size, ftell(deInfo->fptr_stego_img), deInfo->decode_image_data);
            return d_failure;
        }

        // decode each byte to char variable
        decode_lsb(deInfo);

        // write decoded magic string to array
        deInfo->decode_magic_string[i] = deInfo->ms_char;
    }
    
    
    // comparing magic string
    if (strcmp(deInfo->decode_magic_string, "#*") == 0)
    {
        // Magic string is matched
        printf("Magic String matched\n");
        return d_success;
    }
    else
    {
        // Magic string not matched
        printf("FAILED: Magic String does not match\n");
        return d_failure;
    }

}

Status decode_magic_string(DecodeInfo *deInfo, const char *image_data)
{
    // calling function to decode data to text file
    if (decode_data_to_text_file(deInfo, 2, image_data, deInfo->fptr_stego_img) == d_success)
    {
        return d_success;
    }
    else
    {
        return d_failure;
    }
}

Status decode_secret_file_extension_size(DecodeInfo *deInfo)
{
    // creating array to retain 4x8 bytes of data from the stego image files
    char str[32], mask = 0x01;
    deInfo->secret_file_extension_size = 0;
    size_t f_size;
    // read 32 bytes of data from the stego image file
    f_size = fread(str, sizeof(char), 32, deInfo->fptr_stego_img);
    //error handling
    if (f_size != 32)
    {
        printf("Failed to read 32 bytes of extension size data\n");
        return d_failure;
    }
    
    // LSB decoding the secret file extension size
    for (int i = 0; i < 32; i++)
    {
        deInfo->secret_file_extension_size = deInfo->secret_file_extension_size | ((str[i] & mask) << (31 - i));
    }
    printf("Extensionn size: %d\n", deInfo->secret_file_extension_size);
    return d_success;
}


Status decode_secret_file_extension(DecodeInfo *deInfo)
{
    size_t f_size;
    char *ptr = deInfo->secret_file_extension;
    for (int i = 0; i < 4; i++)
    {
        // read 8 bytes from stego image
        f_size = fread(deInfo->decode_image_data, sizeof(char), 8, deInfo->fptr_stego_img);
        if (f_size != 8)
        {
            printf("Unable to read 8 bytes from stego image\n");
            return d_failure;
        }
        
        // decode each byte to char variable
        decode_lsb(deInfo);

        // write decoded file extension name to array
        deInfo->secret_file_extension[i] = deInfo->ms_char;
    }
    
    
    return d_success;
}

Status decode_secret_file_size(DecodeInfo *deInfo)
{
    char str[32], mask = 0x01;
    deInfo->secret_file_size = 0;
    size_t f_size;
    // reading 32 bytes of data from stego image
    f_size = fread(str, sizeof(char), 32, deInfo->fptr_stego_img);
    // error handling
    if (f_size != 32)
    {
        printf("Unable to read 8 bytes from stego file\n");
        return d_failure;
    }

    //LSB encoding secret file size
    for (int i = 0; i < 32; i++)
    {
        deInfo->secret_file_size = deInfo->secret_file_size | ((str[i] & mask) << (31 -i));
    }
    //printf("Secret file size: %d\n", deInfo->secret_file_size);
    return d_success;
}

Status open_decode_text_file(DecodeInfo *deInfo)
{
    /* open output decode file */
    deInfo->fptr_decode_file = fopen(deInfo->file_name_decode, "w");

    // Do error handling
    if (deInfo->fptr_decode_file == NULL)
    {
        perror("fopen\n");
        fprintf(stderr, "Unable to open file %s\n", deInfo->file_name_decode);
        return d_failure;
    }
    return d_success;
}

Status decode_secret_file_data(DecodeInfo *deInfo)
{
    size_t f_size;
    fseek(deInfo->fptr_decode_file, 0, SEEK_SET);
    for (int i = 0; i < deInfo->secret_file_size; i++)
    {
        // read 8 bytes of data from the stego image
        f_size = fread(deInfo->decode_image_data, sizeof(char), 8, deInfo->fptr_stego_img);
        // error handling
        if (f_size != 8)
        {
            printf("Unable to read 8 bytes of data from stego image\n");
            return d_failure;
        }
        
        // decode LSB
        decode_lsb(deInfo);
        // write data to decode text file
        f_size = fwrite(&(deInfo->ms_char), sizeof(char), 1, deInfo->fptr_decode_file);
        // error handling
        if (f_size != 1)
        {
            printf("Unable to write 1 byte of data to decode text file\n");
            return d_failure;
        }
    }
    return d_success;
}

Status do_decoding(DecodeInfo *deInfo)
{
    /* DECODING */

    // open stego image file
    if (open_files_for_decoding(deInfo) == d_success)
    {
        printf("Files opened successfully\n");
        printf("Started Decoding\n");
        if (decode_magic_string(deInfo, deInfo->decode_image_data) == d_success)
        {
            printf("Magic String decoded successful\n");
            if (decode_secret_file_extension_size(deInfo) == d_success)
            {
                printf("Decoded secret file extension size successfully\n");
                if (decode_secret_file_extension(deInfo) == d_success)
                {
                    printf("Decoded secret file extension successfully\n");
                    if (decode_secret_file_size(deInfo) == d_success)
                    {
                        printf("Decoded secret file size successfully\n");
                        if (open_decode_text_file(deInfo) == d_success)
                        {
                            printf("Opened decode text file successfully\n");
                            if (decode_secret_file_data(deInfo) == d_success)
                            {
                                printf("Decode secret file data successfully\n");
                                return d_success;
                            }
                            else
                            {
                                printf("Failed to decode secret file data\n");
                                return d_failure;
                            }
                        }
                        else
                        {
                            printf("Failed to open decoded text file\n");
                            return d_failure;
                        }
                        
                    }
                    else
                    {
                        printf("Failed to decode secret file size\n");
                        return d_failure;
                    }
                    
                }
                else
                {
                    printf("Failed to decode secret file extension\n");
                    return d_failure;
                }
                
            }
            else
            {
                printf("Unable to decode extension size\n");
                return d_failure;
            }
        }
        else
        {
            printf("Failed to decode magic String\n");
            return d_failure;
        }
        
    }
    else
    {
        printf("FAILURE: Files could not be opened\n");
        return d_failure;
    }
    
    
}