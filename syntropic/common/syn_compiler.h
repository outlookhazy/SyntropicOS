/**
 * @file syn_compiler.h
 * @brief Compiler-portable macros for SyntropicOS.
 *
 * Abstracts compiler-specific attributes and intrinsics so the rest of the
 * library (and user code) can use a single set of macros across GCC, Clang,
 * ARMCC (Keil), and IAR.
 * @ingroup syn_core
 */

#ifndef SYN_COMPILER_H
#define SYN_COMPILER_H

#ifdef __cplusplus
extern "C" {
#endif

/* ── Compiler detection ─────────────────────────────────────────────────── */

#if defined(__GNUC__) || defined(__clang__)
  #define SYN_COMPILER_GCC_LIKE  1
#elif defined(__ARMCC_VERSION)
  #define SYN_COMPILER_ARMCC     1
#elif defined(__IAR_SYSTEMS_ICC__)
  #define SYN_COMPILER_IAR       1
#else
  /** @brief Compiler could not be identified. */
  #define SYN_COMPILER_UNKNOWN   1
#endif

/* ── SYN_WEAK ──────────────────────────────────────────────────────────── */
/** Mark a function as a weak symbol so the user can override it. */
#if defined(SYN_COMPILER_GCC_LIKE)
  #define SYN_WEAK   __attribute__((weak))
#elif defined(SYN_COMPILER_ARMCC)
  #define SYN_WEAK   __weak
#elif defined(SYN_COMPILER_IAR)
  #define SYN_WEAK   __weak
#else
  #define SYN_WEAK
#endif

/* ── SYN_PACKED ────────────────────────────────────────────────────────── */
/** Pack a struct (place after the closing brace, before the semicolon). */
#if defined(SYN_COMPILER_GCC_LIKE)
  #define SYN_PACKED   __attribute__((packed))
#elif defined(SYN_COMPILER_ARMCC)
  #define SYN_PACKED   __attribute__((packed))
#elif defined(SYN_COMPILER_IAR)
  #define SYN_PACKED   /* IAR uses #pragma pack — see SYN_PRAGMA_PACK */
#else
  #define SYN_PACKED
#endif

/* ── SYN_INLINE ────────────────────────────────────────────────────────── */
/** Portable always-inline hint. */
#if defined(SYN_COMPILER_GCC_LIKE)
  #define SYN_INLINE   static inline __attribute__((always_inline))
#elif defined(SYN_COMPILER_ARMCC)
  #define SYN_INLINE   static __forceinline
#elif defined(SYN_COMPILER_IAR)
  #define SYN_INLINE   _Pragma("inline=forced") static inline
#else
  #define SYN_INLINE   static inline
#endif

/* ── SYN_UNUSED ────────────────────────────────────────────────────────── */
/** Suppress "unused variable/parameter" warnings. */
#if defined(SYN_COMPILER_GCC_LIKE)
  #define SYN_UNUSED   __attribute__((unused))
#else
  #define SYN_UNUSED
#endif

/* ── SYN_NORETURN ──────────────────────────────────────────────────────── */
/** Mark a function that never returns (e.g. fault handlers). */
#if defined(SYN_COMPILER_GCC_LIKE)
  #define SYN_NORETURN   __attribute__((noreturn))
#elif defined(SYN_COMPILER_ARMCC)
  #define SYN_NORETURN   __attribute__((noreturn))
#elif defined(SYN_COMPILER_IAR)
  #define SYN_NORETURN   __noreturn
#else
  #define SYN_NORETURN
#endif

/* ── SYN_STATIC_ASSERT ────────────────────────────────────────────────── */
/** Compile-time assertion. Uses C11 _Static_assert when available. */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
  #define SYN_STATIC_ASSERT(expr, msg)   _Static_assert(expr, msg)
#else
  /* C99 fallback: negative-size array trick */
  #define SYN_STATIC_ASSERT_JOIN2(a, b) a##b
  /** @brief Token-pasting helper for SYN_STATIC_ASSERT. */
  #define SYN_STATIC_ASSERT_JOIN(a, b)  SYN_STATIC_ASSERT_JOIN2(a, b)
  /** @brief Compile-time assertion (C99 fallback). */
  #define SYN_STATIC_ASSERT(expr, msg) \
      typedef char SYN_STATIC_ASSERT_JOIN(syn_static_assert_, __LINE__)[(expr) ? 1 : -1]
#endif

/* ── SYN_SECTION ───────────────────────────────────────────────────────── */
/** Place a symbol in a named linker section. */
#if defined(SYN_COMPILER_GCC_LIKE)
  #define SYN_SECTION(name)   __attribute__((section(name)))
#elif defined(SYN_COMPILER_ARMCC)
  #define SYN_SECTION(name)   __attribute__((section(name)))
#elif defined(SYN_COMPILER_IAR)
  #define SYN_SECTION(name)   @ name
#else
  #define SYN_SECTION(name)
#endif

/* ── SYN_ALIGN ─────────────────────────────────────────────────────────── */
/** Align a variable to N bytes. */
#if defined(SYN_COMPILER_GCC_LIKE)
  #define SYN_ALIGN(n)   __attribute__((aligned(n)))
#elif defined(SYN_COMPILER_ARMCC)
  #define SYN_ALIGN(n)   __attribute__((aligned(n)))
#elif defined(SYN_COMPILER_IAR)
  #define SYN_ALIGN(n)   /* IAR uses #pragma data_alignment */
#else
  #define SYN_ALIGN(n)
#endif

/* ── SYN_FALLTHROUGH ───────────────────────────────────────────────────── */
/** Suppress -Wimplicit-fallthrough warnings for intentional fallthrough. */
#if defined(SYN_COMPILER_GCC_LIKE)
  #if __GNUC__ >= 7
    #define SYN_FALLTHROUGH   __attribute__((fallthrough))
  #else
    #define SYN_FALLTHROUGH   /* fall through */
  #endif
#else
  #define SYN_FALLTHROUGH
#endif

#ifdef __cplusplus
}
#endif

#endif /* SYN_COMPILER_H */
