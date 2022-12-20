
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
// All of the numbers in the permutation tables are 0-indexed.  However, most
// DES references show them 1-indexed!

// Applied once at the beginning of the algorithm.
static const unsigned char initial_permutation_left[32] = {
    57, 49, 41, 33, 25, 17, 9,  1,
    59, 51, 43, 35, 27, 19, 11, 3,
    61, 53, 45, 37, 29, 21, 13, 5,
    63, 55, 47, 39, 31, 23, 15, 7
};
static const unsigned char initial_permutation_right[32] = {
    56, 48, 40, 32, 24, 16,  8, 0,
    58, 50, 42, 34, 26, 18, 10, 2,
    60, 52, 44, 36, 28, 20, 12, 4,
    62, 54, 46, 38, 30, 22, 14, 6
};

static const unsigned char initial_permutation_both[64] = {
    57, 49, 41, 33, 25, 17, 9,  1,
    59, 51, 43, 35, 27, 19, 11, 3,
    61, 53, 45, 37, 29, 21, 13, 5,
    63, 55, 47, 39, 31, 23, 15, 7,
    56, 48, 40, 32, 24, 16,  8, 0,
    58, 50, 42, 34, 26, 18, 10, 2,
    60, 52, 44, 36, 28, 20, 12, 4,
    62, 54, 46, 38, 30, 22, 14, 6
};

// Inverse of initial_permutation.  Applied once at the end of the algorithm.
static const unsigned char final_permutation[64] = {
    7, 39, 15, 47, 23, 55, 31, 63,
    6, 38, 14, 46, 22, 54, 30, 62,
    5, 37, 13, 45, 21, 53, 29, 61,
    4, 36, 12, 44, 20, 52, 28, 60,
    3, 35, 11, 43, 19, 51, 27, 59,
    2, 34, 10, 42, 18, 50, 26, 58,
    1, 33,  9, 41, 17, 49, 25, 57,
    0, 32,  8, 40, 16, 48, 24, 56
};

// Applied to the half-block at the beginning of the Fiestel function.
static const unsigned char expansion_permutation[48] = {
    31,  0,  1,  2,  3,  4,
     3,  4,  5,  6,  7,  8,
     7,  8,  9, 10, 11, 12,
    11, 12, 13, 14, 15, 16,
    15, 16, 17, 18, 19, 20,
    19, 20, 21, 22, 23, 24,
    23, 24, 25, 26, 27, 28,
    27, 28, 29, 30, 31,  0
};

// Applied at the end of the Feistel function.
static const unsigned char feistel_end_permutation[32] = {
    15,  6, 19, 20, 28, 11, 27, 16,
     0, 14, 22, 25,  4, 17, 30,  9,
     1,  7, 23, 13, 31, 26,  2,  8,
    18, 12, 29,  5, 21, 10,  3, 24
};

// Applied at the end of the Feistel function.
static const unsigned char feistel_undo_end_permutation[32] = {
    8, 16, 22, 30, 12, 27, 1, 17, 23, 15, 29, 5, 25, 19, 9, 0, 7, 13, 24, 2, 3, 28, 10, 18, 31, 11, 21, 6, 4, 26, 14, 20
};

// Converts from full 64-bit key to two key halves: left and right.  Only 48
// bits from the original key are used.
static const unsigned char permuted_choice_1[56] = {
    // Left Half
    56, 48, 40, 32, 24, 16,  8,
     0, 57, 49, 41, 33, 25, 17,
     9,  1, 58, 50, 42, 34, 26,
    18, 10,  2, 59, 51, 43, 35,
    // Right Half
    62, 54, 46, 38, 30, 22, 14,
     6, 61, 53, 45, 37, 29, 21,
    13,  5, 60, 52, 44, 36, 28,
    20, 12,  4, 27, 19, 11,  3
};

// Converts the shifted right and left key halves (concatenated together) into
// the subkey for the round (input into Feistel function).
static const unsigned char permuted_choice_2[48] = {
    13, 16, 10, 23,  0,  4,  2, 27,
    14,  5, 20,  9, 22, 18, 11,  3,
    25,  7, 15,  6, 26, 19, 12,  1,
    40, 51, 30, 36, 46, 54, 29, 39,
    50, 44, 32, 47, 43, 48, 38, 55,
    33, 52, 45, 41, 49, 35, 28, 31
};

