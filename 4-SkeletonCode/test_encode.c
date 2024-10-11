#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "decode.h"


int main(int argc, char *argv[])
{
    EncodeInfo encInfo;
    // validate operation type
    if(check_operation_type(argv) == e_encode)
    {
        printf("Selected Encoding...\n");
        // validate the files
        if(read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            printf("Read and validate arguments is successful\n");
            if(do_encoding(&encInfo) == e_success)
            {
                printf("Encoding Completed..\n");
            }
            else
            {
                printf("FAILED Encoding the data\n");
            }
            
        }
        else
        {
            printf("Failure: Read and validate arguments is unsuccessful\n");
        }
    }
    else if(check_operation_type(argv) == e_decode)
    {
        DecodeInfo deInfo;
        printf("Selected Decoding...\n");

        // validate arguments
        if (read_validate_decode_args(argv, &deInfo) == d_success)
        {
            printf("Read and validate arguments successful\n");

            // start decoding
            if (do_decoding(&deInfo) == d_success)
            {
                printf("Decoding successful\n");
            }
            else
            {
                printf("FAILED: Encoding unsuccessful\n");
            }
            
            
        }
        else
        {
            printf("Failure: Read and arguments is unsuccessful\n");
        }
        
        
    }
    else
    {
        printf("Invalid Operation type, please use as mentioned below\n");
        printf("Usage:\n");
        printf("Encoding: ./a.out -e beautiful.bmp secret.txt stego.bmp\n");
        printf("Decoding: ./a.out -d stego.bmp decode.txt\n");
    }
    return 0;
}

OperationType check_operation_type(char *argv[])
{
    if(strcmp(argv[1], "-e") ==  0)
    {
        return e_encode;
    }
    else if(strcmp(argv[1], "-d") ==  0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}
