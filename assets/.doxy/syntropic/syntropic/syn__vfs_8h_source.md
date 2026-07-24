

# File syn\_vfs.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**storage**](dir_6dc37c1fd63f78e2ff9ec2810c54ee77.md) **>** [**syn\_vfs.h**](syn__vfs_8h.md)

[Go to the documentation of this file](syn__vfs_8h.md)


```C++

#ifndef SYN_VFS_H
#define SYN_VFS_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SYN_VFS_MAX_OPEN_FILES
  #define SYN_VFS_MAX_OPEN_FILES  4  
#endif

#ifndef SYN_VFS_MAX_OPEN_DIRS
  #define SYN_VFS_MAX_OPEN_DIRS   2  
#endif

#ifndef SYN_VFS_MAX_MOUNTS
  #define SYN_VFS_MAX_MOUNTS      2  
#endif
#define SYN_O_RDONLY  0x00  
#define SYN_O_WRONLY  0x01  
#define SYN_O_RDWR    0x02  
#define SYN_O_CREAT   0x04  
#define SYN_O_APPEND  0x08  
#define SYN_O_TRUNC   0x10  
#define SYN_SEEK_SET  0  
#define SYN_SEEK_CUR  1  
#define SYN_SEEK_END  2  
typedef struct {
    char     name[64];  
    uint32_t size;      
    bool     is_dir;    
} SYN_VfsDirEnt;

typedef struct SYN_VfsFile SYN_VfsFile;  
typedef struct SYN_VfsDir SYN_VfsDir;    
typedef struct {
    int     (*open)(SYN_VfsFile *file, const char *path, int flags, void *fs_data);  
    int     (*close)(SYN_VfsFile *file);                                              
    int     (*read)(SYN_VfsFile *file, void *buf, size_t len);                        
    int     (*write)(SYN_VfsFile *file, const void *buf, size_t len);                 
    int32_t (*seek)(SYN_VfsFile *file, int32_t offset, int whence);                   
    int32_t (*tell)(SYN_VfsFile *file);                                               
    int     (*unlink)(const char *path, void *fs_data);                               
    int     (*mkdir)(const char *path, void *fs_data);                                
    int     (*opendir)(SYN_VfsDir *dir, const char *path, void *fs_data);             
    int     (*readdir)(SYN_VfsDir *dir, SYN_VfsDirEnt *ent);                          
    int     (*closedir)(SYN_VfsDir *dir);                                             
} SYN_VfsOps;

struct SYN_VfsFile {
    const SYN_VfsOps *ops;      
    void             *fs_file;  
    bool              is_open;  
};

struct SYN_VfsDir {
    const SYN_VfsOps *ops;      
    void             *fs_dir;   
    bool              is_open;  
};

typedef struct {
    const char       *prefix;      
    size_t            prefix_len;  
    const SYN_VfsOps *ops;         
    void             *fs_data;     
} SYN_VfsMount;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_vfs_init(void);

SYN_Status syn_vfs_mount(const char *prefix, const SYN_VfsOps *ops, void *fs_data);

int syn_vfs_open(const char *path, int flags);

int syn_vfs_close(int fd);

int syn_vfs_read(int fd, void *buf, size_t len);

int syn_vfs_write(int fd, const void *buf, size_t len);

int32_t syn_vfs_seek(int fd, int32_t offset, int whence);

int32_t syn_vfs_tell(int fd);

int syn_vfs_unlink(const char *path);

int syn_vfs_mkdir(const char *path);

int syn_vfs_opendir(const char *path);

int syn_vfs_readdir(int dd, SYN_VfsDirEnt *ent);

int syn_vfs_closedir(int dd);

#ifdef __cplusplus
}
#endif

#endif /* SYN_VFS_H */
```


