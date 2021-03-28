#include <iostream>
#include <vector>
#include <bitset>

#define N_OF_BITS_PIXEL 12
#define N_OF_BITS_BYTE 8
#define N_OF_BITS_INT16 16

using namespace std;

// Read a value from a binary vector
int16_t read_int16_from_binary_vector(vector<bool> bits, int vector_index, uint8_t n_of_bits_from_MSB);
uint16_t read_uint16_from_binary_vector(vector<bool> bits, int vector_index, uint8_t n_of_bits_from_MSB);
uint8_t read_byte_from_binary_vector(vector<bool> bits, unsigned long vector_index);

// Write a value to a binary vector
void push_int16_to_binary_vector(int16_t number, uint8_t n_of_bits_from_LSB, vector<bool>* destination);
void push_byte_to_binary_vector(uint8_t byte, vector<bool>* destination);

// Returns how many bits are actually needed to represent a given number, counting from the MSB.
uint8_t n_of_bits_to_represent_number(int16_t number);
