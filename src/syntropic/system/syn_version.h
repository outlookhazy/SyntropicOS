/**
 * @file syn_version.h
 * @brief Build version and metadata — compile-time constants.
 *
 * Provides version numbers, build date/time, and optional git hash.
 * Users define these in their build system or syn_config.h.
 *
 * @par Usage
 * @code
 *   printf("SyntropicOS v%d.%d.%d built %s\n",
 *          SYN_VERSION_YEAR, SYN_VERSION_MONTH, SYN_VERSION_RELEASE,
 *          SYN_BUILD_DATE);
 *
 *   // Or use the struct:
 *   const SYN_Version *v = syn_version();
 *   printf("%s\n", v->string);
 * @endcode
 * @ingroup syn_system
 */

#ifndef SYN_VERSION_H
#define SYN_VERSION_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Defaults (override in build system or config) ──────────────────────── */

#ifndef SYN_VERSION_YEAR
/** @brief Release year (CalVer). */
#define SYN_VERSION_YEAR   2026
#endif

#ifndef SYN_VERSION_MONTH
/** @brief Release month (CalVer). */
#define SYN_VERSION_MONTH  7
#endif

#ifndef SYN_VERSION_RELEASE
/** @brief Release number within the month (CalVer). */
#define SYN_VERSION_RELEASE 0
#endif

/** Packed version: 0xYYYYMMRR (year.month.release). */
#define SYN_VERSION_CODE \
    (((uint32_t)SYN_VERSION_YEAR  << 16) | \
     ((uint32_t)SYN_VERSION_MONTH <<  8) | \
      (uint32_t)SYN_VERSION_RELEASE)

/** Build date string (e.g., "Jun 27 2026"). */
#ifndef SYN_BUILD_DATE
#define SYN_BUILD_DATE  __DATE__
#endif

/** Build time string (e.g., "14:30:00"). */
#ifndef SYN_BUILD_TIME
#define SYN_BUILD_TIME  __TIME__
#endif

/** Git hash (short). Override in build system: -DSYN_GIT_HASH=\"abc1234\" */
#ifndef SYN_GIT_HASH
#define SYN_GIT_HASH  "unknown"
#endif

/** Application name. */
#ifndef SYN_APP_NAME
#define SYN_APP_NAME  "SyntropicOS"
#endif

/* ── Version struct ─────────────────────────────────────────────────────── */

/** @brief Compile-time version information. */
typedef struct {
    uint16_t     year;        /**< Release year (CalVer)                 */
    uint8_t      month;       /**< Release month (CalVer)                */
    uint8_t      release;     /**< Release number within the month       */
    uint32_t     code;        /**< Packed SYN_VERSION_CODE               */
    const char  *date;        /**< Build date                             */
    const char  *time;        /**< Build time                             */
    const char  *git_hash;    /**< Git short hash                         */
    const char  *app_name;    /**< Application name                       */
} SYN_Version;

/**
 * @brief Get the compile-time version info.
 * @return Pointer to static version struct.
 */
static inline const SYN_Version *syn_version(void)
{
    static const SYN_Version v = {
        .year     = SYN_VERSION_YEAR,
        .month    = SYN_VERSION_MONTH,
        .release  = SYN_VERSION_RELEASE,
        .code     = SYN_VERSION_CODE,
        .date     = SYN_BUILD_DATE,
        .time     = SYN_BUILD_TIME,
        .git_hash = SYN_GIT_HASH,
        .app_name = SYN_APP_NAME,
    };
    return &v;
}

/**
 * @brief Check if running version is at least year.month.release.
 *
 * @param year Required year component.
 * @param month Required month component.
 * @param rel Required release component.
 * @return Non-zero if the current version meets or exceeds the requirements.
 */
static inline int syn_version_at_least(uint16_t year, uint8_t month, uint8_t rel)
{
    uint32_t required = ((uint32_t)year  << 16) |
                        ((uint32_t)month << 8)  |
                         (uint32_t)rel;
    return SYN_VERSION_CODE >= required;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_VERSION_H */
