/**
 * @file test_vfs.c
 * @brief Unity tests for Virtual File System (VFS).
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"

static int mock_open_called = 0;
static int mock_read_called = 0;
static int mock_write_called = 0;
static int mock_close_called = 0;
static int mock_seek_called = 0;

static int mock_open(SYN_VfsFile *file, const char *path, int flags, void *fs_data)
{
    (void)file;
    (void)path;
    (void)flags;
    (void)fs_data;
    mock_open_called++;
    return 0;
}

static int mock_close(SYN_VfsFile *file)
{
    (void)file;
    mock_close_called++;
    return 0;
}

static int mock_read(SYN_VfsFile *file, void *buf, size_t len)
{
    (void)file;
    (void)buf;
    mock_read_called++;
    return (int)len;
}

static int mock_write(SYN_VfsFile *file, const void *buf, size_t len)
{
    (void)file;
    (void)buf;
    mock_write_called++;
    return (int)len;
}

static int32_t mock_seek(SYN_VfsFile *file, int32_t offset, int whence)
{
    (void)file;
    (void)offset;
    (void)whence;
    mock_seek_called++;
    return offset;
}

static const SYN_VfsOps mock_ops = {
    .open  = mock_open,
    .close = mock_close,
    .read  = mock_read,
    .write = mock_write,
    .seek  = mock_seek
};

static void test_vfs_basic(void)
{
    syn_vfs_init();

    /* Mount mock volume */
    SYN_Status st = syn_vfs_mount("/mock", &mock_ops, NULL);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* Try mounting same path (should fail) */
    st = syn_vfs_mount("/mock", &mock_ops, NULL);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);

    mock_open_called = 0;
    mock_read_called = 0;
    mock_write_called = 0;
    mock_close_called = 0;
    mock_seek_called = 0;

    /* Open */
    int fd = syn_vfs_open("/mock/hello.txt", SYN_O_RDWR);
    TEST_ASSERT_TRUE(fd >= 0);
    TEST_ASSERT_EQUAL_INT(1, mock_open_called);

    /* Read */
    uint8_t buffer[10];
    int n = syn_vfs_read(fd, buffer, sizeof(buffer));
    TEST_ASSERT_EQUAL_INT(sizeof(buffer), n);
    TEST_ASSERT_EQUAL_INT(1, mock_read_called);

    /* Write */
    n = syn_vfs_write(fd, "test", 4);
    TEST_ASSERT_EQUAL_INT(4, n);
    TEST_ASSERT_EQUAL_INT(1, mock_write_called);

    /* Seek */
    int32_t pos = syn_vfs_seek(fd, 100, SYN_SEEK_SET);
    TEST_ASSERT_EQUAL_INT(100, pos);
    TEST_ASSERT_EQUAL_INT(1, mock_seek_called);

    /* Close */
    int ret = syn_vfs_close(fd);
    TEST_ASSERT_EQUAL_INT(0, ret);
    TEST_ASSERT_EQUAL_INT(1, mock_close_called);
}

static void test_vfs_exhaustion(void)
{
    syn_vfs_init();
    SYN_Status st = syn_vfs_mount("/mock", &mock_ops, NULL);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    int fds[SYN_VFS_MAX_OPEN_FILES];
    for (int i = 0; i < SYN_VFS_MAX_OPEN_FILES; i++) {
        fds[i] = syn_vfs_open("/mock/file.txt", SYN_O_RDONLY);
        TEST_ASSERT_TRUE(fds[i] >= 0);
    }

    /* Try opening one more (should fail with -2) */
    int fd_extra = syn_vfs_open("/mock/extra.txt", SYN_O_RDONLY);
    TEST_ASSERT_EQUAL_INT(-2, fd_extra);

    /* Close one and retry opening (slot should be reused) */
    int ret = syn_vfs_close(fds[0]);
    TEST_ASSERT_EQUAL_INT(0, ret);

    fd_extra = syn_vfs_open("/mock/extra.txt", SYN_O_RDONLY);
    TEST_ASSERT_TRUE(fd_extra >= 0);

    /* Cleanup remaining */
    syn_vfs_close(fd_extra);
    for (int i = 1; i < SYN_VFS_MAX_OPEN_FILES; i++) {
        syn_vfs_close(fds[i]);
    }
}

void run_vfs_tests(void)
{
    RUN_TEST(test_vfs_basic);
    RUN_TEST(test_vfs_exhaustion);
}
