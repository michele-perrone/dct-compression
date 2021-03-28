#include "binarytools.hpp"

int16_t read_int16_from_binary_vector(vector<bool> bits, int vector_index, uint8_t n_of_bits_from_MSB)
{
    // By default the number is assumed positive (filled with 0-s)
    int16_t number = 0;

    // However if the first bit is 1, then the number is negative (2s complement)...
    if((bits[vector_index] & 1) == 1)
    {
        // ... and it's necessary to "fill it" with 1-s until the MSB.
        number = ~number;
        number = number << n_of_bits_from_MSB;
    }

    for (int shift_index = n_of_bits_from_MSB-1; shift_index >= 0; shift_index--)
    {
        number |= (bits[vector_index] << shift_index);
        vector_index++;
    }

    return number;
}

uint16_t read_uint16_from_binary_vector(vector<bool> bits, int vector_index, uint8_t n_of_bits_from_MSB)
{
    uint16_t number = 0;

    for (int shift_index = n_of_bits_from_MSB-1; shift_index >= 0; shift_index--)
    {
        number |= (bits[vector_index] << shift_index);
        vector_index++;
    }

    return number;
}

uint8_t read_byte_from_binary_vector(vector<bool> bits, unsigned long vector_index)
{
    uint8_t byte = 0;
    for (int shift_index = N_OF_BITS_BYTE-1; shift_index >= 0; shift_index--)
    {
        byte |= (bits[vector_index] << shift_index);
        vector_index++;
    }

    return byte;
}

void push_int16_to_binary_vector(int16_t number, uint8_t n_of_bits_from_LSB, vector<bool>* destination)
{
    for (int shift_index = n_of_bits_from_LSB-1; shift_index >= 0; shift_index--)
    {
        destination->push_back((number >> shift_index) & 1);
    }
}

void push_byte_to_binary_vector(uint8_t byte, vector<bool>* destination)
{
    for (int shift_index = N_OF_BITS_BYTE-1; shift_index >= 0; shift_index--)
    {
        destination->push_back((byte >> shift_index) & 1);
    }
}

uint8_t n_of_bits_to_represent_number(int16_t number)
{
    /* With N bits, the represented range in 2's complement is:
     *              -2^(N-1) ... +2^(N-1)-1
     * the opposite sign of a number is achieved by negating the number and adding 1.
     * Used trick for negative numbers: if I don't add 1 after negating them,
     * I can calculate the number of needed bits the same way I do it for the positive ones.
     */

    int shift_index = N_OF_BITS_INT16;

    // If the number is negative, make it positive.
    // Note: -1 will become 0, so I can treat them the same way.
    if(number < 0)
    {
        number = ~number;
    }

    // If the number is 0 (or a -1 which has been negated), I need one bit.
    if(number == 0)
    {
        return 1;
    }

    // Start shifting from the left (MSB).
    // As soon as a 1 is encountered, stop.
    for (shift_index = N_OF_BITS_INT16-1; shift_index >= 0; shift_index--)
    {
        if(((number >> shift_index) & 1) == 1)
        {
            // Reason for +2:
            // One bit added because shift_index starts from 15, not 16.
            // Second bit added for the 2's complement.
            return (shift_index + 2);
        }
    }

    return N_OF_BITS_INT16;
}
