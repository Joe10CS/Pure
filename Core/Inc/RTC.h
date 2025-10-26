#ifndef __RTC_H
#define __RTC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define FILTER_LIFETIME_DAYS   (180)
#define FILTER_WARNING_DAYS     (14)

/* ============================================================
 *  RTC Interface - Minimal version for Filter Timer tracking
 *  ------------------------------------------------------------
 *  - Resets RTC when filter is replaced
 *  - Computes days since replacement and days left
 *  - Warning when filter is near expiration
 * ============================================================ */

/**
 * @brief Resets the RTC date/time to 1-Jan-2000 00:00:00.
 *        Call this when a new filter is installed (after rinsing).
 */
void RestartFilterTimer(void);

/**
 * @brief Returns number of full days elapsed since last filter replacement.
 */
uint16_t GetDaysSinceFilterReplacement(void);

/**
 * @brief Returns number of full days remaining before filter expiration.
 * @return Remaining days (0 if expired).
 */
int16_t GetFilterDaysLeft(void);

/**
 * @brief Indicates whether current day is within warning period.
 * @return true if within warning period, false otherwise.
 */
bool IsInFilterReplacementWarningPeriod(void);

bool IsFilterExpired(void);

uint32_t ConvertDateToDays(uint16_t y, uint8_t m, uint8_t d);

// TODO debug method - remove
void SetDaysSinceLastFilterReplacement(uint32_t daysSinceReplacement);
#ifdef __cplusplus
}
#endif

#endif /* __RTC_H */