// S-Boxes
// Each value represents 4 bits that the 6-bit input is mapped to.
//
// This is in a different order than you would normally find it in an DES
// reference, so that the sbox lookup is reduced to a single lookup on the
// input byte.
static const unsigned char sbox_0[64] = {
    14,  0,  4, 15, 13,  7,  1,  4,
     2, 14, 15,  2, 11, 13,  8,  1,
     3, 10, 10,  6,  6, 12, 12, 11,
     5,  9,  9,  5,  0,  3,  7,  8,
     4, 15,  1, 12, 14,  8,  8,  2,
    13,  4,  6,  9,  2,  1, 11,  7,
    15,  5, 12, 11,  9,  3,  7, 14,
     3, 10, 10,  0,  5,  6,  0, 13
};
static const unsigned char sbox_1[64] = {
    15,  3,  1, 13,  8,  4, 14,  7,
     6, 15, 11,  2,  3,  8,  4, 14,
     9, 12,  7,  0,  2,  1, 13, 10,
    12,  6,  0,  9,  5, 11, 10,  5,
     0, 13, 14,  8,  7, 10, 11,  1,
    10,  3,  4, 15, 13,  4,  1,  2,
     5, 11,  8,  6, 12,  7,  6, 12,
     9,  0,  3,  5,  2, 14, 15,  9
};
static const unsigned char sbox_2[64] = {
    10, 13,  0,  7,  9,  0, 14,  9,
     6,  3,  3,  4, 15,  6,  5, 10,
     1,  2, 13,  8, 12,  5,  7, 14,
    11, 12,  4, 11,  2, 15,  8,  1,
    13,  1,  6, 10,  4, 13,  9,  0,
     8,  6, 15,  9,  3,  8,  0,  7,
    11,  4,  1, 15,  2, 14, 12,  3,
     5, 11, 10,  5, 14,  2,  7, 12
};
static const unsigned char sbox_3[64] = {
     7, 13, 13,  8, 14, 11,  3,  5,
     0,  6,  6, 15,  9,  0, 10,  3,
     1,  4,  2,  7,  8,  2,  5, 12,
    11,  1, 12, 10,  4, 14, 15,  9,
    10,  3,  6, 15,  9,  0,  0,  6,
    12, 10, 11,  1,  7, 13, 13,  8,
    15,  9,  1,  4,  3,  5, 14, 11,
     5, 12,  2,  7,  8,  2,  4, 14
};
static const unsigned char sbox_4[64] = {
     2, 14, 12, 11,  4,  2,  1, 12,
     7,  4, 10,  7, 11, 13,  6,  1,
     8,  5,  5,  0,  3, 15, 15, 10,
    13,  3,  0,  9, 14,  8,  9,  6,
     4, 11,  2,  8,  1, 12, 11,  7,
    10,  1, 13, 14,  7,  2,  8, 13,
    15,  6,  9, 15, 12,  0,  5,  9,
     6, 10,  3,  4,  0,  5, 14,  3
};
static const unsigned char sbox_5[64] = {
    12, 10,  1, 15, 10,  4, 15,  2,
     9,  7,  2, 12,  6,  9,  8,  5,
     0,  6, 13,  1,  3, 13,  4, 14,
    14,  0,  7, 11,  5,  3, 11,  8,
     9,  4, 14,  3, 15,  2,  5, 12,
     2,  9,  8,  5, 12, 15,  3, 10,
     7, 11,  0, 14,  4,  1, 10,  7,
     1,  6, 13,  0, 11,  8,  6, 13
};
static const unsigned char sbox_6[64] = {
     4, 13, 11,  0,  2, 11, 14,  7,
    15,  4,  0,  9,  8,  1, 13, 10,
     3, 14, 12,  3,  9,  5,  7, 12,
     5,  2, 10, 15,  6,  8,  1,  6,
     1,  6,  4, 11, 11, 13, 13,  8,
    12,  1,  3,  4,  7, 10, 14,  7,
    10,  9, 15,  5,  6,  0,  8, 15,
     0, 14,  5,  2,  9,  3,  2, 12
};
static const unsigned char sbox_7[64] = {
    13,  1,  2, 15,  8, 13,  4,  8,
     6, 10, 15,  3, 11,  7,  1,  4,
    10, 12,  9,  5,  3,  6, 14, 11,
     5,  0,  0, 14, 12,  9,  7,  2,
     7,  2, 11,  1,  4, 14,  1,  7,
     9,  4, 12, 10, 14,  8,  2, 13,
     0, 15,  6, 12, 10,  9, 13,  0,
    15,  3,  3,  5,  5,  6,  8, 11
};

// How much the left and right key halves are shifted every round.
static const unsigned char key_shift_amounts[16] = {1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1};

// Used to selectively glitch spots in code
int glitch = 0;
int print_anyway = 0;

