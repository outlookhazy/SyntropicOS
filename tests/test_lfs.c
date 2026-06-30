#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "mocks/lfs.h"
#include "syntropic/storage/syn_lfs.h"
#include "syntropic/storage/syn_vfs.h"
#include <string.h>

/* Mock State */
static int mock_lfs_ret;
static lfs_size_t mock_lfs_read_ret;
static lfs_size_t mock_lfs_write_ret;
static lfs_soff_t mock_lfs_seek_ret;
static lfs_soff_t mock_lfs_tell_ret;
static int mock_lfs_dir_read_ret;
static struct lfs_info mock_lfs_info;
static int mock_lfs_flags;

void mock_lfs_reset(void)
{
    mock_lfs_ret = 0;
    mock_lfs_read_ret = 0;
    mock_lfs_write_ret = 0;
    mock_lfs_seek_ret = 0;
    mock_lfs_tell_ret = 0;
    mock_lfs_dir_read_ret = 0;
    memset(&mock_lfs_info, 0, sizeof(mock_lfs_info));
    mock_lfs_flags = 0;
}

/* Mock Implementations */
int lfs_file_open(lfs_t *lfs, lfs_file_t *file, const char *path, int flags)
{
    (void)lfs; (void)file; (void)path;
    mock_lfs_flags = flags;
    return mock_lfs_ret;
}

int lfs_file_close(lfs_t *lfs, lfs_file_t *file)
{
    (void)lfs; (void)file;
    return mock_lfs_ret;
}

lfs_ssize_t lfs_file_read(lfs_t *lfs, lfs_file_t *file, void *buffer, lfs_size_t size)
{
    (void)lfs; (void)file; (void)buffer; (void)size;
    return mock_lfs_read_ret;
}

lfs_ssize_t lfs_file_write(lfs_t *lfs, lfs_file_t *file, const void *buffer, lfs_size_t size)
{
    (void)lfs; (void)file; (void)buffer; (void)size;
    return mock_lfs_write_ret;
}

lfs_soff_t lfs_file_seek(lfs_t *lfs, lfs_file_t *file, lfs_soff_t off, int whence)
{
    (void)lfs; (void)file; (void)off; (void)whence;
    return mock_lfs_seek_ret;
}

lfs_soff_t lfs_file_tell(lfs_t *lfs, lfs_file_t *file)
{
    (void)lfs; (void)file;
    return mock_lfs_tell_ret;
}

int lfs_remove(lfs_t *lfs, const char *path)
{
    (void)lfs; (void)path;
    return mock_lfs_ret;
}

int lfs_mkdir(lfs_t *lfs, const char *path)
{
    (void)lfs; (void)path;
    return mock_lfs_ret;
}

int lfs_dir_open(lfs_t *lfs, lfs_dir_t *dir, const char *path)
{
    (void)lfs; (void)dir; (void)path;
    return mock_lfs_ret;
}

int lfs_dir_read(lfs_t *lfs, lfs_dir_t *dir, struct lfs_info *info)
{
    (void)lfs; (void)dir;
    if (mock_lfs_dir_read_ret > 0) {
        *info = mock_lfs_info;
    }
    return mock_lfs_dir_read_ret;
}

int lfs_dir_close(lfs_t *lfs, lfs_dir_t *dir)
{
    (void)lfs; (void)dir;
    return mock_lfs_ret;
}

/* ── Tests ──────────────────────────────────────────────────────────────── */

static void test_lfs_init_config(void)
{
    struct lfs_config cfg;
    SYN_LfsConfig syn_cfg = { .start_addr = 0, .size = 1024, .block_size = 512 };
    
    memset(&cfg, 0, sizeof(cfg));
    syn_lfs_init_config(&cfg, &syn_cfg);
    
    TEST_ASSERT_EQUAL_PTR(&syn_cfg, cfg.context);
    TEST_ASSERT_NOT_NULL(cfg.read);
    TEST_ASSERT_NOT_NULL(cfg.prog);
    TEST_ASSERT_NOT_NULL(cfg.erase);
    TEST_ASSERT_NOT_NULL(cfg.sync);
    TEST_ASSERT_EQUAL(512, cfg.block_size);
    TEST_ASSERT_EQUAL(2, cfg.block_count);
    
    /* Test block device callbacks */
    mock_port_reset();
    
    uint8_t buf[16] = {0};
    int st = cfg.read(&cfg, 1, 16, buf, 16);
    TEST_ASSERT_EQUAL(LFS_ERR_OK, st);
    
    st = cfg.prog(&cfg, 1, 16, buf, 16);
    TEST_ASSERT_EQUAL(LFS_ERR_OK, st);
    
    st = cfg.erase(&cfg, 1);
    TEST_ASSERT_EQUAL(LFS_ERR_OK, st);
    
    st = cfg.sync(&cfg);
    TEST_ASSERT_EQUAL(LFS_ERR_OK, st);
}

