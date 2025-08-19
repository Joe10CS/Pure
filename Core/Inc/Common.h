/*
 * Common.h
 *
  */

#ifndef INC_COMMON_H_
#define INC_COMMON_H_

#define MAX_NUMBER_OF_CARBONATION_STEPS (8)


typedef enum {
	eLevel_Low,
	eLevel_medium,
	eLevel_high,

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

#endif /* INC_COMMON_H_ */