void print_bytes(uint8_t *block, uint64_t length )
{
    if ( block == NULL ) {
        return;
    }

    for ( int i = 0; i < length; i++) {
        fprintf(stdout, "%2x ", block[i]);
    }

    fprintf(stdout, "\n");

    return;
}

void xor(const unsigned char* block_a, const unsigned char* block_b, unsigned char* output, unsigned char nbytes) {
    for (unsigned char i=0; i<nbytes; i++) {
        output[i] = block_a[i] ^ block_b[i];
    }
}

void permute(const unsigned char* input, const unsigned char* table,
             unsigned char* output, unsigned char nbytes) {
    const unsigned char* table_cell = table;

    for (unsigned char i=0; i<nbytes; i++) {
        unsigned char result_byte = 0x00;
        for (unsigned char j=0; j<8; j++) {

            // Retrieve result_bit from lookup and store in result_byte
            unsigned char bit_pos = *table_cell % 8;
            unsigned char mask = 0x80 >> bit_pos;
            unsigned char result_bit = (input[*table_cell/8] & mask) << bit_pos;
            result_byte |= result_bit >> j;

            table_cell++;
        }
        output[i] = result_byte;
    }
}

/*
 * The key given is both halves of the key.  We shift each half of the key to
 * the left separately.
 */
void des_key_shift(unsigned char key[7], unsigned char output[7], unsigned char amount) {
    unsigned char mask;

    // Shift bytes circularly.
    // Last byte will be handled specially.
    for (unsigned char i=0; i<7; i++) {
        output[i] = (key[i] << amount) | (key[i+1] >> (8-amount));
    }

    // Middle byte straddles the two key halves.  Set the right 1 or 2 bits of
    // the left key.
    if (amount == 1) {
        mask = 0xEF;
    } else {
        mask = 0xCF;
    }
    output[3] &= mask;  // Zero the bits out
    output[3] |= (key[0] >> (4-amount)) & ~mask;  // Add left 1 or 2 bits

    // Last bit must borrow from left side of right key.
    if (amount == 1) {
        mask = 0x01;
    } else {
        mask = 0x03;
    }
    output[6] = (key[6] << amount) | ((key[3] >> (4-amount)) & mask);
}

void des_substitution_box(const unsigned char input[6], unsigned char output[4]) {
    unsigned char input_byte;

    // S-Box 0
    input_byte = (input[0] & 0xFC)>>2;
    output[0] = sbox_0[input_byte] << 4;

    input_byte = ((input[0] & 0x03)<<4) + ((input[1] & 0xF0)>>4);
    output[0] = output[0] | sbox_1[input_byte];


    // S-Box 2
    input_byte = ((input[1] & 0x0F)<<2) + ((input[2] & 0xC0)>>6);
    output[1] = sbox_2[input_byte] << 4;

    // S-Box 3
    input_byte = (input[2] & 0x3F);
    output[1] = output[1] | sbox_3[input_byte];

    // S-Box 4
    input_byte = (input[3] & 0xFC)>>2;
    output[2] = sbox_4[input_byte] << 4;

    // S-Box 5
    input_byte = ((input[3] & 0x03)<<4) + ((input[4] & 0xF0)>>4);
    output[2] = output[2] | sbox_5[input_byte];

    // S-Box 6
    input_byte = ((input[4] & 0x0F)<<2) + ((input[5] & 0xC0)>>6);
    output[3] = sbox_6[input_byte] << 4;

    // S-Box 7
    input_byte = (input[5] & 0x3F);
    output[3] = output[3] | sbox_7[input_byte];

}

void des_feistel(const unsigned char input[4], const unsigned char subkey[6], unsigned char output[4]) {
    unsigned char expanded[6];
    unsigned char sbox_output[4];

    // TODO: Can expansion be done faster than a dumb permutation algorithm?
    permute(input, expansion_permutation, expanded, 6);

    // TODO: Can xor and sbox be combined?
    xor(expanded, subkey, expanded, 6);
    des_substitution_box(expanded, sbox_output);
    permute(sbox_output, feistel_end_permutation, output, 4);

}

