

# File syn\_motor\_output.h

[**File List**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_motor\_output.h**](syn__motor__output_8h.md)

[Go to the documentation of this file](syn__motor__output_8h.md)


```C++

#ifndef SYN_MOTOR_OUTPUT_H
#define SYN_MOTOR_OUTPUT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void (*set_output)(void *ctx, int32_t output);

    void (*coast)(void *ctx);

    void (*brake)(void *ctx);

    void *ctx;
} SYN_MotorOutput;

#ifdef __cplusplus
}
#endif

#endif /* SYN_MOTOR_OUTPUT_H */
```


