#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <cmath>
#include "bmp.hpp"
#include "binarytools.hpp"

#define M_PI_16 0.196349540849362077404 // pi/16

struct DCT
{
private:
    BITMAP bmp;
    char fname_in[64];
    char fname_out[64];
    int8_t compression_parameter;
    vector<bool> bits;
    uint64_t bits_ctr;
    float compression_ratio;
    const uint8_t ZIK_ZAK_indexes[64] =
    {
        0,
        1, 8,
        16, 9, 2,
        3, 10, 17, 24,
        32, 25, 18, 11, 4,
        5, 12, 19, 26, 33, 40,
        48, 41, 34, 27, 20, 13, 6,
        7, 14, 21, 28, 35, 42, 49, 56,
        57, 50, 43, 36, 29, 22, 15,
        23, 30, 37, 44, 51, 58,
        59, 52, 45, 38, 31,
        39, 46, 53, 60,
        61, 54, 47,
        55, 62,
        63
    };
    const uint8_t quantization_start_indexes[15] =
    {
        0, 1, 3, 6, 10, 15, 21, 28, 36, 43, 49, 54, 58, 61, 63
    };

public:
    DCT(char _fname_in[64], char _fname_out[64], int8_t _compression_parameter);
    DCT(char _fname_in[64], char _fname_out[64]);
    ~DCT();

    static void show_bitmap(char fname[]);

    void load_bitmap_from_file();

    void save_bitmap_to_file();

    // The compressed bitmap goes into the binary vector
    void compress_bitmap();

    // The bitmap is decompressed from the binary vector
    void decompress_bitmap();

    // Write out the binary vector
    void save_jpeg_to_file();

    void load_jpeg_from_file();

    float calculate_compression_ratio();

private:
    void compress_block(int16_t block_in[8][8]);

    void decompress_block(int16_t block_bmp[8][8]);

    void copy_block_to_bmp(int16_t column_begin_idx, int16_t row_begin_idx, int16_t block_bmp[64]);

    void calculate_block_IDCT(int16_t block_DCT[8][8], int16_t block_bmp[8][8]);

    void copy_block_from_bmp(int16_t column_begin_idx, int16_t row_begin_idx, int16_t block_in[64]);

    void add_N_to_each_element_in_block(int16_t block_in[64], int16_t N);

    void calculate_block_DCT(int16_t block_in[8][8], int16_t block_DCT[8][8]);

    void quantize_block_DCT(int16_t block_DCT[64]);

    void push_block_DCT_to_output(int16_t block_DCT[64]);

    void read_block_DCT_from_input(int16_t block_DCT[64]);

};
