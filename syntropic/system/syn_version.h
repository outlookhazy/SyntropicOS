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
 *          SYN_VERSION_MAJOR, SYN_VERSION_MINOR, SYN_VERSION_PATCH,
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

#ifndef SYN_VERSION_MAJOR
/** @brief Major version number. */
#define SYN_VERSION_MAJOR  0
#endif

#ifndef SYN_VERSION_MINOR
/** @brief Minor version number. */
#define SYN_VERSION_MINOR  1
#endif

#ifndef SYN_VERSION_PATCH
/** @brief Patch version number. */
#define SYN_VERSION_PATCH  0
#endif

/** Packed version: 0xMMmmpp (major.minor.patch). */
#define SYN_VERSION_CODE \
    (((uint32_t)SYN_VERSION_MAJOR << 16) | \
     ((uint32_t)SYN_VERSION_MINOR <<  8) | \
      (uint32_t)SYN_VERSION_PATCH)

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
    uint8_t      major;       /**< Major version number (breaking API changes) */
    uint8_t      minor;       /**< Minor version number (new backward-compatible features) */
    uint8_t      patch;       /**< Patch version number (backward-compatible bug fixes) */
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
        .major    = SYN_VERSION_MAJOR,
        .minor    = SYN_VERSION_MINOR,
        .patch    = SYN_VERSION_PATCH,
        .code     = SYN_VERSION_CODE,
        .date     = SYN_BUILD_DATE,
        .time     = SYN_BUILD_TIME,
        .git_hash = SYN_GIT_HASH,
        .app_name = SYN_APP_NAME,
    };
    return &v;
}

/**
 * @brief Check if running version is at least major.minor.patch.
 *
 * @param maj Required major version component.
 * @param min Required minor version component.
 * @param pat Required patch version component.
 * @return Non-zero if the current version meets or exceeds the requirements.
 */
static inline int syn_version_at_least(uint8_t maj, uint8_t min, uint8_t pat)
{
    uint32_t required = ((uint32_t)maj << 16) |
                        ((uint32_t)min << 8)  |
                         (uint32_t)pat;
    return SYN_VERSION_CODE >= required;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_VERSION_H */
