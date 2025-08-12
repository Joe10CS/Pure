/*
 * FilterRTC.c
 *
 *  Created on: Aug 12, 2025
 *      Author: yossi
 */
#include "main.h"

// ---- filter timer API ----
#define DUE_SECONDS  (15552000u) // 180 days

// ---- backup helpers ----
#define BKP_MAGIC_REG   RTC_BKP_DR0
#define BKP_EPOCH_REG   RTC_BKP_DR1
#define BKP_MAGIC_VAL   (0xA5F1U)

extern RTC_HandleTypeDef hrtc;

static inline void BKP_Write(uint32_t reg, uint32_t val) {
    HAL_RTCEx_BKUPWrite(&hrtc, reg, val);
}
static inline uint32_t BKP_Read(uint32_t reg) {
    return HAL_RTCEx_BKUPRead(&hrtc, reg);
}

// ---- get epoch from RTC calendar (polling; no NVIC needed) ----
static uint32_t rtc_to_epoch(const RTC_DateTypeDef *d, const RTC_TimeTypeDef *t)
{
    int y = 2000 + d->Year;   // HAL stores 0..99
    int m = d->Month;         // 1..12
    int day = d->Date;        // 1..31
    static const int mdays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

    // days since 1970-01-01 (works fine for 2000..2099 range)
    int days = 0;
    for (int yy = 1970; yy < y; ++yy)
        days += 365 + ((yy%4==0) && (yy%100!=0 || yy%400==0));

    int ly = ((y%4==0) && (y%100!=0 || y%400==0));
    for (int mm = 1; mm < m; ++mm) {
        days += mdays[mm-1];
        if (mm == 2 && ly) days += 1;
    }
    days += (day - 1);

    return (uint32_t)days * 86400u + t->Hours * 3600u + t->Minutes * 60u + t->Seconds;
}

uint32_t RTC_GetEpoch(void)
{
    RTC_TimeTypeDef t; RTC_DateTypeDef d;
    HAL_RTC_GetTime(&hrtc, &t, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &d, RTC_FORMAT_BIN); // must be called after GetTime
    return rtc_to_epoch(&d, &t);
}


void FilterRTCTimer_Init(void)
{
    if (BKP_Read(BKP_MAGIC_REG) != BKP_MAGIC_VAL) {
        BKP_Write(BKP_MAGIC_REG, BKP_MAGIC_VAL);
        uint32_t now = RTC_GetEpoch();  // starts from current RTC
        BKP_Write(BKP_EPOCH_REG, now);
    }
}

void FilterRTC_Replaced_StartTimer(void)
{
    uint32_t now = RTC_GetEpoch();
    BKP_Write(BKP_EPOCH_REG, now);
}

bool FilterRTC_IsDue(void)
{
    uint32_t last = BKP_Read(BKP_EPOCH_REG);
    uint32_t now  = RTC_GetEpoch();
    return (now - last) >= DUE_SECONDS;
}

uint32_t FilterRTC_SecondsElapsed(void)
{
    uint32_t last = BKP_Read(BKP_EPOCH_REG);
    uint32_t now  = RTC_GetEpoch();
    return (now - last);
}