void des_encrypt(unsigned char block[8], unsigned char key[8], unsigned char output[8]) {
    // TODO: This whole program could probably benifit from using larger
    //       datatypes that char.
    unsigned char key_halves_a[7];  // left key + right key
    unsigned char key_halves_b[7];  // Also left key + right key
    unsigned char subkey[6];
    unsigned char fiestel_output[4];

    memset(key_halves_a, 0, 7);
    memset(key_halves_b, 0, 7);
    memset(subkey, 0, 6);
    memset(fiestel_output, 0, 4);

    // left_block and right_block must be beside eachother in memory, so the
    // memory is allocated in one chunk and the left and right block use 4
    // bytes each.
    unsigned char both_block[8];
    unsigned char* left_block = &both_block[0];
    unsigned char* right_block = &both_block[4];

    memset(both_block, 0, 8);

    permute(block, initial_permutation_left, left_block, 4);
    permute(block, initial_permutation_right, right_block, 4);

    // TODO: Pre shift permuted_choice_1 to eliminate left shift to generate
    //       the first subkey.  Or maybe even have a lookup table for each
    //       subkey.
    permute(key, permuted_choice_1, key_halves_a, 7);

    // Calculate 16 Rounds
    // Each loop iteration calculates two rounds.  This way there are no
    // memcoppies at the end of each round to for example switch right_block
    // and left_block.
    for (unsigned char i=0; i<16; i++) {
        if ( glitch == 1 && i == 1 ) {
            break;
        }

        if ( (i %2)== 0 ){
            // Generate key (even round)
            des_key_shift(key_halves_a, key_halves_b, key_shift_amounts[i]);
            permute(key_halves_b, permuted_choice_2, subkey, 6);

            // Round calculation (even round)
            des_feistel(right_block, subkey, fiestel_output);
            xor(fiestel_output, left_block, left_block, 4);
        } else {
            // Generate key (odd round)
            des_key_shift(key_halves_b, key_halves_a, key_shift_amounts[i]);
            permute(key_halves_a, permuted_choice_2, subkey, 6);

            // Round calculation (odd round)
            des_feistel(left_block, subkey, fiestel_output);
            xor(fiestel_output, right_block, right_block, 4);
        }
        

        
    }

    // Switch back left and right block
    // This step is skipped, since the final permutation has been modified to
    // account for it.

    permute(left_block, final_permutation, output, 8);
}

void ShiftRightOne( unsigned char input[7], unsigned char output[7] )
{
    unsigned char upper_bit = 0;

    for (unsigned char i=0; i<7; i++) {
        output[i] = (input[i] >> 1) | (upper_bit << 7 );
        upper_bit = (input[i] >> 7) & 1;
    }

    // Move the final bit
    output[0] = output[0] | (upper_bit << 7);

    return;
}

unsigned char undo_perm_choice_2[64] = {
     4, 23,  6, 15,  5,  9, 19, 17, 
    -1, 11,  2, 14, 22,  0,  8, 18, 
     1, -1, 13, 21, 10, -1, 12,  3, 
    -1, 16, 20,  7, 46, 30, 26, 47, 
    34, 40, -1, 45, 27, -1, 38, 31, 
    24, 43, -1, 36, 33, 42, 28, 35, 
    37, 44, 32, 25, 41, -1, 29, 39
};

unsigned char undo_perm_choice_1[64] = {
    7, 15, 23, 55, 51, 43, 35, -1, 
    6, 14, 22, 54, 50, 42, 34, -1, 
    5, 13, 21, 53, 49, 41, 33, -1, 
    4, 12, 20, 52, 48, 40, 32, -1, 
    3, 11, 19, 27, 47, 39, 31, -1, 
    2, 10, 18, 26, 46, 38, 30, -1, 
    1,  9, 17, 25, 45, 37, 29, -1, 
    0,  8, 16, 24, 44, 36, 28, -1
};

void permute2(const unsigned char* input, const unsigned char* table,
             unsigned char* output, unsigned char nbytes) {
    const unsigned char* table_cell = table;

    for (unsigned char i=0; i<nbytes; i++) {
        unsigned char result_byte = 0x00;
        for (unsigned char j=0; j<8; j++) {
            // Retrieve result_bit from lookup and store in result_byte
            unsigned char bit_pos = *table_cell % 8;
            if ( *table_cell == 255 ) {
                table_cell++;
                continue;
            }

            unsigned char mask = 0x80 >> bit_pos;
            unsigned char result_bit = (input[*table_cell/8] & mask) << bit_pos;
            result_byte |= result_bit >> j;

            table_cell++;
        }
        output[i] = result_byte;
    }
}

/*
    Take 8 bytes from a potential pre-sbox transformation and concatenate it to 6-bytes.

    Each byte contains 6 useful bits
 */
