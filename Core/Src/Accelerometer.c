/**
  ******************************************************************************
  * @file           : Accelerometer.c
  * @brief          : Accelerometer I/F and functions
   ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "lis2de12_reg.h"

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define TWOS_COMPLEMENT_UINT8_TO_DEC(v)     (((v)&(0x80))?(-((int8_t)((~(v))+1))):((int8_t)(v)))
#define ABS_TWOS_COMPLEMENT_UINT8_TO_DEC(v) (((v)&(0x80))?(((int8_t)((~(v))+1))):((int8_t)(v)))
#define ABS_VAL(v)                          (((v)<0)?(-(v)):(v))
#define ACCEL_FILTER_LEN  (100)
#define ACCEL_FILTER_GAIN (10)
#define ACCEL_FILTER_DATA(prev,raw) ((((prev)*(ACCEL_FILTER_LEN-ACCEL_FILTER_GAIN))+((raw)*ACCEL_FILTER_GAIN))/ACCEL_FILTER_LEN)
#define SLANT_ANGLE_THRESHOLD (7)
#define SINGLE_AXIS_SLANT (40)
#define TWO_AXES_SLANT (60)

// Actual time is threshold x 10mSecs  (max value 256)
#define SLANTED_THRESHOLD_TIME_COUNT (50)

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
extern SPI_HandleTypeDef hspi1;
uint8_t raw_g = 0;
int8_t gx = 0;
int8_t gy = 0;
int8_t gz = 0;
int8_t filtered_x = 0;
int8_t filtered_y = 0;
int8_t filtered_z = 0;

// counter for sequential slant
uint8_t count_slanted_plux_x = 0;
uint8_t count_slanted_minus_x = 0;
uint8_t count_slanted_plux_y = 0;
uint8_t count_slanted_minus_y = 0;
uint8_t count_slanted_plux_x_plus_y = 0;
uint8_t count_slanted_minus_x_plus_y = 0;
uint8_t count_slanted_plux_x_minus_y = 0;
uint8_t count_slanted_minus_x_minus_y = 0;

//#define NUM_RECORD (330)
//uint32_t rec_cnt = 0;
//int8_t rec_gx[NUM_RECORD];
//int8_t rec_gy[NUM_RECORD];
//int8_t rec_filt_gx[NUM_RECORD];


/* Private function prototypes -----------------------------------------------*/
void AccelerometerWriteRegister(uint8_t reg, uint8_t val);
void AccelerometerReadRegister(uint8_t reg, uint8_t *val);

bool IsSlantedFast();
bool gIsSlanted = false;
/* Private user code ---------------------------------------------------------*/

void AccelerometerInit(void)
{
	lis2de12_reg_t reg;

	// start the sensor
	reg.byte = 0;
	reg.ctrl_reg1.xen = PROPERTY_ENABLE;
	reg.ctrl_reg1.yen = PROPERTY_ENABLE;
	reg.ctrl_reg1.zen = PROPERTY_ENABLE;
	reg.ctrl_reg1.lpen = PROPERTY_ENABLE;
//	reg.ctrl_reg1.odr = (uint8_t)LIS2DE12_ODR_25Hz;
	reg.ctrl_reg1.odr = (uint8_t)LIS2DE12_ODR_100Hz;
	AccelerometerWriteRegister(LIS2DE12_CTRL_REG1, reg.byte);

}


bool IsSlanted() {
#ifndef DEBUG_NO_ACCELEROMETER
    uint8_t raw_g;

    // ----------------------------
    // 1. Read + Convert + Filter Z
    // ----------------------------
    AccelerometerReadRegister(LIS2DE12_OUT_Z_H, &raw_g);
    int8_t gz = TWOS_COMPLEMENT_UINT8_TO_DEC(raw_g);
    filtered_z = ACCEL_FILTER_DATA(filtered_z, gz);

    // ----------------------------
    // 2. Read + Convert + Filter X
    // ----------------------------
    AccelerometerReadRegister(LIS2DE12_OUT_X_H, &raw_g);
    int8_t gx = TWOS_COMPLEMENT_UINT8_TO_DEC(raw_g);
    filtered_x = ACCEL_FILTER_DATA(filtered_x, gx);

    // ----------------------------
    // 3. Read + Convert + Filter Y
    // ----------------------------
    AccelerometerReadRegister(LIS2DE12_OUT_Y_H, &raw_g);
    int8_t gy = TWOS_COMPLEMENT_UINT8_TO_DEC(raw_g);
    filtered_y = ACCEL_FILTER_DATA(filtered_y, gy);

    // ----------------------------------------------------------
    // 4. Upside-down / free-fall detection (must come before tilt)
    // ----------------------------------------------------------
    if (filtered_z >= 0) {
        gIsSlanted = true;
        return true;
    }

    // ----------------------------------------------------------
    // 5. Compute squares (integer-only, no overflow)
    // ----------------------------------------------------------
    int32_t x2 = filtered_x * filtered_x;
    int32_t y2 = filtered_y * filtered_y;
    int32_t z2 = filtered_z * filtered_z;

    int32_t mag2 = x2 + y2 + z2;

    // ----------------------------------------------------------
    // 6. Angle thresholds (45° enter, 35° exit)
    //     thresholds scaled by 1000
    // ----------------------------------------------------------
    const int32_t COS45_SQ = 500;   // 0.5 * 1000
    const int32_t COS35_SQ = 671;   // 0.671 * 1000

    if (!gIsSlanted) {
        // ENTER tilt if z² <= 0.5 * mag²
        if (z2 * 1000 <= COS45_SQ * mag2)
            gIsSlanted = true;
    }
    else {
        // EXIT tilt if z² >= 0.671 * mag²
        if (z2 * 1000 >= COS35_SQ * mag2)
            gIsSlanted = false;
    }

    return gIsSlanted;

#else// DEBUG_NO_ACCELEROMETER
    return false;
#endif

}




