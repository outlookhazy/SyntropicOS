/**
 * @file main.c
 * @brief STM32 Protothread Task Scheduler & CPU Profiler Integration Example.
 *
 * Demonstrates the SyntropicOS cooperative protothread scheduler (syn_sched)
 * working hand-in-hand with the CPU load profiler (syn_profiler) on STM32 targets:
 * - Creating protothread tasks with priorities via syn_task_create()
 * - Managing task execution in a cooperative scheduling loop via syn_sched_run()
 * - Profiling CPU execution time, peak run time, and CPU % per task via syn_profiler
 * - Outputting real-time CPU performance breakdown tables over STM32 UART
 */

/* ── Main Entry Point ───────────────────────────────────────────────────── */

extern int main_bare(void);
extern int main_sched(void);

int main(void)
{
#if defined(USE_BARE_LOOP)
    return main_bare();
#else
    return main_sched();
#endif
}
