

# File syn\_lfs.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**storage**](dir_6dc37c1fd63f78e2ff9ec2810c54ee77.md) **>** [**syn\_lfs.h**](syn__lfs_8h.md)

[Go to the documentation of this file](syn__lfs_8h.md)


```C++

#ifndef SYN_LFS_H
#define SYN_LFS_H

#include "syn_vfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t start_addr;  
    uint32_t size;        
    uint32_t block_size;  
} SYN_LfsConfig;

const SYN_VfsOps *syn_lfs_get_ops(void);

#if __has_include("lfs.h")
#include "lfs.h"

void syn_lfs_init_config(struct lfs_config *cfg, const SYN_LfsConfig *syn_cfg);
#endif

#ifdef __cplusplus
}
#endif

#endif /* SYN_LFS_H */
```


