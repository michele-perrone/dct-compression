#include "DCT.hpp"

DCT::DCT(char _fname_in[64], char _fname_out[64], int8_t _compression_parameter)
{
    strcpy(fname_in, _fname_in);
    strcpy(fname_out, _fname_out);
    compression_parameter = _compression_parameter;
    if (compression_parameter < 0 || compression_parameter > 15)
    {
        printf("Compression parameter can be only between 0 and 15.\n"
               "You entered %d. Setting to default (8).\n", compression_parameter);
    }
}

DCT::DCT(char _fname_in[64], char _fname_out[64])
{
    strcpy(fname_in, _fname_in);
    strcpy(fname_out, _fname_out);
    compression_parameter = 0;
}

DCT::~DCT()
{
    ReleaseBitmapData(&bmp);
}

float DCT::calculate_compression_ratio()
{
    if(bits.empty())
    {
        return 0;
    }

    return (bmp.fileheader.FileSize) / (bits.size()/8.0);
}

void DCT::show_bitmap(char fname[])
{
    char command[128];
    sprintf(command, "%s %s &", "xdg-open ", fname);
    system(command);
    return;
}

void DCT::load_bitmap_from_file()
{
    FILE* fpin;

    if ((fpin = fopen (fname_in, "rb")) == NULL)
    {
        printf ("Cannot open input file. Quitting...\n");
        exit (EXIT_FAILURE);
    }

    bmp = ReadBitmap(fpin);
    fclose (fpin);
}

void DCT::save_bitmap_to_file()
{
    FILE* fpout;

    if ((fpout = fopen (fname_out, "wb")) == NULL)
    {
        printf ("Cannot open output file. Quitting...\n");
        exit (EXIT_FAILURE);
    }

    WriteBitmap(bmp, fpout);
    fclose (fpout);
}

// The compressed bitmap goes into the binary vector
void DCT::compress_bitmap()
{
    int16_t block_in[8][8];
    int16_t n_of_rows;
    int16_t n_of_columns;
    int16_t row_ctr;
    int16_t column_ctr;

    n_of_rows = n_of_columns = bmp.height;
    push_int16_to_binary_vector(n_of_rows, 16, &bits);

    // For each 8*8 block in the original BMP...
    for(column_ctr = 0; column_ctr < n_of_columns; column_ctr += 8)
    {
        for(row_ctr = 0; row_ctr < n_of_columns; row_ctr += 8)
        {
            copy_block_from_bmp(column_ctr, row_ctr, &block_in[0][0]);

            compress_block(block_in);
        }
    }
    return;
}

// The bitmap is decompressed from the binary vector
void DCT::decompress_bitmap()
{
    int16_t block_bmp[8][8];
    int16_t n_of_rows;
    int16_t n_of_columns;
    int16_t row_ctr;
    int16_t column_ctr;

    bits_ctr = 0;

    n_of_rows = n_of_columns = read_uint16_from_binary_vector(bits, bits_ctr, 16);
    bits_ctr += 16;

    bmp = CreateEmptyBitmap(n_of_rows, n_of_columns);

    // For each 8*8 DCT block...
    for(column_ctr = 0; column_ctr < n_of_columns; column_ctr += 8)
    {
        for(row_ctr = 0; row_ctr < n_of_columns; row_ctr += 8)
        {
            decompress_block(block_bmp);

            copy_block_to_bmp(column_ctr, row_ctr, &block_bmp[0][0]);
        }
    }
    return;

}

void DCT::save_jpeg_to_file()
{
    FILE* fpout = fopen(fname_out, "wb");
    uint64_t bits_to_write = bits.size();
    uint64_t bytes_to_write = ceil(bits_to_write/8.0);
    uint8_t bit_overhead = (bytes_to_write * 8) - bits_to_write;
    uint8_t current_byte = 0;

    // Add a bit overhead to the vector, so its size is a multiple of 8
    for(uint8_t overhead_ctr = 0; overhead_ctr < bit_overhead; overhead_ctr++)
    {
        bits.push_back(0);
    }

    // The first 8 bits indicate the bit overhead at the end of the file
    fwrite(&bit_overhead, 1, 1, fpout);

    // Then write out the jpeg
    for (uint64_t byte_ctr = 0; byte_ctr < bytes_to_write; byte_ctr++)
    {
        current_byte = read_byte_from_binary_vector(bits, (byte_ctr * 8));
        fwrite(&current_byte, 1, 1, fpout);
    }

    fclose(fpout);

    return;
}

void DCT::load_jpeg_from_file()
{
    FILE* fpin = fopen(fname_in, "rb");
    uint8_t bit_overhead;
    uint8_t current_byte;

    // First 8 bits indicate the bit overhead at the end of the file
    fread(&bit_overhead, 1, 1, fpin);

    // Then read the jpeg
    while(fread(&current_byte, 1, 1, fpin) != 0)
    {
        push_byte_to_binary_vector(current_byte, &bits);
    }

    // After the vector is loaded, remove the bit overhead
    for(uint8_t overhead_ctr = 0; overhead_ctr < bit_overhead; overhead_ctr++)
    {
        bits.pop_back();
    }

    fclose(fpin);

    return;
}

