// gcc -o reverse reverse.c
// bunch of warnings cuz them pointer types - it's fine
// ./reverse
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>


// Functions copied from app
uint8_t encode1(uint8_t n) 
{    uint16_t start_state = n;
    uint16_t lfsr = start_state;
    uint16_t period = 0;
    do {
        lfsr ^= lfsr >> 7;
        lfsr ^= lfsr << 9;
        lfsr ^= lfsr >> 13;
        ++period;
    } while (lfsr != start_state && period != start_state);
    uint8_t res= ((lfsr >> 4) & 0xff);
    return res;
}
uint8_t encode2(uint8_t n) 
{
    uint16_t seed = n;
    uint16_t period = 0;
    do
    {
        uint16_t lsb = seed & 1;
        seed >>= 1;
        if (lsb)
            seed ^= 0x1EE7u;
        ++period;
    } while (period != seed);
    uint8_t res= ((seed >> 2) & 0xff);
  return res;
}
uint8_t encode3(uint8_t n)
{  /* taps: 16 14 13 11; feedback polynomial: x^16 + x^14 + x^13 + x^11 + 1 */
    uint16_t start_state = n;
    uint16_t lfsr = start_state;
    uint16_t bit;
    uint16_t period = 0;
    do { 
        bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1u;
        lfsr = (lfsr >> 1) | (bit << 15);
        ++period;
    } while (lfsr != start_state && period != start_state);
    uint8_t res= ((lfsr >> 3) & 0xff);
    return res;
}

// Global values copied from app
uint8_t differs_1[16] = { 0x0d, 0x21, 0x22, 0x2c, 0x23, 0x51, 0x0f, 0x14, 0x2a, 0xeb, 0x2a, 0xe2, 0x22, 0xD8, 0x26, 0xc7};
uint8_t differs_2[16] = { 0x17, 0x63, 0xfc, 0x08, 0xfd, 0x91, 0x21, 0x17, 0x07, 0xa9, 0xf6, 0x13, 0xf4, 0x51, 0x0a, 0x2a };
uint8_t differs_3[16] = { 0xcb, 0x42, 0x21, 0x06, 0x06, 0xc3, 0xd2, 0xc5, 0x00, 0x4e, 0xa2, 0x22, 0x0a, 0x9f, 0x5a, 0x05 };
uint8_t subFunc(uint8_t a, uint8_t b) // reverse named from app
{
    return a + b;
}
uint8_t addFunc(uint8_t a, uint8_t b) // reverse named from app
{
    return b - a;
}
uint8_t (*functions[16])( uint8_t, uint8_t ) = { subFunc, subFunc, subFunc, subFunc, addFunc, addFunc, addFunc, addFunc, subFunc, addFunc, subFunc, addFunc, addFunc, subFunc, addFunc, subFunc };

// lookup tables for encoding functions
uint8_t encode_values1[256];
uint8_t encode_values2[256];
uint8_t encode_values3[256];

void getEncodedValues()
{
    memset (encode_values1, '\0', 256);
    memset (encode_values2, '\0', 256);
    memset (encode_values3, '\0', 256);
    // get values from encode function into lookup table
    for (int i = 0; i <= 255; i++)
        encode_values1[i] = encode1(i);
    for (int i = 0; i <= 255; i++)
        encode_values2[i] = encode2(i);
    for (int i = 0; i <= 255; i++)
        encode_values3[i] = encode3(i);
}

int stage1 ()
{
    uint8_t stage1_values[4] = {0x33,0x40,0x32,0x35}; // encoded values provided by app
    uint8_t midvalues[16]= {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0};
    uint8_t solution[17 ] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0};

    printf("STAGE1:\n");
    printf("Encode 1 function inputs: \n");
    for (int j = 0; j< 4; j++)
    {  // loop over 4 possible output values
        printf("Result 0x%x = ", stage1_values[j]);
        for (int i = 0; i <= 255; i++)
        {
            if (stage1_values[j] == encode_values1[i]) // find if an entry exists - will solve for multiple
            {
                if (j == 0 && i < 0x7f)  // first value + limit feature
                {
                    for (int l =0; l<4; l++)
                        midvalues[(j*4)+l] = i; // store in first 4 slots
                    printf("0x%x, ", i);
                }
                if (j == 1 && i > 0x7f) // second value + limit feature
                {
                    for (int l =0; l<4; l++)
                        midvalues[(j*4)+l] = i; // store in second 4 slots
                    printf("0x%x, ", i);
                }
                if (j == 2 && i < 0x7f) // third value + limit feature
                {
                    for (int l =0; l<4; l++)
                        midvalues[(j*4)+l] = i; // store in third 4 slots
                    printf("0x%x, ", i);
                }
                if (j == 3 && i < 0x9f)// fourth value + limit feature
                {
                    for (int l =0; l<4; l++)
                        midvalues[(j*4)+l] = i; // store in fourth 4 slots
                    printf("0x%x, ", i);
                }
            }
        }
        printf("\n");
    }
    
    // Reverse mid values to original values by using reverse named functions with same function call and global differ values
    for (int i = 0; i < 4; i++)
    {
        printf("[");
        for (int k = 0; k < 4; k++)
        {
            solution[(i*4)+k] = functions[(i*4)+k](differs_1[(i*4)+k], midvalues[(i*4)+k]);
            printf("0x%x", solution[(i*4)+k]);
            if (k < 3)
                printf(",");
        }
        printf("]");
    }

    printf("\nStage 1: \"%s\"\n\n", (char *)solution);
    return 0;
}

