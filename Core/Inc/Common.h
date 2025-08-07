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
	eCycle_on,
	eCycle_off,

	eCycle_number_of_cycles
}eCarbonationCycle;

#endif /* INC_COMMON_H_ */
