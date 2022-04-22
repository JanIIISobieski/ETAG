#include <unity.h>
#include <Tag_SD.h>
#include <Tag_ADC.h>

uint8_t id = 16;
const uint32_t buffer_size = 4096;
static volatile uint16_t __attribute__((aligned(16))) dma_adc_buff1[buffer_size];
static volatile uint16_t __attribute__((aligned(16))) dma_adc_buff2[buffer_size];

Tag_Queue buffer_queue(16);
Tag_Queue *buffer_ptr = &buffer_queue;

ADCBuffer dma1(dma_adc_buff1, dma_adc_buff2, buffer_size, id, buffer_ptr);
ADC *adc = new ADC();

void test_ADC() {
    /*
      Note: like some other tests, this test should be run after SD card has been initialized
    */
    uint16_t num_writes = 0;
    uint16_t max_writes = 160;

    const int sampling_pin = 23;

    elapsedMillis timed_write;

    //Set up pin modes
    pinMode(sampling_pin, INPUT);
   
    delay(50);

    //Ready the file
    TEST_ASSERT_TRUE(file_open("Writing_While_Sampling.bin", O_RDWR | O_CREAT | O_TRUNC));

    //Set up the ADC
    uint8_t avg = 0;
    int resolution = 12; //bits
    int frequency = 500000;

    adc_setup(adc, avg, resolution);
    dma_start(&dma1, adc, ADC_0);
    adc_start(adc, sampling_pin, frequency);

    timed_write = 0;
    while (num_writes < max_writes) {
        if (buffer_queue.num_to_write() > 0) {
            TEST_ASSERT_EQUAL(2*buffer_size, file_write(buffer_queue.pop(), 2*buffer_size));
            file_flush();
            num_writes++;
        }
    }

    dma1.stopOnCompletion(true); // note that stopOnCompletion is overloaded
    TEST_ASSERT_TRUE(dma1.stopOnCompletion()); // see above comment
    TEST_ASSERT_TRUE(file_close());

    TEST_ASSERT_EQUAL(max_writes, dma1.get_trigger_count());

    TEST_ASSERT_EQUAL(0, buffer_queue.get_pop_index()); // should have wrapped around to index 0
    TEST_ASSERT_EQUAL(0, buffer_queue.num_to_write()); // should have written everything

    delay(1000); // last buffer is still filling from dma1.stopOnCompletion()

    dma1.reset();
    buffer_queue.reset();

    TEST_ASSERT_EQUAL(0, buffer_queue.num_to_write());

    uint16_t post_reset_expectation[buffer_size] = {0};
    uint8_t* id_ptr = (uint8_t*)(&post_reset_expectation);
    *(id_ptr) = id;

    TEST_ASSERT_EQUAL_UINT8_ARRAY(post_reset_expectation, dma_adc_buff1, 2*buffer_size);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(post_reset_expectation, dma_adc_buff2, 2*buffer_size);
}