void DCT::compress_block(int16_t block_in[8][8])
{
    int16_t block_DCT[8][8];

    add_N_to_each_element_in_block(&block_in[0][0], -128);

    calculate_block_DCT(block_in, block_DCT);

    if(compression_parameter != 0)
    {
        quantize_block_DCT(&block_DCT[0][0]);
    }

    push_block_DCT_to_output(&block_DCT[0][0]);

    return;
}

void DCT::decompress_block(int16_t block_bmp[8][8])
{
    int16_t block_DCT[8][8];

    read_block_DCT_from_input(&block_DCT[0][0]);

    calculate_block_IDCT(block_DCT, block_bmp);

    add_N_to_each_element_in_block(&block_bmp[0][0], +128);

    return;
}

void DCT::copy_block_to_bmp(int16_t column_begin_idx, int16_t row_begin_idx, int16_t block_bmp[64])
{
    int16_t column_end_idx = column_begin_idx + 8;
    int16_t row_end_idx = row_begin_idx + 8;

    int16_t block_ctr = 0;
    int16_t row_ctr;
    int16_t column_ctr = column_begin_idx;

    while(column_ctr < column_end_idx)
    {
        row_ctr = row_begin_idx;
        while(row_ctr < row_end_idx)
        {
            PIXEL(bmp, row_ctr, column_ctr).red =
                    PIXEL(bmp, row_ctr, column_ctr).green =
                    PIXEL(bmp, row_ctr, column_ctr).blue = block_bmp[block_ctr];

            block_ctr++;

            row_ctr++;
        }
        column_ctr++;
    }
}

void DCT::calculate_block_IDCT(int16_t block_DCT[8][8], int16_t block_bmp[8][8])
{
    double element;
    int x, y, u, v;
    double Cu, Cv;

    for (x = 0; x < 8; x++)
    {
        for (y = 0; y < 8; y++)
        {
            element = 0.0;
            for (u = 0; u < 8; u++)
            {
                for (v = 0; v < 8; v++)
                {
                    if (u == 0)
                    {
                        Cu = M_SQRT1_2;
                    }
                    else
                    {
                        Cu = 1.0;
                    }
                    if (v == 0)
                    {
                        Cv = M_SQRT1_2;
                    }
                    else
                    {
                        Cv = 1.0;
                    }
                    element += (block_DCT[u][v] * Cu * Cv * cos((2 * x + 1) * u * M_PI_16) * cos((2 * y + 1) * v * M_PI_16));
                }
            }
            block_bmp[x][y] = 0.25 * element;
        }
    }

#ifdef DEBUG
    cout << "DCT block: " << endl;
    for(int i = 0; i < 8; i++)
    {
        for(int j = 0; j < 8; j++)
        {
            cout << block_DCT[i][j] << " ";
        }
        cout << endl;
    }
    cout << endl;

    cout << "Block after iDCT: " << endl;
    for(int i = 0; i < 8; i++)
    {
        for(int j = 0; j < 8; j++)
        {
            cout << block_bmp[i][j] << " ";
        }
        cout << endl;
    }
    cout << endl;
#endif

    return;

}


void DCT::copy_block_from_bmp(int16_t column_begin_idx, int16_t row_begin_idx, int16_t block_in[64])
{
    int16_t column_end_idx = column_begin_idx + 8;
    int16_t row_end_idx = row_begin_idx + 8;

    int16_t block_ctr = 0;
    int16_t row_ctr;
    int16_t column_ctr;

    column_ctr = column_begin_idx;
    while(column_ctr < column_end_idx)
    {
        row_ctr = row_begin_idx;
        while(row_ctr < row_end_idx)
        {
            block_in[block_ctr] = (PIXEL(bmp, row_ctr, column_ctr).red +
                                         PIXEL(bmp, row_ctr, column_ctr ).green +
                                         PIXEL(bmp, row_ctr, column_ctr ).blue) / 3.0;
            block_ctr++;

            row_ctr++;
        }
        column_ctr++;
    }

#ifdef DEBUG
    cout << "Copied block: " << endl;
    for(int8_t ctr = 0; ctr < 64; ctr++)
    {
        if(ctr%8==0)
        {
            cout << endl;
        }
        printf("%d ", block_in[ctr]);

    }
    cout << endl;
#endif
}


void DCT::add_N_to_each_element_in_block(int16_t block_in[64], int16_t N)
{
    for(int16_t ctr = 0; ctr < 64; ctr++)
    {
        block_in[ctr] += N;
    }
}

void DCT::calculate_block_DCT(int16_t block_in[8][8], int16_t block_DCT[8][8])
{
    double element = 0.0;

    for (int u = 0; u < 8; u++)
    {
        for (int v = 0; v < 8; v++)
        {
            element = 0.0;
            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    element += block_in[x][y] * cos(((2*x + 1) * u * M_PI_16)) *  cos(((2*y + 1) * v * M_PI_16));
                }
            }
            if ((u == 0) && (v == 0))
            {
                element /= 8.0;
            }
            else if (((u == 0) && (v != 0)) || ((u != 0) && (v == 0)))
            {
                element /= (4.0 * M_SQRT2);
            }
            else
            {
                element /= 4.0;
            }

            block_DCT[u][v] = element;
        }
    }

