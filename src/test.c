#include <stdio.h>
#include <string.h>
#include <msgpack.h>

#define ROWS 3
#define COLS 4

void packFloat2D(float array[ROWS][COLS], msgpack_packer *packer)
{
    msgpack_pack_array(packer, ROWS);
    for (int i = 0; i < ROWS; i++)
    {
        msgpack_pack_array(packer, COLS);
        for (int j = 0; j < COLS; j++)
        {
            msgpack_pack_float(packer, array[i][j]);
        }
    }
}

void unpackFloat2D(float array[ROWS][COLS], msgpack_object_array *objArray)
{
    for (int i = 0; i < ROWS; i++)
    {
        msgpack_object_array row = objArray[i].via.array;
        for (int j = 0; j < COLS; j++)
        {
            array[i][j] = row.ptr[j].via.f64;
        }
    }
}

int main()
{
    // Create a 2D float array
    float originalArray[ROWS][COLS] = {
        {1.1, 2.2, 3.3, 4.4},
        {5.5, 6.6, 7.7, 8.8},
        {9.9, 10.10, 11.11, 12.12}};

    // Initialize a buffer for packing and unpacking
    size_t buffer_size = 1024;
    char buffer[buffer_size];

    // Pack the 2D float array into the buffer
    msgpack_sbuffer sbuffer;
    msgpack_sbuffer_init(&sbuffer);
    msgpack_packer packer;
    msgpack_packer_init(&packer, &sbuffer, msgpack_sbuffer_write);

    packFloat2D(originalArray, &packer);

    // Unpack the 2D float array from the buffer
    msgpack_unpacked unpacked;
    msgpack_unpacked_init(&unpacked);

    memcpy(buffer, sbuffer.data, sbuffer.size);

    // Unpack the data from the buffer
    msgpack_unpack_return result = msgpack_unpack_next(&unpacked, buffer, sbuffer.size, NULL);

    // Check if the unpacking was successful
    if (result == MSGPACK_UNPACK_SUCCESS)
    {
        // Check if the unpacked object is an array
        if (unpacked.data.type == MSGPACK_OBJECT_ARRAY)
        {
            msgpack_object_array objArray = unpacked.data.via.array;
            if (objArray.size == ROWS && objArray.ptr[0].type == MSGPACK_OBJECT_ARRAY &&
                objArray.ptr[0].via.array.size == COLS)
            {
                float unpackedArray[ROWS][COLS];
                unpackFloat2D(unpackedArray, objArray.ptr); // Pass objArray.ptr as argument

                // Print the unpacked 2D float array
                printf("Unpacked 2D float array:\n");
                for (int i = 0; i < ROWS; i++)
                {
                    for (int j = 0; j < COLS; j++)
                    {
                        printf("%f ", unpackedArray[i][j]);
                    }
                    printf("\n");
                }
            }
            else
            {
                printf("Error: Unpacked object is not a 2D float array\n");
            }
        }
        else
        {
            printf("Error: Unpacked object is not an array\n");
        }
    }
    else
    {
        printf("Error: Failed to unpack data\n");
    }

    // Clean up the resources
    msgpack_sbuffer_destroy(&sbuffer);
    msgpack_unpacked_destroy(&unpacked);

    return 0;
}
