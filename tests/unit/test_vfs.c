/**
 * @file test_vfs.c
 * @brief Unity tests for Virtual File System (VFS).
 */

#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

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

static const SYN_VfsOps mock_ops = {.open = mock_open,
                                    .close = mock_close,
                                    .read = mock_read,
                                    .write = mock_write,
                                    .seek = mock_seek};

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

static int mock_unlink_called = 0;
static int mock_mkdir_called = 0;
static int mock_opendir_called = 0;
static int mock_readdir_called = 0;
static int mock_closedir_called = 0;
static int mock_tell_called = 0;

static int mock_unlink(const char *path, void *fs_data)
{
    (void)path;
    (void)fs_data;
    mock_unlink_called++;
    return 0;
}

static int mock_mkdir(const char *path, void *fs_data)
{
    (void)path;
    (void)fs_data;
    mock_mkdir_called++;
    return 0;
}

static int mock_opendir(SYN_VfsDir *dir, const char *path, void *fs_data)
{
    (void)dir;
    (void)path;
    (void)fs_data;
    mock_opendir_called++;
    return 0;
}

static int mock_readdir(SYN_VfsDir *dir, SYN_VfsDirEnt *ent)
{
    (void)dir;
    mock_readdir_called++;
    strcpy(ent->name, "test_file.txt");
    ent->size = 123;
    ent->is_dir = false;
    return 1;
}

static int mock_closedir(SYN_VfsDir *dir)
{
    (void)dir;
    mock_closedir_called++;
    return 0;
}

static int32_t mock_tell(SYN_VfsFile *file)
{
    (void)file;
    mock_tell_called++;
    return 42;
}

static int mock_open_fail(SYN_VfsFile *file, const char *path, int flags, void *fs_data)
{
    (void)file;
    (void)path;
    (void)flags;
    (void)fs_data;
    return -5;
}

static int mock_opendir_fail(SYN_VfsDir *dir, const char *path, void *fs_data)
{
    (void)dir;
    (void)path;
    (void)fs_data;
    return -6;
}

static const SYN_VfsOps mock_full_ops = {.open = mock_open,
                                         .close = mock_close,
                                         .read = mock_read,
                                         .write = mock_write,
                                         .seek = mock_seek,
                                         .tell = mock_tell,
                                         .unlink = mock_unlink,
                                         .mkdir = mock_mkdir,
                                         .opendir = mock_opendir,
                                         .readdir = mock_readdir,
                                         .closedir = mock_closedir};

static const SYN_VfsOps mock_minimal_ops = {.open = mock_open};

static const SYN_VfsOps mock_fail_ops = {.open = mock_open_fail, .opendir = mock_opendir_fail};