void AccelerometerWriteRegister(uint8_t reg, uint8_t val) {
	// reg |= 0x40; - needed if writing multiple data bytes and need to auto increment write address
	HAL_GPIO_WritePin(ACCEL_CS_GPIO_Port, ACCEL_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, &reg, 1, 1000);
	HAL_SPI_Transmit(&hspi1, &val, 1, 1000);
	HAL_GPIO_WritePin(ACCEL_CS_GPIO_Port, ACCEL_CS_Pin, GPIO_PIN_SET);
}

void AccelerometerReadRegister(uint8_t reg, uint8_t *val) {
	reg = 0x80 | reg; //set the read bit
	// reg |= 0x40; - needed if reading multiple data bytes and need to auto increment write address
	HAL_GPIO_WritePin(ACCEL_CS_GPIO_Port, ACCEL_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, &reg, 1, 1000);
	HAL_SPI_Receive(&hspi1, val, 1, 1000);
	HAL_GPIO_WritePin(ACCEL_CS_GPIO_Port, ACCEL_CS_Pin, GPIO_PIN_SET);
}

bool AccelerometerIsPresent(void) {
    uint8_t who_am_i = 0;

    // Read the WHO_AM_I register
    AccelerometerReadRegister(LIS2DE12_WHO_AM_I, &who_am_i);

    // Check if the returned value matches the expected device ID
    if (who_am_i == LIS2DE12_ID) {
        return true; // Device is present
    } else {
        return false; // Device is not present or not communicating correctly
    }
}

#if 0
// JIG stuff: added self test
//#include "lis2de12_reg.h" // include the header file for the accelerometer

#define SELF_TEST_MIN_THRESHOLD ... // Define the minimum threshold
#define SELF_TEST_MAX_THRESHOLD ... // Define the maximum threshold


#define LIS2DE12_SELF_TEST (1 << 6)


uint8_t RunSelfTest(void) {
    uint8_t self_test_pass = FALSE;
    int16_t self_test_data_x, self_test_data_y, self_test_data_z;
    uint8_t reg_value;

    // Enable self-test mode
    // Write to the appropriate register to enable self-test
    // For example, setting a bit in CTRL_REG4
    lis2de12_read_reg(&reg_ctx, LIS2DE12_CTRL_REG4, &reg_value, 1);
    reg_value |= LIS2DE12_SELF_TEST;
    lis2de12_write_reg(&reg_ctx, LIS2DE12_CTRL_REG4, &reg_value, 1);

    // Read accelerometer data
    // Read X, Y, Z data from output registers
    axis3bit16_t data_raw_acceleration;
    memset(data_raw_acceleration.u8bit, 0x00, 3*sizeof(int16_t));
    lis2de12_acceleration_raw_get(&reg_ctx, data_raw_acceleration.u8bit);
    self_test_data_x = data_raw_acceleration.i16bit[0];
    self_test_data_y = data_raw_acceleration.i16bit[1];
    self_test_data_z = data_raw_acceleration.i16bit[2];

    // Compare the read values with expected thresholds
    if ((self_test_data_x >= SELF_TEST_MIN_THRESHOLD) && (self_test_data_x <= SELF_TEST_MAX_THRESHOLD) &&
        (self_test_data_y >= SELF_TEST_MIN_THRESHOLD) && (self_test_data_y <= SELF_TEST_MAX_THRESHOLD) &&
        (self_test_data_z >= SELF_TEST_MIN_THRESHOLD) && (self_test_data_z <= SELF_TEST_MAX_THRESHOLD)) {
        self_test_pass = TRUE;
    }

    // Disable self-test mode and return to normal operation
    lis2de12_read_reg(&reg_ctx, LIS2DE12_CTRL_REG4, &reg_value, 1);
    reg_value &= ~LIS2DE12_SELF_TEST;
    lis2de12_write_reg(&reg_ctx, LIS2DE12_CTRL_REG4, &reg_value, 1);

    return self_test_pass;
}
#endif