void ConcatFromSbox( unsigned char sb0, 
                     unsigned char sb1, 
                     unsigned char sb2, 
                     unsigned char sb3,
                     unsigned char sb4,
                     unsigned char sb5,
                     unsigned char sb6,
                     unsigned char sb7,
                     unsigned char out[6])
{
    out[0] = ((sb0 << 2) & (0xFC)) | ( (sb1 >> 4) & 0x3);

    out[1] = ((sb1 << 4) & (0xF0)) | ( (sb2 >> 2) & 0xF);

    out[2] = ((sb2 << 6) & (0xC0)) | (sb3 & 0x3F);

    out[3] = ((sb4 << 2) & (0xFC)) | ( (sb5 >> 4) & 0x3);

    out[4] = ((sb5 << 4) & (0xF0)) | ( (sb6 >> 2) & 0xF);

    out[5] = ((sb6 << 6) & (0xC0)) | (sb7 & 0x3F);

    return;
}

void GeneratePossibleExpandedKeys( unsigned char *post_sbox, int length )
{
    unsigned char sbox_0_potentials[64];
    unsigned char sbox_1_potentials[64];
    unsigned char sbox_2_potentials[64];
    unsigned char sbox_3_potentials[64];
    unsigned char sbox_4_potentials[64];
    unsigned char sbox_5_potentials[64];
    unsigned char sbox_6_potentials[64];
    unsigned char sbox_7_potentials[64];

    int sb0p_len = 0;
    int sb1p_len = 0;
    int sb2p_len = 0;
    int sb3p_len = 0;
    int sb4p_len = 0;
    int sb5p_len = 0;
    int sb6p_len = 0;
    int sb7p_len = 0;

    memset(sbox_0_potentials, 0, 64);
    memset(sbox_1_potentials, 0, 64);
    memset(sbox_2_potentials, 0, 64);
    memset(sbox_3_potentials, 0, 64);
    memset(sbox_4_potentials, 0, 64);
    memset(sbox_5_potentials, 0, 64);
    memset(sbox_6_potentials, 0, 64);
    memset(sbox_7_potentials, 0, 64);

    unsigned char sb0 = (post_sbox[0] >> 4) & 0xf;
    unsigned char sb1 = post_sbox[0] & 0xf;
    unsigned char sb2 = (post_sbox[1] >> 4) & 0xf;
    unsigned char sb3 = post_sbox[1] & 0xf;
    unsigned char sb4 = (post_sbox[2] >> 4) & 0xf;
    unsigned char sb5 = post_sbox[2] & 0xf;
    unsigned char sb6 = (post_sbox[3] >> 4) & 0xf;
    unsigned char sb7 = post_sbox[3] & 0xf;

    printf("%x %x %x %x %x %x %x %x\n", sb0, sb1, sb2, sb3, sb4, sb5, sb6, sb7);

    for ( int i = 0; i < 64; i++ ) {
        if ( sbox_0[i] == sb0) {
            sbox_0_potentials[sb0p_len++] = i;
        }
    }

    for ( int i = 0; i < 64; i++ ) {
        if ( sbox_1[i] == sb1) {
            sbox_1_potentials[sb1p_len++] = i;
        }
    }

    for ( int i = 0; i < 64; i++ ) {
        if ( sbox_2[i] == sb2) {
            sbox_2_potentials[sb2p_len++] = i;
        }
    }

    for ( int i = 0; i < 64; i++ ) {
        if ( sbox_3[i] == sb3) {
            sbox_3_potentials[sb3p_len++] = i;
        }
    }

    for ( int i = 0; i < 64; i++ ) {
        if ( sbox_4[i] == sb4) {
            sbox_4_potentials[sb4p_len++] = i;
        }
    }

    for ( int i = 0; i < 64; i++ ) {
        if ( sbox_5[i] == sb5) {
            sbox_5_potentials[sb5p_len++] = i;
        }
    }

    for ( int i = 0; i < 64; i++ ) {
        if ( sbox_6[i] == sb6) {
            sbox_6_potentials[sb6p_len++] = i;
        }
    }

    for ( int i = 0; i < 64; i++ ) {
        if ( sbox_7[i] == sb7) {
            sbox_7_potentials[sb7p_len++] = i;
        }
    }


    unsigned char concatted_sbox[6];
    memset(concatted_sbox, 0, 6);

    unsigned char sbb[8] = { 0x1, 0x1c, 0xc, 0x1a, 0x3e, 0x3f, 0x3e, 0x3f};
    
    ConcatFromSbox(sbb[0],sbb[1],sbb[2],sbb[3],sbb[4],sbb[5],sbb[6],sbb[7], concatted_sbox);

    printf("Found Sbox: ");
    print_bytes( concatted_sbox, 6);

    unsigned char testout[7];

    memset( testout, 0, 7);

    /// This cannot recover 8 bits [ 8, 17, 21, 24, 34, 37, 42,53]
    permute2( concatted_sbox, undo_perm_choice_2, testout, 6);

    printf("After Perm Choice 2 Undo: ");
    print_bytes(testout, 7);

    unsigned char shiftedright[7];

    memset(shiftedright, 0, 7);

    ShiftRightOne(testout, shiftedright);

    printf("Shifted: ");
    print_bytes(shiftedright, 7);

    unsigned char finkey[8];
    memset( finkey,0, 8);

    permute2(shiftedright, undo_perm_choice_1, finkey, 7);
    printf("Finkey: \t");
    print_bytes(finkey, 8);

    return;
}