static void test_vfs_edge_cases(void)
{
    syn_vfs_init();

    /* 1. Mount overflow */
    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_mount("/m1", &mock_ops, NULL));
    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_mount("/m2", &mock_ops, NULL));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_vfs_mount("/m3", &mock_ops, NULL));

    /* Reset and mount one full ops, one minimal ops, and one failing ops */
    syn_vfs_init();
    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_mount("/full", &mock_full_ops, NULL));
    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_mount("/mini", &mock_minimal_ops, NULL));

    /* 2. Prefix matching edge cases */
    /* "/full" prefix should not match path "/fully" */
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_open("/fully", SYN_O_RDONLY));
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_open("/fuller/file", SYN_O_RDONLY));

    /* Prefix match without boundary slash - "/full" prefix matches "/full" */
    mock_open_called = 0;
    int fd = syn_vfs_open("/full", SYN_O_RDONLY);
    TEST_ASSERT_TRUE(fd >= 0);
    TEST_ASSERT_EQUAL_INT(1, mock_open_called);
    syn_vfs_close(fd);

    /* 3. Invalid file descriptor checks */
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_read(-1, NULL, 10));
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_read(SYN_VFS_MAX_OPEN_FILES, NULL, 10));
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_read(0, NULL, 10));

    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_write(-1, NULL, 10));
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_write(SYN_VFS_MAX_OPEN_FILES, NULL, 10));
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_write(0, NULL, 10));

    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_seek(-1, 0, SYN_SEEK_SET));
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_seek(SYN_VFS_MAX_OPEN_FILES, 0, SYN_SEEK_SET));
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_seek(0, 0, SYN_SEEK_SET));

    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_tell(-1));
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_tell(SYN_VFS_MAX_OPEN_FILES));
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_tell(0));

    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_close(-1));
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_close(SYN_VFS_MAX_OPEN_FILES));
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_close(0));

    /* 4. Missing VfsOps Callbacks (Minimal Ops) */
    fd = syn_vfs_open("/mini/file.txt", SYN_O_RDONLY);
    TEST_ASSERT_TRUE(fd >= 0);

    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_read(fd, NULL, 10));
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_write(fd, NULL, 10));
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_seek(fd, 0, SYN_SEEK_SET));
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_tell(fd));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd));

    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_unlink("/mini/file.txt"));
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_mkdir("/mini/dir"));

    /* Test syn_vfs_tell on a file that implements tell */
    fd = syn_vfs_open("/full/file.txt", SYN_O_RDONLY);
    TEST_ASSERT_TRUE(fd >= 0);
    mock_tell_called = 0;
    TEST_ASSERT_EQUAL_INT(42, syn_vfs_tell(fd));
    TEST_ASSERT_EQUAL_INT(1, mock_tell_called);
    syn_vfs_close(fd);

    /* 5. Directory Operations & Exhaustion */
    mock_opendir_called = 0;
    mock_readdir_called = 0;
    mock_closedir_called = 0;
    mock_unlink_called = 0;
    mock_mkdir_called = 0;
    mock_tell_called = 0;

    TEST_ASSERT_EQUAL_INT(0, syn_vfs_mkdir("/full/dir"));
    TEST_ASSERT_EQUAL_INT(1, mock_mkdir_called);

    TEST_ASSERT_EQUAL_INT(0, syn_vfs_unlink("/full/file.txt"));
    TEST_ASSERT_EQUAL_INT(1, mock_unlink_called);

    int dd1 = syn_vfs_opendir("/full/dir");
    TEST_ASSERT_TRUE(dd1 >= 0);
    TEST_ASSERT_EQUAL_INT(1, mock_opendir_called);

    int dd2 = syn_vfs_opendir("/full/dir");
    TEST_ASSERT_TRUE(dd2 >= 0);

    int dd3 = syn_vfs_opendir("/full/dir");
    TEST_ASSERT_EQUAL_INT(-2, dd3);

    /* Invalid directory descriptors */
    SYN_VfsDirEnt ent;
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_readdir(-1, &ent));
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_readdir(SYN_VFS_MAX_OPEN_DIRS, &ent));

    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_closedir(-1));
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_closedir(SYN_VFS_MAX_OPEN_DIRS));

    /* Readdir and Closedir */
    TEST_ASSERT_EQUAL_INT(1, syn_vfs_readdir(dd1, &ent));
    TEST_ASSERT_EQUAL_STRING("test_file.txt", ent.name);
    TEST_ASSERT_EQUAL_INT(123, ent.size);
    TEST_ASSERT_FALSE(ent.is_dir);
    TEST_ASSERT_EQUAL_INT(1, mock_readdir_called);

    TEST_ASSERT_EQUAL_INT(0, syn_vfs_closedir(dd1));
    TEST_ASSERT_EQUAL_INT(1, mock_closedir_called);

    /* Now that dd1 (slot 0) is closed, verify it returns error */
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_readdir(dd1, &ent));
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_closedir(dd1));

    /* Open directory on minimal mount */
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_opendir("/mini/dir"));

    /* Clean up dd2 */
    syn_vfs_closedir(dd2);

    /* 6. Fail-to-open and fail-to-opendir callback scenarios */
    syn_vfs_init();
    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_mount("/fail", &mock_fail_ops, NULL));

    TEST_ASSERT_EQUAL_INT(-5, syn_vfs_open("/fail/file.txt", SYN_O_RDONLY));
    TEST_ASSERT_EQUAL_INT(-6, syn_vfs_opendir("/fail/dir"));

    /* 7. Directory operations with missing readdir and closedir callbacks */
    syn_vfs_init();
    static const SYN_VfsOps mock_no_dir_callbacks_ops = {.opendir = mock_opendir};
    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_mount("/nocb", &mock_no_dir_callbacks_ops, NULL));
    int dd_nocb = syn_vfs_opendir("/nocb/dir");
    TEST_ASSERT_TRUE(dd_nocb >= 0);
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_readdir(dd_nocb, &ent));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_closedir(dd_nocb));
}

