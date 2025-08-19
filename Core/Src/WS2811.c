// WS2811.c
#include "main.h"
#include "WS2811.h"
#include <string.h>

extern TIM_HandleTypeDef  htim1;          // TIM1 (PA11 = CH4)
extern DMA_HandleTypeDef  hdma_tim1_up;   // TIM1_UP DMA

#define WS_TICKS_PER_BIT   80
#define WS_TIM_ARR         (WS_TICKS_PER_BIT - 1)       // 79
#define WS_CCR_0           20                           // ~0.3125 us
#define WS_CCR_1           55                           // ~0.8594 us (T1L ~0.391 us)

static volatile bool     s_ws_busy    = false;
static uint16_t          s_bits_total = 0;
static uint16_t          s_bits_sent  = 0;
static uint16_t          s_expected_tc = 0;

// One halfword per WS bit; big enough for full chain
static uint16_t s_ccr_stream[24 * NUMBER_OF_LEDS];

// Forward decls
static inline uint8_t pct_to_byte(uint8_t p);
static void encode_first_n_channels_percent(uint8_t *percent_grb, uint16_t channels);
static void encode_first_n_channels_raw    ( uint8_t *raw_grb,     uint16_t channels);

// Optional: your init registers DMA callbacks somewhere (once)
static void WS_TIM1_DMA_Complete(DMA_HandleTypeDef *hdma);
static void WS_TIM1_DMA_Error   (DMA_HandleTypeDef *hdma);

void WS_InitLeds(void)
{
    __HAL_TIM_SET_AUTORELOAD(&htim1, WS_TIM_ARR);
    __HAL_TIM_DISABLE_OCxPRELOAD(&htim1, TIM_CHANNEL_4);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);

    // Register DMA callbacks (required on G0 HAL)
    HAL_DMA_RegisterCallback(&hdma_tim1_up, HAL_DMA_XFER_CPLT_CB_ID,  WS_TIM1_DMA_Complete);
    HAL_DMA_RegisterCallback(&hdma_tim1_up, HAL_DMA_XFER_ERROR_CB_ID, WS_TIM1_DMA_Error);

    s_ws_busy = false;
}

bool WS_IsBusy(void) { return s_ws_busy; }
#if 0
// ---- Percent API (0..100) ----
HAL_StatusTypeDef WS_SetLeds(uint8_t *leds_percent, uint16_t num_to_set_channels)
{
    if (s_ws_busy)                    return HAL_BUSY;
    if (!leds_percent)                return HAL_ERROR;
    if (num_to_set_channels == 0)     return HAL_ERROR;
    if (num_to_set_channels > NUMBER_OF_LEDS) num_to_set_channels = NUMBER_OF_LEDS;
    if ((num_to_set_channels % 3u) != 0u)     return HAL_ERROR; // must send whole devices

    encode_first_n_channels_percent(leds_percent, num_to_set_channels);
    s_bits_total   = (uint16_t)(8u * num_to_set_channels);
    s_bits_sent    = 0;
    s_expected_tc  = (s_bits_total > 0u) ? (s_bits_total - 1u) : 0u;

    // --- race-proof start ---
    TIM1->CCR4 = s_ccr_stream[0];                  // preload first bit
    s_bits_sent = 1;

    // Clear lingering DMA flags, then arm for remaining bits
    __HAL_DMA_CLEAR_FLAG(&hdma_tim1_up, DMA_FLAG_TC4 | DMA_FLAG_HT4 | DMA_FLAG_TE4 | DMA_FLAG_GI4);
    if (s_expected_tc > 0u) {
        if (HAL_DMA_Start_IT(&hdma_tim1_up,
                             (uint32_t)&s_ccr_stream[1],
                             (uint32_t)&TIM1->CCR4,
                             s_expected_tc) != HAL_OK) {
            return HAL_ERROR;
        }
        __HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_UPDATE); // one write each 1.25us
    }

    __HAL_TIM_SET_COUNTER(&htim1, 0);
    if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4) != HAL_OK) {
        if (s_expected_tc > 0u) {
            __HAL_TIM_DISABLE_DMA(&htim1, TIM_DMA_UPDATE);
            HAL_DMA_Abort(&hdma_tim1_up);
        }
        return HAL_ERROR;
    }

    s_ws_busy = true;
    return HAL_OK;
}

