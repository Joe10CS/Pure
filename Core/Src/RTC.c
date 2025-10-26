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

bool IsInFilterReplacementWarningPeriod(void)
{
    int16_t left = GetFilterDaysLeft();
    return (left <= FILTER_WARNING_DAYS && left > 0);
}
bool IsFilterExpired(void)
{
    return (GetFilterDaysLeft() == 0);
}

uint32_t ConvertDateToDays(uint16_t y, uint8_t m, uint8_t d)
{
    // Date → days since 0000-03-01 (valid for 2000–2099)
    if (m < 3) { y--; m += 12; }
    return 365UL * y + y/4 - y/100 + y/400 + (153UL * (m - 3) + 2) / 5 + d - 1;
}

// TODO Consider taking out this function it is probably needed only for testing
void SetDaysSinceLastFilterReplacement(uint32_t daysSinceReplacement)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    // Base = 1-Jan-2000 + daysSinceReplacement
    // We'll compute the simulated calendar date.
    uint16_t year = 2000;
    uint8_t month = 1;
    uint8_t day = 1;

    // Add daysSinceReplacement to the base date
    // (simple integer math, valid 2000–2099)
    uint32_t baseDays = ConvertDateToDays(2000, 1, 1) + daysSinceReplacement;
    uint32_t y = 2000;
    while (baseDays >= ConvertDateToDays(y + 1, 1, 1) - ConvertDateToDays(2000, 1, 1))
        y++;
    year = (uint16_t)y;

    // rough back-convert to month/day (lightweight)
    for (month = 1; month <= 12; month++)
    {
        uint32_t daysThisMonth = ConvertDateToDays(year, month + 1, 1) - ConvertDateToDays(year, month, 1);
        if (baseDays < daysThisMonth) break;
        baseDays -= daysThisMonth;
    }
    day = (uint8_t)(baseDays + 1);

    sTime.Hours = 0;
    sTime.Minutes = 0;
    sTime.Seconds = 0;
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

    sDate.Year  = (uint8_t)(year - 2000);
    sDate.Month = month;
    sDate.Date  = day;
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}

