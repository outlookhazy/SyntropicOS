#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_RTC) || SYN_USE_RTC

/**
 * @file syn_rtc.c
 * @brief RTC driver implementation — port delegation + epoch math.
 */

#include "syn_rtc.h"
#include "../util/syn_assert.h"

/* ── Days in each month (non-leap year) ─────────────────────────────────── */

/** @brief Days per month (non-leap year). */
static const uint8_t s_days_in_month[12] = {
    31u, 28u, 31u, 30u, 31u, 30u, 31u, 31u, 30u, 31u, 30u, 31u
};

/**
 * @brief Test if a year is a leap year.
 * @param year  Gregorian year.
 * @return true if leap year.
 */
static bool rtc_is_leap(uint16_t year)
{
    return ((year % 4u == 0u) && (year % 100u != 0u || year % 400u == 0u));
}

/**
 * @brief Get days in a given month.
 * @param month  Month (1–12).
 * @param year   Gregorian year.
 * @return Day count.
 */
static uint8_t rtc_days_in_month(uint8_t month, uint16_t year)
{
    uint8_t d = s_days_in_month[(uint8_t)(month - 1u)];
    if (month == 2u && rtc_is_leap(year)) {
        d++;
    }
    return d;
}

bool syn_rtc_is_valid(const SYN_RTC_DateTime *dt)
{
    SYN_ASSERT(dt != NULL);
    if (dt->year  < 1970u)                                return false;
    if (dt->month < 1u || dt->month > 12u)                return false;
    if (dt->day   < 1u || dt->day > rtc_days_in_month(dt->month, dt->year))
                                                           return false;
    if (dt->hour   > 23u)                                 return false;
    if (dt->minute > 59u)                                 return false;
    if (dt->second > 59u)                                 return false;
    return true;
}

SYN_Status syn_rtc_init(void)
{
    return syn_port_rtc_init();
}

SYN_Status syn_rtc_get(SYN_RTC_DateTime *dt)
{
    SYN_ASSERT(dt != NULL);
    return syn_port_rtc_get(dt);
}

SYN_Status syn_rtc_set(const SYN_RTC_DateTime *dt)
{
    SYN_ASSERT(dt != NULL);
    if (!syn_rtc_is_valid(dt)) {
        return SYN_INVALID_PARAM;
    }
    return syn_port_rtc_set(dt);
}



uint32_t syn_rtc_to_epoch(const SYN_RTC_DateTime *dt)
{
    SYN_ASSERT(dt != NULL);

    /* Count days from 1970-01-01 to the start of dt->year */
    uint32_t days = 0u;
    uint16_t y;
    for (y = 1970u; y < dt->year; y++) {
        days += rtc_is_leap(y) ? 366u : 365u;
    }

    /* Add days for elapsed months in the current year */
    uint8_t m;
    for (m = 1u; m < dt->month; m++) {
        days += (uint32_t)rtc_days_in_month(m, dt->year);
    }

    /* Add elapsed days in the current month (day is 1-indexed) */
    days += (uint32_t)dt->day - 1u;

    return days      * 86400u
         + (uint32_t)dt->hour   * 3600u
         + (uint32_t)dt->minute * 60u
         + (uint32_t)dt->second;
}



void syn_rtc_from_epoch(uint32_t epoch, SYN_RTC_DateTime *dt)
{
    SYN_ASSERT(dt != NULL);

    /* Extract time-of-day */
    dt->second = (uint8_t)(epoch % 60u); epoch /= 60u;
    dt->minute = (uint8_t)(epoch % 60u); epoch /= 60u;
    dt->hour   = (uint8_t)(epoch % 24u); epoch /= 24u;

    /* epoch now = days since 1970-01-01 */
    uint32_t days = epoch;

    /* Find year */
    uint16_t year = 1970u;
    for (;;) {
        uint32_t days_in_year = rtc_is_leap(year) ? 366u : 365u;
        if (days < days_in_year) break;
        days -= days_in_year;
        year++;
    }
    dt->year = year;

    /* Find month */
    uint8_t month = 1u;
    for (;;) {
        uint32_t dim = (uint32_t)rtc_days_in_month(month, year);
        if (days < dim) break;
        days -= dim;
        month++;
    }
    dt->month = month;
    dt->day   = (uint8_t)(days + 1u);
}

#endif /* SYN_USE_RTC */
