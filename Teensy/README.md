# EEG Tag
Repository for the modular EEG Tag that will form the base functions for the Tag.

# Software
The code is written in Microsoft Visual Studio Code using the PlatformIO extension.
The latter is especially recommended since it makes writing software for the Teensy (and other microcontrollers) much smoother and easier.

## Useful console commands
  * `pio run logger -t upload`: upload the logging version of the code onto the Teensy
  * `pio run teensy40 -t upload`: upload version with no logging onto the Teensy
  * `pio device monitor -p /dev/ttyACM1 > /debug/debug.txt`: start the serial monitor listening to port /dev/ttyACM1 and pipe its outputs to debug.txt in debug folder. This will save the log data for checking afterwards.
  * `doxygen <DOXYFILE>`: update the Doxygen build for documentation

# SD Card Benchmark
The benchmark is based on the [TeensySdioDemo.ino from SdFat-beta](https://github.com/greiman/SdFat-beta/blob/master/examples/TeensySdioDemo/TeensySdioDemo.ino), using `SD_FAT_TYPE = 3` (allowing for FAT16/FAT32/exFAT).
## KODAK 4 GB micro SD
### DMA SDIO Mode
|  size (bytes) |  write (KB/sec) |  read (KB/sec) |
|:-------------:|:---------------:|:--------------:|
|    512        |     252.87      |    2929.39     |
|    1024       |     492.54      |    4655.93     |
|    2048       |     982.25      |    7621.69     |
|    4096       |    1425.91      |    9921.22     |
|    8192       |    2249.03      |   12519.10     |
|    16384      |    3591.64      |   14409.88     |
|    32768      |    5801.69      |   15554.05     |

### FIFO Mode
|  size (bytes) |  write (KB/sec) |  read (KB/sec) |
|:-------------:|:---------------:|:--------------:|
|    512        |    12601.45     |    16578.28    |
|    1024       |    12575.42     |    16568.22    |
|    2048       |    9815.53      |    16612.16    |
|    4096       |    12639.61     |    16588.24    |
|    8192       |    12687.92     |    16580.28    |
|    16384      |    12636.87     |    16646.18    |
|    32768      |    12679.04     |    16608.64    |

## SanDisk Ultra 32 GB micro SD
### DMA SDIO Mode
|  size (bytes) |  write (KB/sec) |  read (KB/sec) |
|:-------------:|:---------------:|:--------------:|
|    512        |     559.18      |    2432.61     |
|    1024       |     819.58      |    3081.60     |
|    2048       |    1913.90      |    5670.38     |
|    4096       |    4246.62      |    9229.80     |
|    8192       |    6763.66      |   13413.27     |
|    16384      |    9689.88      |   17454.85     |
|    32768      |   12709.28      |   20208.35     |

### FIFO Mode
|  size (bytes) |  write (KB/sec) |  read (KB/sec) |
|:-------------:|:---------------:|:--------------:|
|    512        |     3462.76     |    22562.76    |
|    1024       |    19041.73     |    22853.01    |
|    2048       |    16099.52     |    22912.68    |
|    4096       |    18691.49     |    22940.82    |
|    8192       |    17465.46     |    22907.74    |
|    16384      |    17643.07     |    22913.93    |
|    32768      |    18781.59     |    22970.47    |

# ADC sampling
## Sampling without writing (no DMAMEM)
Simple performance test using pin 15 to sample the 3.3 V delivered by pin 18 to a 330 Ohm resistor that connects to ground.

|                 |   1 kHz   |   10 kHz  |   100 kHz  |   500 kHz   |
|:---------------:|:---------:|:---------:|:----------:|:-----------:|
|    1st buffer   |  4180 ms  | 414777 us |  46229 us  |   8192 us   | 
|    2nd buffer   |  4096 ms  | 409600 us |  40960 us  |   8192 us   |
|    3rd buffer   |  4096 ms  | 409600 us |  40960 us  |   8192 us   |
|    4th buffer   |  4096 ms  | 409600 us |  40960 us  |   8192 us   |
|    5th buffer   |  4096 ms  | 409600 us |  40960 us  |   8192 us   |

No difference in the performance here with DMAMEM included (numbers remain identical).

## Sampling while writing (no DMAMEM)

No signficiant changes from the above table, with the exception that the delay in writing the first buffer is further exagerated. In this case, the first buffer of 500 kHz sampling takes about 13000 us, rather than 8192 us. Every buffer afterward appears to take the proper 8192 us of time to fill. Likely cause is the time from initialization to actual start of the ADC, and thus, first buffer taking longer isn't a problem.

## MPU-9250
Connecting the interrupt pin of the MPU-9250 breakout board to pin 2 of the teensy and using an interrupt routine to check for whether new data is available has much better sampling performance than in the for loop. The interrupt fires every 5.005 ms (exactly 5004.969696 +/- 0.17 us, which corresponds to almost every 5.005 ms except when it takes 5.004 ms, usually about 1/30 times it takes it slightly less time than 5.005 ms). Using a loop results in the time between actually sampling to be about 5004.79 +/- 79.27 us. Interrupt routine is the way to go.

## ADS-1299

The EEG Sampling Chip

### Registers
  * Use test signal:
    * CHnSet: 0x65
    * CONFIG 2:
      * 0xD4 (fCLK/2^21)
      * 0xD5 (fCLK/2^20)
      * With these, CAL_AMP in CONFIG 2 will be set to 1, so it will be easier to see the rise and fall of each pin
    * MISC1: 0x00
  * Sampling Rate (SPS - Samples per second)
    * 0x96: 250 SPS
    * 0x95: 500 SPS
    * 0x94: 1000 SPS
    * 0x93: 2000 SPS
    * 0x92: 4000 SPS
    * 0x91: 8000 SPS
    * 0x90: 16000 SPS
  * BIAS pin
    * 0xEC: use BIAS
    * 0xE8: no BIAS
    * The BIAS pin attempts to reduce the common-mode interference by inputting a current into the body
  * Sampling
    * 0x60: active pin (PGA Gain of 24, no SRB2 connection and normal electrode input)
    * 0x81: deactivate pin

## ETag Lead Colors Notes
  * White - Reference
  * Red - Bias
  * Orange - Channel 1
  * Yellow - Channel 2
  * Green - Channel 3
  * Blue - Channel 4