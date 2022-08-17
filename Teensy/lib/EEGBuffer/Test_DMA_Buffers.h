#include <unity.h>
#include "DMAChannel.h"

#define SMLOE  (1<<31)
#define DMLOE  (1<<30)
#define MLOFF(n)  (n<<10)
#define NBYTES(n) (n<<0)

void dumpDMA_TCD(DMABaseClass *dmabc)
{
	Serial.printf("%x %x:", (uint32_t)dmabc, (uint32_t)dmabc->TCD);

	Serial.printf("SA:%x SO:%d AT:%x NB:%x SL:%d DA:%x DO: %d CI:%x DL:%x CS:%x BI:%x\n", (uint32_t)dmabc->TCD->SADDR,
		dmabc->TCD->SOFF, dmabc->TCD->ATTR, dmabc->TCD->NBYTES, dmabc->TCD->SLAST, (uint32_t)dmabc->TCD->DADDR, 
		dmabc->TCD->DOFF, dmabc->TCD->CITER, dmabc->TCD->DLASTSGA, dmabc->TCD->CSR, dmabc->TCD->BITER);
}

void test_dma_copy() {
    const int data_buffer_length = 4;
    volatile uint8_t data_buffer[data_buffer_length];

    const int final_buffer_length = 10;
    volatile uint8_t final_buffer[final_buffer_length] = {0};

    for (size_t i = 0; i < data_buffer_length; i++) {
        data_buffer[i] = i+1;
    }
    
    DMAChannel dma_channel1;

    dma_channel1.sourceBuffer(data_buffer, data_buffer_length);
    dma_channel1.destinationCircular(final_buffer, final_buffer_length);
    dma_channel1.TCD->NBYTES = 0b10111111111111111111000000000100;

    dumpDMA_TCD(&dma_channel1);

    dma_channel1.triggerManual();

    dumpDMA_TCD(&dma_channel1);

    dma_channel1.triggerManual();

    dumpDMA_TCD(&dma_channel1);

    delay(1000);

    Serial.println("------------------");
    for (size_t i = 0; i < final_buffer_length; i++)
    {
        Serial.println(final_buffer[i]);
    }
    

    //TEST_ASSERT_EQUAL_UINT8_ARRAY(data_buffer1, final_buffer, data_buffer_length);
}