#ifdef DEBUG
    cout << "Calculated block DCT: " << endl;
    for(int8_t i = 0; i < 8; i++)
    {
        for(int8_t j = 0; j < 8; j++)
        {
            printf("%d ", block_DCT[i][j]);
        }
        cout << endl;
    }
    cout << endl;
#endif

    return;
}

void DCT::quantize_block_DCT(int16_t block_DCT[64])
{
    uint8_t ctr = quantization_start_indexes[15 - compression_parameter];
    while(ctr < 64)
    {
        block_DCT[ZIK_ZAK_indexes[ctr]] = 0;
        ctr++;
    }
}

void DCT::push_block_DCT_to_output(int16_t block_DCT[64])
{
    // First goes the DC component. 12 bits used.
    push_int16_to_binary_vector(block_DCT[0], 12, &bits);
#ifdef DEBUG
    cout << "DC component: " << (int)block_DCT[0] << endl;
#endif

    // Then do the AC components
    uint8_t n_of_zeros;
    uint8_t n_of_bits_for_AC;
    uint8_t ctr;

    n_of_zeros = 0;
    ctr = 1; // Because 0 is the DC component
    while(ctr < 64)
    {
        if(block_DCT[ZIK_ZAK_indexes[ctr]] == 0)
        {
            // Reset the zero counter
            n_of_zeros = 0;

            // Rule A and B put a 0 at the beginning.
            bits.push_back(0);
#ifdef DEBUG
            cout << " " << 0 << " ";
#endif

            while(block_DCT[ZIK_ZAK_indexes[ctr]] == 0 && ctr < 64)
            {
                n_of_zeros++;
                ctr++;
            }

            push_int16_to_binary_vector(n_of_zeros, 6, &bits);
#ifdef DEBUG
            cout << (int)n_of_zeros << " ";
#endif
        }

        if(block_DCT[ZIK_ZAK_indexes[ctr]] != 0 && ctr < 64)
        {
            // If there haven't been any zeros before this AC component,
            // then it's rule C.
            if(n_of_zeros == 0)
            {
                bits.push_back(1);
#ifdef DEBUG
                cout << " " << 1 << " ";
#endif
            }
            // Otherwise, reset the zero counter.
            else
            {
                n_of_zeros = 0;
            }

            n_of_bits_for_AC = n_of_bits_to_represent_number(block_DCT[ZIK_ZAK_indexes[ctr]]);
            push_int16_to_binary_vector(n_of_bits_for_AC, 4, &bits);
#ifdef DEBUG
            cout << (int)n_of_bits_for_AC << " ";
#endif

            push_int16_to_binary_vector(block_DCT[ZIK_ZAK_indexes[ctr]], n_of_bits_for_AC, &bits);
#ifdef DEBUG
            cout << (int)block_DCT[ZIK_ZAK_indexes[ctr]] << "";
#endif

            ctr++;
        }
    }
#ifdef DEBUG
    cout << endl;
#endif
    return;
}

void DCT::read_block_DCT_from_input(int16_t block_DCT[64])
{
    uint16_t block_ctr = 0;
    uint16_t n_of_bits_AC;
    uint16_t n_of_zeros;
    uint16_t zero_ctr;

    // First read the DC component (12 bits).
    block_DCT[0] = read_int16_from_binary_vector(bits, bits_ctr, 12);
    bits_ctr += 12;
    block_ctr += 1;

    while(block_ctr < 64)
    {
        // Rule C
        if(bits[bits_ctr] == 1)
        {
            bits_ctr += 1;
            n_of_bits_AC = read_uint16_from_binary_vector(bits, bits_ctr, 4);
            bits_ctr += 4;

            block_DCT[ZIK_ZAK_indexes[block_ctr]] = read_int16_from_binary_vector(bits, bits_ctr, n_of_bits_AC);
            bits_ctr += n_of_bits_AC;
            block_ctr += 1;
        }
        else if(bits[bits_ctr] == 0)
        {
            bits_ctr += 1;
            n_of_zeros = read_uint16_from_binary_vector(bits, bits_ctr, 6);
            bits_ctr += 6;

            // Add the zeros to the DCT block
            zero_ctr = 0;
            while(zero_ctr < n_of_zeros)
            {
                block_DCT[ZIK_ZAK_indexes[block_ctr]] = 0;
                zero_ctr += 1;
                block_ctr += 1;
            }

            // Rule A
            if (block_ctr < 64)
            {
                n_of_bits_AC = read_uint16_from_binary_vector(bits, bits_ctr, 4);
                bits_ctr += 4;

                block_DCT[ZIK_ZAK_indexes[block_ctr]] = read_int16_from_binary_vector(bits, bits_ctr, n_of_bits_AC);
                bits_ctr += n_of_bits_AC;
                block_ctr += 1;
            }
        }
    }
    return;
}