static int mock_stat(const char *path, SYN_VfsDirEnt *ent, void *fs_data)
{
    (void)path;
    (void)fs_data;
    if (ent) {
        strcpy(ent->name, "stat_file.txt");
        ent->size = 1024;
        ent->is_dir = false;
    }
    return 0;
}

static int mock_rename(const char *old_path, const char *new_path, void *fs_data)
{
    (void)old_path;
    (void)new_path;
    (void)fs_data;
    return 0;
}

static void test_vfs_unmount_stat_rename(void)
{
    syn_vfs_init();

    static const SYN_VfsOps ops = {
        .open = mock_open, .close = mock_close, .stat = mock_stat, .rename = mock_rename};

    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_mount("/flash", &ops, NULL));

    SYN_VfsDirEnt ent;
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_stat("/flash/data.bin", &ent));
    TEST_ASSERT_EQUAL_STRING("stat_file.txt", ent.name);

    TEST_ASSERT_EQUAL_INT(0, syn_vfs_rename("/flash/old.txt", "/flash/new.txt"));

    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_unmount("/flash"));
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_open("/flash/data.bin", SYN_O_RDONLY));
}

static void test_vfs_duplicate_mount_and_unmount_nonexistent(void)
{
    syn_vfs_init();
    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_mount("/flash", &mock_full_ops, NULL));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_vfs_mount("/flash", &mock_full_ops, NULL));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_vfs_unmount("/nonexistent"));
}

static const SYN_VfsOps ops_open_fail = {.open = mock_open_fail};
static const SYN_VfsOps ops_dir_fail = {.opendir = mock_opendir_fail};

static void test_vfs_unmount_shift_and_null_checks(void)
{
    syn_vfs_init();
    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_mount("/m1", &mock_full_ops, NULL));
    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_mount("/m2", &mock_full_ops, NULL));

    /* Unmount /m1 to trigger shift loop j = i .. g_mount_count - 1 */
    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_unmount("/m1"));

    /* Test stat and rename missing ops */
    SYN_VfsDirEnt ent;
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_stat("/m2/file.txt", &ent));
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_rename("/m2/file1", "/m2/file2"));
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_rename("/m2/file1", "/nonexistent/file2"));

    /* Test ops without read, write, seek, tell callbacks */
    syn_vfs_init();
    static const SYN_VfsOps mock_minimal_ops = {.open = mock_open};
    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_mount("/minimal", &mock_minimal_ops, NULL));
    int fd_min = syn_vfs_open("/minimal/file.txt", SYN_O_RDWR);
    TEST_ASSERT_TRUE(fd_min >= 0);
    uint8_t dummy[4];
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_read(fd_min, dummy, sizeof(dummy)));
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_write(fd_min, dummy, sizeof(dummy)));
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_seek(fd_min, 0, SYN_SEEK_SET));
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_tell(fd_min));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd_min));

    /* Test find_mount prefix character mismatch (line 105) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_mount("/sd", &mock_full_ops, NULL));
    int fd_mismatch = syn_vfs_open("/sp/file.txt", SYN_O_RDONLY);
    TEST_ASSERT_EQUAL_INT(-1, fd_mismatch);

    /* Exact mount path match -> rel_path becomes "/" (lines 122-124) */
    syn_vfs_init();
    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_mount("/mount_exact", &mock_minimal_ops, NULL));
    int fd_exact = syn_vfs_open("/mount_exact", SYN_O_RDONLY);
    TEST_ASSERT_TRUE(fd_exact >= 0);
    syn_vfs_close(fd_exact);

    /* Open failure callback returning -5 (lines 184-187) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_mount("/fail_open", &ops_open_fail, NULL));
    TEST_ASSERT_EQUAL_INT(-5, syn_vfs_open("/fail_open/file", SYN_O_RDONLY));

    /* Rename across different mounts (line 149) */
    syn_vfs_init();
    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_mount("/m_a", &mock_full_ops, NULL));
    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_mount("/m_b", &mock_full_ops, NULL));
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_rename("/m_a/file1", "/m_b/file2"));

    /* Opendir failure callback and opendir exhaustion (lines 291 & 300) */
    syn_vfs_init();
    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_mount("/fail_dir", &ops_dir_fail, NULL));
    TEST_ASSERT_EQUAL_INT(-6, syn_vfs_opendir("/fail_dir"));

    syn_vfs_init();
    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_mount("/dir_ok", &mock_full_ops, NULL));
    int dirs[SYN_VFS_MAX_OPEN_DIRS];
    for (int i = 0; i < SYN_VFS_MAX_OPEN_DIRS; i++) {
        dirs[i] = syn_vfs_opendir("/dir_ok");
        TEST_ASSERT_TRUE(dirs[i] >= 0);
    }
    TEST_ASSERT_EQUAL_INT(-2, syn_vfs_opendir("/dir_ok"));
    for (int i = 0; i < SYN_VFS_MAX_OPEN_DIRS; i++) {
        syn_vfs_closedir(dirs[i]);
    }

    syn_vfs_init();
}