int stage2 ()
{
    uint8_t stage2_values[4] = {0x33,0x45,0x44,0x79}; // encoded values provided by app
    uint8_t midval[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    uint8_t solution[17] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0};

    uint8_t input_key[4][20];
    uint8_t keycount[4] = {0,0,0,0};
    memset(input_key, '\0', (4*20));

    printf("STAGE2:\n");
    printf("Encode 2 function inputs: \n");
    for (int j = 0; j < 4; j++)
    {  // loop over 4 possible output values
        printf("Result 0x%x = ", stage2_values[j]);
        for (uint8_t i = 0; i < 255; i++)
        {
            if (stage2_values[j] == encode_values2[i])
            {
                if (i > 0x20 && i < 0x7f )
                {
                    if (j == 1)
                    {
                        uint8_t enc1Val = (i >> 4) + ((i & 0xf) << 4); 
                        input_key[j][keycount[j]] = enc1Val;
                    }
                    else
                        input_key[j][keycount[j]] = i;
                    printf("0x%x, ", input_key[j][keycount[j]++]);
                }
            }
        }
        printf("\n");
    }

    printf("Encode 1 function inputs: \n");
    for (int j = 0; j < 4; j++)
    {
        for (int k = 0; k < keycount[j]; k++)
        {
            printf("Result 0x%x = ", input_key[j][k]);
            for (uint8_t i = 0; i < 255; i++)
            {
                if (input_key[j][k] == encode_values1[i])
                {
                    if (j == 0 && i < 0x7f)
                    {
                        for (int l =0; l<4; l++)
                            midval[j+(l*4)] = i; // store in first 4 slots
                        printf("0x%x, ", midval[j]);
                    }
                    if (j == 1 && i > 0x7f)
                    {
                        printf("0x%x, ", i);
                        for (int l =0; l<4; l++)
                            midval[j+(l*4)] = i; // store in first 4 slots
                    }
                    if (j == 2 && i < 0x6f)
                    {
                        printf("0x%x, ", i);
                        for (int l =0; l<4; l++)
                            midval[j+(l*4)] = i; // store in first 4 slots
                    }
                    if (j == 3 && i < 0x6f)
                    {
                        printf("0x%x, ", i);
                        for (int l =0; l<4; l++)
                            midval[j+(l*4)] = i; // store in first 4 slots
                    }
                }
            }
            printf("\n");
        }
    }

    // Reverse mid values to input values by using reverse named functions with same function call and global differ values
    for (int i = 0; i < 4; i++)
    {
        printf("[");
        for (int k = 0; k < 4; k++)
        {
            solution[(i*4)+k] = functions[(i*4)+k](differs_2[(i*4)+k], midval[(i*4)+k]);
            printf("0x%x", solution[(i*4)+k]);
            if (k < 3)
                printf(",");
        }
        printf("]");
    }
    printf("\nStage 2: \"%s\"\n\n", (char *)solution);
    return 0;
}

int stage3()
{
    uint8_t stage3_values[4] = {0x2B,0x74,0x51,0x73};  // encoded values provided by app
    uint8_t midvalues[16]= {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0};
    uint8_t solution[17] = {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0};

    int count =0;

    printf("STAGE3:\n");
    printf("Encode x function inputs: \n");
    for (int j = 0; j < 4; j++)
    {  // loop over 4 possible output values
        for (uint8_t i = 0; i < 255; i++)
        { // check for values in encode 1
            if (stage3_values[j] == encode_values1[i])
            {
                printf("enc1: 0x%x = 0x%x\n", stage3_values[j], i);
                midvalues[count++] = i;
            }
        }
        for (uint8_t i = 0; i < 255; i++)
        { // check for values in encode 2
            if (stage3_values[j] == encode_values2[i] )
            {       
                if ( stage3_values[j] == 0x51 && i >= 0x6f)
                    continue;
                printf("enc2: 0x%x = 0x%x\n", stage3_values[j], i);
                midvalues[count++] = i;
            }
        }
        for (uint8_t i = 0; i < 255; i++)
        { // check for values in encode 3
            if (stage3_values[j] == encode_values3[i])
            {
                printf("enc3: 0x%x = 0x%x\n", stage3_values[j], i);
                midvalues[count++] = i;
            }
        }
    }

    uint8_t midval[16]= {0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0};
    for (int i = 0; i < count; i++)
    {
        for (int j = i; j < 16; j+=4)
        {
            uint8_t nxVal = functions[15-j](differs_3[j], midvalues[i]);
            midval[j]=nxVal;
        }
    }

    for (int i = 0; i < 4; i++)
    {
        printf("[");
        for (int k = 0; k < 4; k++)
        {
            printf("0x%x", midval[(i*4)+k]);
            if (k < 3)
                printf(",");
        }
        printf("]");
    }
    printf("\nStage 3: %s\n\n", midval);
}

