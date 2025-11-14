/*
 * Common.h
 *
  */

#ifndef INC_COMMON_H_
#define INC_COMMON_H_


#define READY_STATE_TIMEOUT_MSECS (10000U)
#define FILTER_TO_CARBONATION_DELAY_MSECS (1000U)
#define MAX_NUMBER_OF_CARBONATION_STEPS (8)


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
void CheckLongPressButtonsPeriodic();
bool IsAnyKeyPressed();

extern uint32_t glb_safty_error_state;
#endif /* INC_COMMON_H_ */