static const char *tier_a_last_path = NULL;
static void *tier_a_last_fs_data = NULL;

static int tier_a_capture_open(SYN_VfsFile *file, const char *path, int flags, void *fs_data)
{
    (void)file;
    (void)flags;
    tier_a_last_path = path;
    tier_a_last_fs_data = fs_data;
    return 0;
}

static void test_tier_a_vfs_busy_unmount_root_and_readdir(void)
{
    syn_vfs_init();
    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_mount("/flash", &mock_full_ops, NULL));

    /* Busy unmount rejected while a file descriptor is open */
    int fd = syn_vfs_open("/flash/log.bin", SYN_O_RDONLY);
    TEST_ASSERT_TRUE(fd >= 0);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_vfs_unmount("/flash"));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd));

    /* Busy unmount rejected while a directory descriptor is open, and NULL ent guard */
    int dd = syn_vfs_opendir("/flash");
    TEST_ASSERT_TRUE(dd >= 0);
    TEST_ASSERT_EQUAL_INT(-1, syn_vfs_readdir(dd, NULL));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_vfs_unmount("/flash"));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_closedir(dd));
    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_unmount("/flash"));

    /* Root "/" prefix mount + nested "/sub" longest-prefix resolution */
    static const SYN_VfsOps capture_ops = {.open = tier_a_capture_open, .close = mock_close};
    int root_tag = 1;
    int sub_tag = 2;
    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_mount("/sub", &capture_ops, &sub_tag));
    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_mount("/", &capture_ops, &root_tag));

    int fd_root = syn_vfs_open("/hello.txt", SYN_O_RDONLY);
    TEST_ASSERT_TRUE(fd_root >= 0);
    TEST_ASSERT_EQUAL_STRING("/hello.txt", tier_a_last_path);
    TEST_ASSERT_EQUAL_PTR(&root_tag, tier_a_last_fs_data);

    int fd_slash = syn_vfs_open("/", SYN_O_RDONLY);
    TEST_ASSERT_TRUE(fd_slash >= 0);
    TEST_ASSERT_EQUAL_STRING("/", tier_a_last_path);
    TEST_ASSERT_EQUAL_PTR(&root_tag, tier_a_last_fs_data);
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd_slash));

    int fd_sub = syn_vfs_open("/sub/item.txt", SYN_O_RDONLY);
    TEST_ASSERT_TRUE(fd_sub >= 0);
    TEST_ASSERT_EQUAL_STRING("/item.txt", tier_a_last_path);
    TEST_ASSERT_EQUAL_PTR(&sub_tag, tier_a_last_fs_data);
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd_sub));

    /* Idle "/sub" unmounts cleanly even while "/" still holds fd_root open */
    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_unmount("/sub"));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_vfs_unmount("/"));
    TEST_ASSERT_EQUAL_INT(0, syn_vfs_close(fd_root));
    TEST_ASSERT_EQUAL(SYN_OK, syn_vfs_unmount("/"));

    syn_vfs_init();
}

void run_vfs_tests(void)
{
    RUN_TEST(test_vfs_basic);
    RUN_TEST(test_vfs_exhaustion);
    RUN_TEST(test_vfs_edge_cases);
    RUN_TEST(test_vfs_unmount_stat_rename);
    RUN_TEST(test_vfs_duplicate_mount_and_unmount_nonexistent);
    RUN_TEST(test_vfs_unmount_shift_and_null_checks);
    RUN_TEST(test_tier_a_vfs_busy_unmount_root_and_readdir);
}