// ---- Raw API (0..255) ----
HAL_StatusTypeDef WS_SetLedsRaw(uint8_t *grb_bytes, uint16_t num_to_set_channels)
{
    if (s_ws_busy)                    return HAL_BUSY;
    if (!grb_bytes)                   return HAL_ERROR;
    if (num_to_set_channels == 0)     return HAL_ERROR;
    if (num_to_set_channels > NUMBER_OF_LEDS) num_to_set_channels = NUMBER_OF_LEDS;
    if ((num_to_set_channels % 3u) != 0u)     return HAL_ERROR; // must send whole devices

    encode_first_n_channels_raw(grb_bytes, num_to_set_channels);
    s_bits_total   = (uint16_t)(8u * num_to_set_channels);
    s_bits_sent    = 0;
    s_expected_tc  = (s_bits_total > 0u) ? (s_bits_total - 1u) : 0u;

    // --- same start sequence ---
    TIM1->CCR4 = s_ccr_stream[0];
    s_bits_sent = 1;

    __HAL_DMA_CLEAR_FLAG(&hdma_tim1_up, DMA_FLAG_TC4 | DMA_FLAG_HT4 | DMA_FLAG_TE4 | DMA_FLAG_GI4);
    if (s_expected_tc > 0u) {
        if (HAL_DMA_Start_IT(&hdma_tim1_up,
                             (uint32_t)&s_ccr_stream[1],
                             (uint32_t)&TIM1->CCR4,
                             s_expected_tc) != HAL_OK) {
            return HAL_ERROR;
        }
        __HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_UPDATE);
    }

    __HAL_TIM_SET_COUNTER(&htim1, 0);
    if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4) != HAL_OK) {
        if (s_expected_tc > 0u) {
            __HAL_TIM_DISABLE_DMA(&htim1, TIM_DMA_UPDATE);
            HAL_DMA_Abort(&hdma_tim1_up);
        }
        return HAL_ERROR;
    }

    s_ws_busy = true;
    return HAL_OK;
}
#endif
static inline uint8_t pct_to_byte(uint8_t p)
{
    if (p > 100) p = 100;
    return (uint8_t)((p * 255u + 50u) / 100u);
}

static void encode_first_n_channels_raw(uint8_t *raw_grb, uint16_t channels)
{
    uint16_t *p = s_ccr_stream;
    for (uint16_t i = 0; i < channels; ++i) {
        uint8_t v = raw_grb[i];                     // 0..255, MSB first
        for (int b = 7; b >= 0; --b) *p++ = ((v >> b) & 1) ? WS_CCR_1 : WS_CCR_0;
    }
}

static void encode_first_n_channels_percent(uint8_t *percent_grb, uint16_t channels)
{
    uint16_t *p = s_ccr_stream;
    for (uint16_t i = 0; i < channels; ++i) {
        uint8_t v = pct_to_byte(percent_grb[i]);    // 0..100% -> 0..255
        for (int b = 7; b >= 0; --b) *p++ = ((v >> b) & 1) ? WS_CCR_1 : WS_CCR_0;
    }
}

static void ws_start_frame_common(void)
{
    // --- HARDENED START SEQUENCE ---
    // 0) Make sure PWM stopped and UDE off so line idles LOW
    __HAL_TIM_DISABLE_DMA(&htim1, TIM_DMA_UPDATE);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);

    // 1) Clear any pending DMA flags on our channel
    __HAL_DMA_CLEAR_FLAG(&hdma_tim1_up, DMA_FLAG_TC4 | DMA_FLAG_HT4 | DMA_FLAG_TE4 | DMA_FLAG_GI4);

    // 2) Clear TIM1 update flag (THIS prevents the immediate DMA write)
    __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);

    // 3) Preload first bit and reset counter
    TIM1->CCR4 = s_ccr_stream[0];
    __HAL_TIM_SET_AUTORELOAD(&htim1, WS_TIM_ARR);
    __HAL_TIM_SET_COUNTER(&htim1, 0);

    // 4) Arm DMA for the remaining bits (doesn't run yet)
    if (s_expected_tc > 0u) {
        HAL_DMA_Start_IT(&hdma_tim1_up,
                         (uint32_t)&s_ccr_stream[1],
                         (uint32_t)&TIM1->CCR4,
                         s_expected_tc);
    }

    // 5) Start PWM (period #0 uses CCR[0])
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);

    // 6) Ensure UIF is still clear, then enable UDE so the FIRST DMA happens
    //    at the END of the first period, not immediately.
    __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);
    if (s_expected_tc > 0u) __HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_UPDATE);

    s_ws_busy = true;
}