static void test_lfs_bd_errors(void)
{
    struct lfs_config cfg;
    SYN_LfsConfig syn_cfg = { .start_addr = 0, .size = 1024, .block_size = 512 };
    syn_lfs_init_config(&cfg, &syn_cfg);
    
    mock_port_reset();
    
    uint8_t buf[16] = {0};
    
    /* Simulate Flash Error by going OOB without wrapping */
    syn_cfg.start_addr = 4096; /* Force OOB */
    
    int st = cfg.read(&cfg, 1, 16, buf, 16);
    TEST_ASSERT_EQUAL(LFS_ERR_IO, st);
    
    st = cfg.prog(&cfg, 1, 16, buf, 16);
    TEST_ASSERT_EQUAL(LFS_ERR_IO, st);
    
    st = cfg.erase(&cfg, 1);
    TEST_ASSERT_EQUAL(LFS_ERR_IO, st);
}

static void test_lfs_vfs_file(void)
{
    mock_lfs_reset();
    
    const SYN_VfsOps *ops = syn_lfs_get_ops();
    TEST_ASSERT_NOT_NULL(ops);
    
    lfs_t dummy_lfs;
    SYN_VfsFile file;
    memset(&file, 0, sizeof(file));
    
    /* Test open variations */
    int st = ops->open(&file, "test.txt", SYN_O_RDONLY, &dummy_lfs);
    TEST_ASSERT_EQUAL(0, st);
    TEST_ASSERT_EQUAL(LFS_O_RDONLY, mock_lfs_flags);
    
    ops->close(&file);
    
    ops->open(&file, "test.txt", SYN_O_WRONLY | SYN_O_CREAT | SYN_O_APPEND, &dummy_lfs);
    TEST_ASSERT_EQUAL(LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND, mock_lfs_flags);
    ops->close(&file);
    
    ops->open(&file, "test.txt", SYN_O_RDWR | SYN_O_TRUNC, &dummy_lfs);
    TEST_ASSERT_EQUAL(LFS_O_RDWR | LFS_O_TRUNC, mock_lfs_flags);
    
    /* IO */
    mock_lfs_read_ret = 42;
    TEST_ASSERT_EQUAL(42, ops->read(&file, NULL, 0));
    
    mock_lfs_write_ret = 24;
    TEST_ASSERT_EQUAL(24, ops->write(&file, NULL, 0));
    
    mock_lfs_seek_ret = 100;
    TEST_ASSERT_EQUAL(100, ops->seek(&file, 0, SYN_SEEK_CUR));
    TEST_ASSERT_EQUAL(100, ops->seek(&file, 0, SYN_SEEK_END));
    TEST_ASSERT_EQUAL(100, ops->seek(&file, 0, SYN_SEEK_SET));
    
    mock_lfs_tell_ret = 200;
    TEST_ASSERT_EQUAL(200, ops->tell(&file));
    
    ops->close(&file);
}

static void test_lfs_vfs_file_errors(void)
{
    mock_lfs_reset();
    const SYN_VfsOps *ops = syn_lfs_get_ops();
    
    /* Open fails */
    mock_lfs_ret = -1;
    SYN_VfsFile file;
    lfs_t dummy_lfs;
    TEST_ASSERT_EQUAL(-1, ops->open(&file, "test.txt", SYN_O_RDONLY, &dummy_lfs));
    
    /* Operations on unopened file */
    memset(&file, 0, sizeof(file));
    TEST_ASSERT_EQUAL(-1, ops->read(&file, NULL, 0));
    TEST_ASSERT_EQUAL(-1, ops->write(&file, NULL, 0));
    TEST_ASSERT_EQUAL(-1, ops->seek(&file, 0, SYN_SEEK_SET));
    TEST_ASSERT_EQUAL(-1, ops->tell(&file));
    TEST_ASSERT_EQUAL(-1, ops->close(&file));
    
    /* Pool exhaustion */
    mock_lfs_ret = 0;
    SYN_VfsFile files[SYN_VFS_MAX_OPEN_FILES + 1];
    for (int i = 0; i < SYN_VFS_MAX_OPEN_FILES; i++) {
        TEST_ASSERT_EQUAL(0, ops->open(&files[i], "test", SYN_O_RDONLY, &dummy_lfs));
    }
    
    /* Next one should fail */
    TEST_ASSERT_EQUAL(-1, ops->open(&files[SYN_VFS_MAX_OPEN_FILES], "test", SYN_O_RDONLY, &dummy_lfs));
    
    /* Clean up */
    for (int i = 0; i < SYN_VFS_MAX_OPEN_FILES; i++) {
        ops->close(&files[i]);
    }
}

