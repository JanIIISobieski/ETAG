/*

#include <unity.h>
#include <Tag_SD.h>

static volatile uint16_t __attribute__((aligned(16))) buffer_write[4096] = {0};

void test_sd_init() {
    //global variable sd card is initialized
    bool sd_bool = SD_begin();
    TEST_ASSERT_TRUE(sd_bool);
}

void test_file_operations() {
    TEST_ASSERT_FALSE(path_exists("file_op_test.bin"));

    bool file_bool = file_open("file_op_test.bin", O_RDWR | O_CREAT | O_TRUNC);
    TEST_ASSERT_TRUE(file_bool);
    file_bool = file_close();
    TEST_ASSERT_TRUE(file_bool);

    TEST_ASSERT_TRUE(path_exists("file_op_test.bin"));

    TEST_ASSERT_TRUE(file_rm("file_op_test.bin"));
    TEST_ASSERT_FALSE(path_exists("file_op_test.bin"));
}

void test_directory_operations() {
    TEST_ASSERT_TRUE(folder_open("/"));
    if (!path_exists("Testing")){
        TEST_ASSERT_TRUE(folder_mkdir("Testing"));
    }
    TEST_ASSERT_TRUE(path_exists("Testing"));
    TEST_ASSERT_TRUE(folder_cd("Testing"));
}

void test_serial_write_read() {
    bool file_bool = file_open("sd_test.bin", O_RDWR | O_CREAT | O_TRUNC);
    TEST_ASSERT_TRUE(file_bool);

    String string = "a\n";
    size_t written_size = file_serial_write(string);

    TEST_ASSERT_EQUAL_size_t(string.length(), written_size);

    int ind = 0;
    while (file.available()) {
        TEST_ASSERT_EQUAL_CHAR(string.charAt(ind++), file.read());
    }

    file_bool = file_close();
    TEST_ASSERT_TRUE(file_bool);
}

void test_buffer_write_read() {
    uint16_t buffer_size = 4096;
    uint16_t buffer_write[buffer_size] = {0};
    uint16_t buffer_read[buffer_size] = {0};

    for (uint16_t i = 0; i < buffer_size; i++) {
        buffer_write[i] = i;
    }

    TEST_ASSERT_EQUAL(2, sizeof(uint16_t)); //16-bits, and so 2 bytes
    size_t written_size = sizeof(uint16_t)*buffer_size;

    TEST_ASSERT_TRUE(file_open("sd_write_buffer.bin", O_RDWR | O_CREAT | O_TRUNC));

    //write the vector on the SD card
    TEST_ASSERT_EQUAL_size_t(written_size, file_write(buffer_write, written_size));

    file_rewind();

    //read the vector back from the SD card
    TEST_ASSERT_EQUAL_size_t(written_size, file_read(buffer_read, written_size));
    TEST_ASSERT_EQUAL_UINT16_ARRAY(buffer_write, buffer_read, buffer_size);
    TEST_ASSERT_TRUE(file_close());
}

void test_volatile_buffer_write_read(void) {
    uint16_t buffer_size = 4096;
    uint16_t buffer_read[buffer_size] = {0};

    for (uint16_t i = 0; i < buffer_size; i++) {
        buffer_write[i] = i;
    }

    size_t written_size = sizeof(uint16_t)*buffer_size;

    TEST_ASSERT_TRUE(file_open("sd_volatile_write_buffer.bin", O_RDWR | O_CREAT | O_TRUNC));

    //write the vector on the SD card
    TEST_ASSERT_EQUAL_size_t(written_size, file_write(buffer_write, written_size));

    file_rewind();

    //read the vector back from the SD card
    TEST_ASSERT_EQUAL_size_t(written_size, file_read(buffer_read, written_size));
    TEST_ASSERT_EQUAL_UINT16_ARRAY(buffer_write, buffer_read, buffer_size);
    TEST_ASSERT_TRUE(file_close());
}


*/