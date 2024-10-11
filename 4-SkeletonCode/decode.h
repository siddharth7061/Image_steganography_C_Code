#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types

typedef struct _DecodeInfo
{
    /* Stego File Data */
    char *file_name_stego;
    FILE *fptr_stego_img;

    /* Output File .txt */
    char *file_name_decode;
    FILE *fptr_decode_file;
    int secret_file_extension_size;
    char secret_file_extension[4];
    int secret_file_size;

    /* MAGIC STRING ARRAY */
    char ms_char;
    char decode_image_data[8];
    char decode_magic_string[2];
} DecodeInfo;

/* Read and validate arguments */
Status read_validate_decode_args(char *argv[], DecodeInfo *deInfo);

/* Do encoding */
Status do_decoding(DecodeInfo *deInfo);

/* Open Files for decoding */
Status open_files_for_decoding(DecodeInfo *deinfo);

/* Decode magic string */
Status decode_magic_string(DecodeInfo *deInfo, const char *image_data);

/* Decode data to text file */
Status decode_data_to_text_file(DecodeInfo *deInfo, int size, const char *image_data, FILE *fptr_stego_imgs);

/* Decode lsb to text file */
Status decode_lsb(DecodeInfo *deInfo);

/* Decode secret file extension size */
Status decode_secret_file_extension_size(DecodeInfo *deInfo);

/* Decode secret file extension */
Status decode_secret_file_extension(DecodeInfo *deInfo);

/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *deInfo);

/* Decode secret file data */
Status decode_secret_file_data(DecodeInfo *deInfo);

/* Open decode text file */
Status open_decode_text_file(DecodeInfo *deInfo);

#endif