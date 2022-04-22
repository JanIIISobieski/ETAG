#include <unity.h>

void test_bitfields() {
    // For this to work, this union must be constructed backwards, from the right-most byte to the left-most byte
    // i.e. This register is RESERVED1, DAISY_EN, CLK_EN, RESERVED2, DR in the register map in the documentation, while the C++ union is 
    // DR, RESERVED2, CLK_EN, DAISY_EN, RESERVED1 since the bit field is constructed from right to left.
    typedef union {
        struct {
            uint8_t DR:3;           // Output data rate. 000 -> 16000 SPS, 110 -> 250 SPS,
            uint8_t Reserved2:2;    // Reserved, always write 2h
            uint8_t CLK_EN:1;       // CLK Connection
            uint8_t DAISY_EN:1;     // Daisy-chain (0) or Multiple Readback (1)
            uint8_t Reserved1:1;    // Always write 1h
        } fields;
        uint8_t byte_value;
    } ADS_CONFIG_1_REGISTER;

    ADS_CONFIG_1_REGISTER config1;
    config1.fields.Reserved1 = 1;  //Assigning things from left bit to right bit for readability
    config1.fields.DAISY_EN = 1;
    config1.fields.CLK_EN = 1;
    config1.fields.Reserved2 = 2;
    config1.fields.DR = 0;

    //Expect    1       1         1        10      000
    //        Res1   DSY_EN    CLK_EN     RES2      DR
    TEST_ASSERT_BITS_HIGH(0b11110000, config1.byte_value);  // Check the bitmap

    config1.fields.Reserved1 = 0;
    config1.fields.DAISY_EN = 0;
    config1.fields.CLK_EN = 0;
    config1.fields.Reserved2 = 1;
    config1.fields.DR = 7;

    TEST_ASSERT_EQUAL(0b00001111, config1.byte_value);  // Check the bitmap

    config1.fields.Reserved1 = 1;
    config1.fields.DAISY_EN = 1;
    config1.fields.CLK_EN = 1;
    config1.fields.Reserved2 = 0;
    config1.fields.DR = 6;

    TEST_ASSERT_EQUAL(0b11100110, config1.byte_value);
}