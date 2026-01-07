/*
 * RTC.c
 */
#include "RTC.h"


extern RTC_HandleTypeDef hrtc;

void RestartFilterTimer(void)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    // Set time to 00:00:00 and date to 1-Jan-2000 (arbitrary base)
    sTime.Hours = 0;
    sTime.Minutes = 0;
    sTime.Seconds = 0;
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

    sDate.WeekDay = RTC_WEEKDAY_SATURDAY;
    sDate.Month   = RTC_MONTH_JANUARY;
    sDate.Date    = 1;
    sDate.Year    = 0; // Year 2000
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}
uint16_t GetDaysSinceFilterReplacement(void)
{
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;

    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    uint16_t year  = 2000 + sDate.Year;
    uint16_t month = sDate.Month;
    uint16_t day   = sDate.Date;

    // Compute number of days since base date (1-Jan-2000)
    uint32_t days = ConvertDateToDays(year, month, day) - ConvertDateToDays(2000, 1, 1);

    return (uint16_t)days;
}
int16_t GetFilterDaysLeft(void)
{
    uint16_t elapsed = GetDaysSinceFilterReplacement();

    if (elapsed >= FILTER_LIFETIME_DAYS)
        return 0;
    return (FILTER_LIFETIME_DAYS - elapsed);
}

eFilterStatus GetFilterStatus(void)
{
    int16_t daysLeft = GetFilterDaysLeft();
    if (daysLeft == 0)
        return eFilterStatus_Expired;
    else if (daysLeft <= FILTER_WARNING_DAYS)
        return eFilterStatus_Warning;
    else
        return eFilterStatus_OK;
}

bool IsInFilterReplacementWarningPeriod(void)
{
    int16_t left = GetFilterDaysLeft();
    return (left <= FILTER_WARNING_DAYS && left > 0);
}
bool IsFilterExpired(void)
{
    return (GetFilterDaysLeft() == 0);
}
bool IsFilterTimeOK()
{
    return (GetFilterDaysLeft() > FILTER_WARNING_DAYS);
}

uint32_t ConvertDateToDays(uint16_t y, uint8_t m, uint8_t d)
{
    // Date → days since 0000-03-01 (valid for 2000–2099)
    if (m < 3) { y--; m += 12; }
    return 365UL * y + y/4 - y/100 + y/400 + (153UL * (m - 3) + 2) / 5 + d - 1;
}

void ForceFilterExpired()
{
	SetDaysSinceLastFilterReplacement(FILTER_LIFETIME_DAYS+1);
}

void SetDaysSinceLastFilterReplacement(uint32_t daysSinceReplacement)
{
    if (daysSinceReplacement > 200)
        daysSinceReplacement = 200;  // clamp for safety in tests

    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    // Year is always 2000
    uint16_t year = 2000;
    static const uint8_t daysInMonth[12] = {
        31, // Jan
        29, // Feb (2000 is leap year)
        31, // Mar
        30, // Apr
        31, // May
        30, // Jun
        31, // Jul
        31, // Aug
        30, // Sep
        31, // Oct
        30, // Nov
        31  // Dec
    };

    uint32_t remaining = daysSinceReplacement;
    uint8_t month = 1;
    uint8_t day   = 1;

    for (uint8_t i = 0; i < 12; i++)
    {
        uint8_t dim = daysInMonth[i];
        if (remaining < dim)
        {
            month = i + 1;
            day   = (uint8_t)(remaining + 1); // 0 → day 1
            break;
        }
        remaining -= dim;
    }

    // Set time
    sTime.Hours   = 0;
    sTime.Minutes = 0;
    sTime.Seconds = 0;
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

    // Set date (year offset from 2000)
    sDate.Year  = (uint8_t)(year - 2000);
    sDate.Month = month;
    sDate.Date  = day;

    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}