void SetKeyBit( unsigned char key[8], int bit )
{
    unsigned char bit_pos = bit % 8;
    unsigned char byte_pos = bit / 8;

    key[byte_pos] = key[byte_pos] | (1 << bit_pos);

    return;
}

// bits to set: [14 23 38 53 55 60]
/*
  Input:
    c - the 4 bit value of the sbox to find
    bits - the bits in the key that affect this sbox
    start_bit - the offset into the final sbox where this value is
    masks - array used to hold the bit masks that result in the sbox. This is a binary value that indicates which bits in the bits array should be set to 1
Output:
    The masks array will be filled in with bit combinations that result in the requested sbox.
*/
int BruteSbox(unsigned char c, unsigned char bits[6], int start_bit, unsigned char masks[6] )
{
    unsigned char plaintext[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned char key[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned char ciphertext[8];
    unsigned char back_out_permute[8];
    unsigned char back_out_end_feistel_permute[4];
    unsigned char tb = 0;
    int mask_count = 0;

    memset(ciphertext, 0, 8);

    // 2^6 possibilities for each byte
    for ( int i = 0; i < 64; i++ ){
        // Start with a clean slate
        memset( key, 0, 8);
        memset(ciphertext, 0, 8);
        memset(back_out_permute, 0, 8);
        memset(back_out_end_feistel_permute, 0, 4);

        int j = i;
        int cbit = 0;

        while ( j ) {
            if ( j & 1 ) {
                SetKeyBit(key, bits[cbit]);
            }

            j = j >> 1;
            cbit += 1;
        }

        // The key is set up so do the encryption
        des_encrypt(plaintext, key, ciphertext);

        // Undo the initial permutation
        permute(ciphertext, initial_permutation_both, back_out_permute, 8);

        // Get back the sbox
        permute(back_out_permute+4, feistel_undo_end_permutation, back_out_end_feistel_permute, 4);

        tb = back_out_end_feistel_permute[start_bit/8];

        if ( start_bit % 8 ) {
            tb = tb & 0xf;
        } else {
            tb = (tb >> 4) & 0xf;
        }

        if ( tb == c ) {
            masks[mask_count] = i;
            mask_count++;
        }
    }

    return 0;
}

// Given an 8 byte key set the requested bits based on the mask
void SetKeyWithMask( unsigned char key[8], unsigned char bits[6], unsigned char mask)
{
    int j = mask;
    int cbit = 0;

    while ( j ) {
        if ( j & 1 ) {
            SetKeyBit(key, bits[cbit]);
        }

        j = j >> 1;
        cbit += 1;
    }

    return;
}

// The meat of the matter finding the key.
void BreakIt(unsigned char desired_sbox[4], unsigned char known_pt[8], unsigned char known_ct[8], unsigned char outkey[8], unsigned char expected[8])
{
    unsigned char plaintext[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    // These are the 8 bits that are lost from the compression permutation
    unsigned char brutebits[8] = { 1, 2, 12, 13, 42, 45, 52, 54};

    unsigned char ciphertext[8];
    memset(ciphertext, 0, 8);
    
    
    // These arrays store the 6 possible sbox values that can result in a particular output
    unsigned char sbox0_masks[6] = {0};
    unsigned char sbox1_masks[6] = {0};
    unsigned char sbox2_masks[6] = {0};
    unsigned char sbox3_masks[6] = {0};
    unsigned char sbox4_masks[6] = {0};
    unsigned char sbox5_masks[6] = {0};
    unsigned char sbox6_masks[6] = {0};
    unsigned char sbox7_masks[6] = {0};
    
    // This is how the bits affect the sboxes in the first round of DES
    // sbox 0 14 23 38 53 55 60
    // sbox 1  6 15 21 39 46 63
    // sbox 2  5 30 31 37 44 62 32->31?
    // sbox 3  7 22 29 36 47 61
    // sbox 4  4 18 28 33 35 50
    // sbox 5  3 17 26 27 41 51
    // sbox 6  9 19 20 34 57 59 69->59
    // sbox 7 10 11 25 43 49 58
    // No effect: 0 1 2 8 12 13 16 24 32 40 42 45 48 52 54 56

    unsigned char sbox_zero_bits[6] = { 14, 23, 38, 53, 55, 60 };
    unsigned char sbox_one_bits[6] = { 6, 15, 21, 39, 46, 63 };
    unsigned char sbox_two_bits[6] = { 5, 30, 31, 37, 44, 62 };

    unsigned char sbox_thr_bits[6] = {7, 22, 29, 36, 47, 61};
    unsigned char sbox_for_bits[6] = {4,  18, 28, 33, 35, 50};
    unsigned char sbox_fiv_bits[6] = {3, 17, 26, 27, 41, 51};
    unsigned char sbox_six_bits[6] = {9, 19, 20, 34, 57, 59};
    unsigned char sbox_sev_bits[6] = {10, 11, 25, 43, 49, 58};

    // 5 fc ed 2b
    BruteSbox((desired_sbox[0] >> 4) & 0xf, sbox_zero_bits, 0, sbox0_masks);
    BruteSbox(desired_sbox[0] & 0xf, sbox_one_bits, 4, sbox1_masks);
    BruteSbox((desired_sbox[1] >> 4), sbox_two_bits, 8, sbox2_masks);
    BruteSbox(desired_sbox[1] & 0xf, sbox_thr_bits, 12, sbox3_masks);
    BruteSbox((desired_sbox[2] >> 4), sbox_for_bits, 16, sbox4_masks);
    BruteSbox(desired_sbox[2] & 0xf, sbox_fiv_bits, 20, sbox5_masks);
    BruteSbox((desired_sbox[3] >> 4), sbox_six_bits, 24, sbox6_masks);
    BruteSbox(desired_sbox[3] & 0xf, sbox_sev_bits, 28, sbox7_masks);

    // At this point there are 4^8 possible keys to produce the output cipher text
    // Along with this there are 2^8 bits that do not affect the S-boxs in the first round
    //      due to the compression permutation (Not including parity bits). This leaves
    //      a total of 4^8 * 2^8 possible keys.
    for ( int a = 0; a < 4; a++ ) {
        unsigned char keya[8] = {0};
        // Do this out here so that we don't have to set it every time
        SetKeyWithMask(keya, sbox_zero_bits, sbox0_masks[a]);

        for ( int b = 0; b < 4; b++) {
            unsigned char keyb[8] = {0};
            memcpy(keyb, keya, 8);

            SetKeyWithMask(keyb, sbox_one_bits, sbox1_masks[b]);

            for ( int c = 0; c < 4; c++) {
                unsigned char keyc[8] = {0};
                memcpy(keyc, keyb, 8);

                SetKeyWithMask(keyc, sbox_two_bits, sbox2_masks[c]);
                
                for ( int d = 0; d < 4; d++) {
                    unsigned char keyd[8] = {0};
                    memcpy(keyd, keyc, 8);

                    SetKeyWithMask(keyd, sbox_thr_bits, sbox3_masks[d]);

                    for ( int e = 0; e < 4; e++) {
                        unsigned char keye[8] = {0};
                        memcpy(keye, keyd, 8);

                        SetKeyWithMask(keye, sbox_for_bits, sbox4_masks[e]);

                        for ( int f = 0; f < 4; f++) {
                            unsigned char keyf[8] = {0};
                            memcpy(keyf, keye, 8);

                            SetKeyWithMask(keyf, sbox_fiv_bits, sbox5_masks[f]);

                            for ( int g = 0; g < 4; g++) {
                                unsigned char keyg[8] = {0};
                                memcpy(keyg, keyf, 8);

                                SetKeyWithMask(keyg, sbox_six_bits, sbox6_masks[g]);

                                for ( int h = 0; h < 4; h++) {
                                    unsigned char keyh[8] = {0};
                                    memcpy(keyh, keyg, 8);

                                    SetKeyWithMask(keyh, sbox_sev_bits, sbox7_masks[h]);

                                    for (int bb = 0; bb < 256; bb++) {
                                        unsigned char keyi[8] = {0};
                                        memcpy(keyi, keyh, 8);

                                        SetKeyWithMask(keyi, brutebits, bb);

                                        des_encrypt(plaintext, keyi, ciphertext);

                                        if ( memcmp(ciphertext, expected, 8) == 0 ) {

                                            memset(ciphertext, 0, 8);
                                            glitch = 0;
                                            des_encrypt(known_pt, keyi, ciphertext);
                                            glitch = 1;

                                            if ( memcmp(ciphertext, known_ct, 8) == 0 ){
                                                printf("KEY REALLY FOUND: ");
                                                print_bytes(keyi, 8);
                                                memcpy(outkey, keyi, 8);
                                                return;
                                            }
                                        }
                                    }
                                    
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    printf("JUST RETURNING NOT FOUND\n");
    return;
}

void SetDESParityBits( unsigned char input[8], unsigned char output[8])
{
    int count = 0;
    unsigned char byte = 0;

    for ( int i = 0; i < 8; i++) {
        count = 0;

        byte = input[i] >> 1;

        for (int j = 0; j < 7; j++) {
            if ( byte & 1) {
                count++;
            }

            byte = byte >> 1;
        }

        byte = input[i] & 0xfe;

        // If the byte has odd parity set the bit to make it even
        if ( count % 2 ) {
            byte |= 1;
        }

        output[i] = byte;
    }
}

int main(int argc, char **argv)
{
    int opt, fd;

    // Filename for the output keyfile
    char *keyfile = NULL;

    // I need a single good plaintext/cryptext pair
    // Filename for the known plaintext
    char *goodplainfile = NULL;

    // Filename for the known cryptext
    char *goodcryptfile = NULL;

    // Filename for the encrypted null bytes
    char *nullbytecipher = NULL;

    // This will hold the final leaked key. It should be the same as key
    unsigned char leaked_key[8];

    // Now run DES with the glitch
    glitch = 1;

    unsigned char ciphertext[8];
    memset(ciphertext, 0, 8);

    unsigned char good[8];
    memset(good, 0, 8);

    unsigned char goodcipher[8];
    memset(goodcipher, 0, 8);

    while((opt = getopt(argc, argv, "k:p:c:n:")) != -1) { 
        switch(opt) { 
            case 'k': 
                if ( keyfile ) {
                    printf("duplicate keyfile\n");
                    break;
                }

                keyfile = strdup(optarg);

                break;
            case 'p': 
                if ( goodplainfile ) {
                    printf("duplicate known plaintext\n");
                    break;
                }

                goodplainfile = strdup(optarg);

                break;
            case 'c': 
                if ( goodcryptfile ) {
                    printf("duplicate known ciphertext\n");
                    break;
                }

                goodcryptfile = strdup(optarg);
                break;
            case 'n': 
                if ( nullbytecipher ) {
                    printf("duplicate null byte ciphertext\n");
                    break;
                }

                nullbytecipher = strdup(optarg);
                break;
            case ':': 
                printf("option needs a value\n"); 
                break; 
            case '?': 
                printf("unknown option: %c\n", optopt);
                break; 
        } 
    } 

    if ( !keyfile || !goodplainfile || !goodcryptfile || !nullbytecipher) {
        printf("missing options\n");
        exit(0);
    }

    fd = open( goodplainfile, O_RDONLY);

    if ( fd < 0 ) {
        printf("failed to open goodplainfile\n");
        exit(0);
    }

    read(fd, good, 8);
    close(fd);

    fd = open( goodcryptfile, O_RDONLY);

    if ( fd < 0 ) {
        printf("failed to open goodcipher\n");
        exit(0);
    }

    read(fd, goodcipher, 8);
    close(fd);

    fd = open( nullbytecipher, O_RDONLY);

    if ( fd < 0 ) {
        printf("failed to open goodcipher\n");
        exit(0);
    }

    read(fd, ciphertext, 8);
    close(fd);

    printf("Read null: ");
    print_bytes(ciphertext, 8);

    unsigned char back_out_permute[8];
    unsigned char back_out_end_feistel_permute[4];

    memset(back_out_permute, 0, 8);
    memset(back_out_end_feistel_permute, 0, 4);

    // This undoes the final permutation
    permute(ciphertext, initial_permutation_both, back_out_permute, 8);

    // This gets back to the output from the sboxes.
    permute(back_out_permute+4, feistel_undo_end_permutation, back_out_end_feistel_permute, 4);

    printf("\nStarting the search for the good key now.\n");

    BreakIt( back_out_end_feistel_permute, good, goodcipher, leaked_key, ciphertext );

    // Make sure to set the parity bits. Otherwise the key comparison could fail even if valid
    SetDESParityBits(leaked_key, leaked_key);

    printf("Leaked key: ");
    print_bytes(leaked_key, 8);

    fd = open( keyfile, O_RDWR | O_CREAT);

    if ( fd < 0 ) {
        printf("failed to open keyfile\n");
        exit(0);
    }

    write(fd, leaked_key, 8);
    close(fd);
}