static void test_lfs_vfs_dir(void)
{
    mock_lfs_reset();
    const SYN_VfsOps *ops = syn_lfs_get_ops();
    lfs_t dummy_lfs;
    
    /* Unlink/mkdir */
    TEST_ASSERT_EQUAL(0, ops->unlink("test.txt", &dummy_lfs));
    TEST_ASSERT_EQUAL(0, ops->mkdir("dir", &dummy_lfs));
    
    /* Dir open */
    SYN_VfsDir dir;
    memset(&dir, 0, sizeof(dir));
    TEST_ASSERT_EQUAL(0, ops->opendir(&dir, "dir", &dummy_lfs));
    
    /* Dir read */
    mock_lfs_dir_read_ret = 1;
    mock_lfs_info.type = LFS_TYPE_REG;
    mock_lfs_info.size = 123;
    strcpy(mock_lfs_info.name, "file.txt");
    
    SYN_VfsDirEnt ent;
    memset(&ent, 0, sizeof(ent));
    TEST_ASSERT_EQUAL(1, ops->readdir(&dir, &ent));
    TEST_ASSERT_EQUAL_STRING("file.txt", ent.name);
    TEST_ASSERT_EQUAL(123, ent.size);
    TEST_ASSERT_FALSE(ent.is_dir);
    
    mock_lfs_info.type = LFS_TYPE_DIR;
    TEST_ASSERT_EQUAL(1, ops->readdir(&dir, &ent));
    TEST_ASSERT_TRUE(ent.is_dir);
    
    /* Dir read EOF */
    mock_lfs_dir_read_ret = 0;
    TEST_ASSERT_EQUAL(0, ops->readdir(&dir, &ent));
    
    /* Dir close */
    TEST_ASSERT_EQUAL(0, ops->closedir(&dir));
}

static void test_lfs_vfs_dir_errors(void)
{
    mock_lfs_reset();
    const SYN_VfsOps *ops = syn_lfs_get_ops();
    lfs_t dummy_lfs;
    
    /* Open fails */
    mock_lfs_ret = -1;
    SYN_VfsDir dir;
    TEST_ASSERT_EQUAL(-1, ops->opendir(&dir, "dir", &dummy_lfs));
    
    /* Operations on unopened dir */
    memset(&dir, 0, sizeof(dir));
    SYN_VfsDirEnt ent;
    TEST_ASSERT_EQUAL(-1, ops->readdir(&dir, &ent));
    TEST_ASSERT_EQUAL(-1, ops->closedir(&dir));
    
    /* Pool exhaustion */
    mock_lfs_ret = 0;
    SYN_VfsDir dirs[SYN_VFS_MAX_OPEN_DIRS + 1];
    for (int i = 0; i < SYN_VFS_MAX_OPEN_DIRS; i++) {
        TEST_ASSERT_EQUAL(0, ops->opendir(&dirs[i], "dir", &dummy_lfs));
    }
    
    /* Next one should fail */
    TEST_ASSERT_EQUAL(-1, ops->opendir(&dirs[SYN_VFS_MAX_OPEN_DIRS], "dir", &dummy_lfs));
    
    /* Clean up */
    for (int i = 0; i < SYN_VFS_MAX_OPEN_DIRS; i++) {
        ops->closedir(&dirs[i]);
    }
}

void run_lfs_tests(void)
{
    RUN_TEST(test_lfs_init_config);
    RUN_TEST(test_lfs_bd_errors);
    RUN_TEST(test_lfs_vfs_file);
    RUN_TEST(test_lfs_vfs_file_errors);
    RUN_TEST(test_lfs_vfs_dir);
    RUN_TEST(test_lfs_vfs_dir_errors);
}