// results from previous stages
uint8_t stage1vals[16] = {0x54,0x68,0x69,0x73,0x5F,0x31,0x73,0x6E,0x74,0x5F,0x74,0x68,0x65,0x5F,0x61,0x4E};
uint8_t stage2vals[16] = {0x73,0x57,0x65,0x52,0x5f,0x63,0x48,0x33,0x63,0x4b,0x5f,0x37,0x68,0x45,0x5f,0x74};
uint8_t stage3vals[16] = {0x30,0x6b,0x33,0x6e,0x5f,0x70,0x40,0x39,0x65,0x5f,0x70,0x52,0x6f,0x4c,0x6c,0x79};
// stage 4 globals
uint8_t subFun(uint8_t a, uint8_t b) // reverse named
{
    return a + b;
}
uint8_t addFun(uint8_t a, uint8_t b) // reverse named
{
    return b - a;
}
uint8_t (*asFun4[8])( uint8_t, uint8_t ) = {addFun, subFun, subFun, subFun, addFun, addFun, addFun, subFun};
uint8_t (*whichStage4[8])[16] = {stage2vals, stage2vals, stage1vals, stage3vals, stage3vals, stage1vals, stage1vals, stage3vals};
uint8_t (*encFun4a[8])(uint8_t) = {encode2, encode3, encode2, encode1, encode2, encode1, encode1, encode3};
uint8_t (*encFun4b[8])(uint8_t) = {encode2, encode3, encode2, encode3, encode2, encode1, encode3, encode3};
uint8_t offset4[8] = {1,0,1,1,0,1,0,1};
uint8_t reverse4[8] = {0,0,1,0,0,0,0,1};

int stage4()
{
    uint8_t *encSet;
    uint8_t result[16] = {  0x52,0x3b,0x2d,0x65,0x6a,0x68,0x22,0x79,
                            0x3e,0x36,0x2a,0x68,0x5c,0x22,0x3c,0x28};  // encoded values provided by app
    uint8_t values[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    printf("STAGE4:\n");
    printf("Encode x function inputs: \n");
    for (int i = 0; i < 8; i++)
    {  // loop over first half of output values
        // determine which encoding function to use
        if (encFun4a[i] == encode1) 
            encSet = encode_values1;
        if (encFun4a[i] == encode2)
            encSet = encode_values2;
        if (encFun4a[i] == encode3)
            encSet = encode_values3;
        
        for (int j = 0; j <= 255; j++)
        { // check for values in encode x function
            if (encSet[j] == result[i])
            {
                printf("Result: 0x%x = 0x%x,", result[i], j);
                values[i]=j;
            }
        }
        values[i] = asFun4[i]((*whichStage4[i])[i], values[i]); // add/sub with stage x output
        printf("  = 0x%x\n", values[i]);
    }
    for (int i = 8; i < 16; i++)
    {  // loop over second half of output values
        if (reverse4[i-8]) // is value reverse
            result[i] = (result[i] >> 4) + ((result[i] & 0xf) << 4);
        printf("Result: 0x%x = ", result[i]);

        // determine which encoding function to use
        if (encFun4b[i-8] == encode1)
            encSet = encode_values1;
        if (encFun4b[i-8] == encode2)
            encSet = encode_values2;
        if (encFun4b[i-8] == encode3)
            encSet = encode_values3;
        
        for (int j = 0; j <= 255; j++)
        { // check for values in encode x function
            if (encSet[j] == result[i])
            {
                printf("0x%x = ", j);
                values[i]=j;
            }
        }
        values[i] = addFun(offset4[i-8], values[i]); // add offset
        printf("0x%x = ", values[i]);
        values[i] = addFun(result[i-8], values[i]); // add result from first half
        printf("0x%x\n", values[i]);
    }


    for (int i = 0; i < 4; i++)
    {
        printf("[");
        for (int k = 0; k < 4; k++)
        {
            printf("0x%x", values[(i*4)+k]);
            if (k < 3)
                printf(",");
        }
        printf("]");
    }
    printf("\nStage 4: %s\n\n", values);

}

int main (int argc, char * argv[])
{
    getEncodedValues();
    stage1();
    stage2();
    stage3();
    stage4();
    return 0;
}