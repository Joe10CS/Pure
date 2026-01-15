/*
 * Common.h
 *
  */

#ifndef INC_COMMON_H_
#define INC_COMMON_H_


// #define READY_STATE_TIMEOUT_MSECS (10000U) 10 sec
#define READY_STATE_TIMEOUT_MSECS (600000U) // 10 minutes
#define FILTER_TO_CARBONATION_DELAY_MSECS (1000U)
#define MAX_NUMBER_OF_CARBONATION_STEPS (8)

// Time window in mSec from power up to enter OOTB
#define RESET_TO_OOTB_MSEC (3000)

#define FILTER_LIFETIME_DAYS   (90) // expiration in days
#define FILTER_WARNING_DAYS     (9) // last days before expiration

#define MAX_FILTERING_COUNTER (300) // 300 filtering cycles

#define CO2_LIFETIME_MSECS   (140000) // was 170000
#define CO2_MAX_SINGLE_PULSE_MSECS   (15000)

#define MINIMUM_NO_WATER_IN_PUMP_DETECTION_TIME_MSEC (10000)
#define NO_WATER_IN_PUMP_DETECTION_ADC_THRESHOLD (755) // 200mA

// ADC value above which the UV is considered on
#define UV_MIN_ADC_THRESHOLD (197) // 20mA


#define SOLENOID_PUMP_HW_FDBK_GRACE_TIME_MSECS (10)
#define SOLENOID_PUMP_HW_FDBK_MAX_COUNT (3)


#define IS_FILTER_BUTTON_PRESSED() (HAL_GPIO_ReadPin(BTN3_GPIO_Port, BTN3_Pin) == GPIO_PIN_RESET)
#define IS_CARB_LEVEL_BUTTON_PRESSED() (HAL_GPIO_ReadPin(BTN2_GPIO_Port, BTN2_Pin) == GPIO_PIN_RESET)


typedef enum {
	eLevel_off = 0,
	eLevel_Low = 1,
	eLevel_medium = 2,
	eLevel_high = 3,

	eLevel_number_of_levels
}eCarbonationLevel;

typedef enum {
	eBottle_1_Litter,
	eBottle_0_5_Litter
}eBottleSize;

typedef enum {
	eCycle_on,
	eCycle_off,

	eCycle_number_of_cycles
}eCarbonationCycle;

typedef enum {
	eDone_OK = 0,
}eDoneResults;

void SendDoneMessage(eDoneResults result);
void CheckButtonsPressPeriodic();
bool IsAnyKeyPressed();

extern uint32_t glb_safty_error_state;
#endif /* INC_COMMON_H_ */
