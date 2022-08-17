#include <MPU9250.h>
#include <unity.h>

MPU9250 IMU(MPU9250_ADDRESS, I2Cport, I2Cclock);

void test_IMU_setup() {
    Wire.begin();
    delay(2000);

    byte IMU_ID = IMU.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
    byte MAG_ID = IMU.readByte(AK8963_ADDRESS, WHO_AM_I_AK8963);

    TEST_ASSERT_EQUAL_UINT8(0x71, IMU_ID);
    TEST_ASSERT_EQUAL_UINT8(0x48, MAG_ID);

    IMU.MPU9250SelfTest(IMU.selfTest);
    IMU.calibrateMPU9250(IMU.gyroBias, IMU.accelBias);
    
    //Check deviation from factory trim values of the MPU9250
    float max_deviation_from_factory_trim = 14.0;
    TEST_ASSERT_LESS_OR_EQUAL(max_deviation_from_factory_trim, abs(IMU.selfTest[0]));
    TEST_ASSERT_LESS_OR_EQUAL(max_deviation_from_factory_trim, abs(IMU.selfTest[1]));
    TEST_ASSERT_LESS_OR_EQUAL(max_deviation_from_factory_trim, abs(IMU.selfTest[2]));
    TEST_ASSERT_LESS_OR_EQUAL(max_deviation_from_factory_trim, abs(IMU.selfTest[3]));
    TEST_ASSERT_LESS_OR_EQUAL(max_deviation_from_factory_trim, abs(IMU.selfTest[4]));
    TEST_ASSERT_LESS_OR_EQUAL(max_deviation_from_factory_trim, abs(IMU.selfTest[5]));

    IMU.initAK8963(IMU.factoryMagCalibration); // no test for this

    Wire.end();
}
