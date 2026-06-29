/**
 * @file syn_assert.h
 * @brief Configurable assertion macro for SyntropicOS.
 *
 * SYN_ASSERT(expr) evaluates @p expr and, if false, calls
 * syn_assert_failed() with the file name and line number. The user
 * can override syn_assert_failed() to implement custom behavior
 * (e.g., log to UART, blink an LED, enter debugger).
 *
 * Define SYN_DISABLE_ASSERT before including this header (or in
 * syn_config.h) to compile out all assertions for release builds.
 * @ingroup syn_core
 */

#ifndef SYN_ASSERT_H
#define SYN_ASSERT_H

#include "../common/syn_compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Called when an assertion fails.
 *
 * This function is declared weak so you can provide your own
 * implementation. The default (in syn_port_stubs.c) enters an
 * infinite loop.
 *
 * @param file  Source file name where the assertion failed.
 * @param line  Line number where the assertion failed.
 */
SYN_NORETURN void syn_assert_failed(const char *file, int line);

#ifndef SYN_DISABLE_ASSERT

#ifdef __CPPCHECK__
  /* Tell cppcheck that assertion failures halt execution */
  void abort(void);
  #define SYN_ASSERT(expr) do { if (!(expr)) { abort(); } } while(0)
#else
  /**
   * @brief Assert that @p expr is true. If false, call syn_assert_failed().
   */
  #define SYN_ASSERT(expr)                                 \
      do {                                                   \
          if (!(expr)) {                                     \
              syn_assert_failed(__FILE__, __LINE__);        \
          }                                                  \
      } while (0)
#endif

#else
  #define SYN_ASSERT(expr)   ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* SYN_ASSERT_H */
