/**
 * @file unity_config.h
 * @brief Host incremental unit-runner configuration for file-aware test filtering.
 */

#ifndef SYN_UNITY_CONFIG_H
#define SYN_UNITY_CONFIG_H

#ifdef SYN_HOST_UNIT_RUNNER
void syn_unit_run_test(void (*func)(void), const char *func_name, const char *file_name,
                       int line_num);
#define RUN_TEST(func) syn_unit_run_test((func), #func, __FILE__, (int)(__LINE__))
#endif

#endif /* SYN_UNITY_CONFIG_H */
