#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "binarytools.hpp"
#include "DCT.hpp"

using namespace std;

int main(int argc, char* argv[])
{
    // Show a BMP
    if (*argv[1] == 'S' && argc == 3)
    {
        DCT::show_bitmap(argv[2]);
    }
    // Compress
    else if (*argv[1] == 'C' && argc == 5)
    {
        DCT* dct_compress = new DCT(argv[2], argv[3], atoi(argv[4]));

        cout << "Loading bitmap... ";
        dct_compress->load_bitmap_from_file();
        cout << "done!" << endl;

        cout << "Compressing bitmap... ";
        dct_compress->compress_bitmap();
        cout << "done!" << endl;

        cout << "Saving jpeg to file... ";
        dct_compress->save_jpeg_to_file();
        cout << "done!" << endl;

        cout << "Compression ratio: " << dct_compress->calculate_compression_ratio() <<":1" << endl;

        delete dct_compress;
    }
    // Decompress
    else if (*argv[1] == 'D' && argc == 4)
    {
        DCT* dct_decompress = new DCT(argv[2], argv[3]);

        cout << "Loading jpeg from file... ";
        dct_decompress->load_jpeg_from_file();
        cout << "done!" << endl;

        cout << "Decompressing jpeg... ";
        dct_decompress->decompress_bitmap();
        cout << "done!" << endl;

        cout << "Saving bitmap to file... ";
        dct_decompress->save_bitmap_to_file();
        cout << "done!" << endl;

        cout << "Opening up bitmap file... ";
        DCT::show_bitmap(argv[3]);
        cout << "done!" << endl;

        delete dct_decompress;
    }
    // Usage help
    else
    {
        printf("Usage:\n");
        printf("   S(how) BMP:\t\t[S] <foo.BMP>\n");
        printf("   C(ompress) BMP:\t[C] <foo.IMG> <foo.BIN> <0-15>\n");
        printf("   D(ecompress) JPEG:\t[D] <foo.BIN> <foo.BMP>\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