HAL_StatusTypeDef WS_SetLedsRaw(uint8_t *grb_bytes, uint16_t num_to_set_channels)
{
    if (s_ws_busy)                    return HAL_BUSY;
    if (!grb_bytes)                   return HAL_ERROR;
    if (num_to_set_channels == 0)     return HAL_ERROR;
    if (num_to_set_channels > NUMBER_OF_LEDS) num_to_set_channels = NUMBER_OF_LEDS;
    if ((num_to_set_channels % 3u) != 0u)     return HAL_ERROR;    // must be whole devices

    // Encode EXACTLY num_to_set_channels bytes (GRB, MSB-first)
    encode_first_n_channels_raw(grb_bytes, num_to_set_channels);

    // We transmit EXACTLY num_to_set_channels * 8 bits (your requirement)
    s_bits_total  = (uint16_t)(8u * num_to_set_channels);
    s_expected_tc = (s_bits_total > 0u) ? (s_bits_total - 1u) : 0u;  // remaining after first bit

    // Start with the hardened sequence
    ws_start_frame_common();
    return HAL_OK;
}

HAL_StatusTypeDef WS_SetLeds(uint8_t *leds_percent, uint16_t num_to_set_channels)
{
    if (s_ws_busy)                    return HAL_BUSY;
    if (!leds_percent)                return HAL_ERROR;
    if (num_to_set_channels == 0)     return HAL_ERROR;
    if (num_to_set_channels > NUMBER_OF_LEDS) num_to_set_channels = NUMBER_OF_LEDS;
    if ((num_to_set_channels % 3u) != 0u)     return HAL_ERROR;

    encode_first_n_channels_percent(leds_percent, num_to_set_channels);
    s_bits_total  = (uint16_t)(8u * num_to_set_channels);
    s_expected_tc = (s_bits_total > 0u) ? (s_bits_total - 1u) : 0u;

    ws_start_frame_common();
    return HAL_OK;
}
// ---- DMA callbacks ----
static void WS_TIM1_DMA_Complete(DMA_HandleTypeDef *hdma)
{
    if (hdma == &hdma_tim1_up) {
        __HAL_TIM_DISABLE_DMA(&htim1, TIM_DMA_UPDATE);
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);     // PA11 idles LOW (reset/latch)
        s_ws_busy = false;                           // 10 ms cadence >> 280 us
    }
}

static void WS_TIM1_DMA_Error(DMA_HandleTypeDef *hdma)
{
    if (hdma == &hdma_tim1_up) {
        __HAL_TIM_DISABLE_DMA(&htim1, TIM_DMA_UPDATE);
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);
        s_ws_busy = false;
        // optional: inspect HAL_DMA_GetError(hdma)
    }
}
#if 0
// ---- Encoders ----
static inline uint8_t pct_to_byte(uint8_t p)
{
    if (p > 100) p = 100;
    return (uint8_t)((p * 255u + 50u) / 100u);
}

// MSB-first, GRB order, first N channels
static void encode_first_n_channels_percent(uint8_t *percent_grb, uint16_t channels)
{
    uint16_t *p = s_ccr_stream;
    for (uint16_t i = 0; i < channels; ++i) {
        uint8_t v = pct_to_byte(percent_grb[i]);
        for (int b = 7; b >= 0; --b) {
            *p++ = ((v >> b) & 1) ? WS_CCR_1 : WS_CCR_0;
        }
    }
}

static void encode_first_n_channels_raw(uint8_t *raw_grb, uint16_t channels)
{
    uint16_t *p = s_ccr_stream;
    for (uint16_t i = 0; i < channels; ++i) {
        uint8_t v = raw_grb[i];               // already 0..255
        for (int b = 7; b >= 0; --b) {
            *p++ = ((v >> b) & 1) ? WS_CCR_1 : WS_CCR_0;
        }
    }
}
#endif
