

# File syn\_assert.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_assert.h**](syn__assert_8h.md)

[Go to the documentation of this file](syn__assert_8h.md)


```C++

#ifndef SYN_ASSERT_H
#define SYN_ASSERT_H

#include "../common/syn_compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

SYN_NORETURN void syn_assert_failed(const char *file, int line);

#ifndef SYN_DISABLE_ASSERT

#ifdef __CPPCHECK__
  /* Tell cppcheck that assertion failures halt execution */
  void abort(void);
  #define SYN_ASSERT(expr) do { if (!(expr)) { abort(); } } while(0)
#else
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
```


