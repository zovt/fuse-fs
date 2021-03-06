#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <bsd/string.h>
#include <assert.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "data.h"

static super_blk* fs;

// implementation for: man 2 access
// Checks if a file exists.
int
nufs_access(const char *path, int mask)
{
	printf("access(%s, %04o)\n", path, mask);
	return fs_access(fs, path, mask);
}

// implementation for: man 2 stat
// gets an object's attributes (type, permissions, size, etc)
int
nufs_getattr(const char *path, struct stat *st)
{
        printf("getattr(%s)\n", path);
        return fs_getattr(fs, path, st);
}

// implementation for: man 2 readdir
// lists the contents of a directory
int
nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
             off_t offset, struct fuse_file_info *fi)
{
        printf("readdir(%s)\n", path);
        return fs_readdir(fs, path, buf, filler, offset, fi);
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
int
nufs_mknod(const char *path, mode_t mode, dev_t rdev)
{
        printf("mknod(%s, %04o)\n", path, mode);
        return fs_mknod(fs, path, mode, rdev);
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
int
nufs_mkdir(const char *path, mode_t mode)
{
        printf("mkdir(%s)\n", path);
        return -1;
}

int
nufs_unlink(const char *path)
{
        printf("unlink(%s)\n", path);
        return fs_unlink(fs, path);
}

int
nufs_rmdir(const char *path)
{
        printf("rmdir(%s)\n", path);
        return -1;
}

// implements: man 2 rename
// called to move a file within the same filesystem
int
nufs_rename(const char *from, const char *to)
{
        printf("rename(%s => %s)\n", from, to);
        return fs_rename(fs, from, to);
}

int
nufs_chmod(const char *path, mode_t mode)
{
        printf("chmod(%s, %04o)\n", path, mode);
        return fs_chmod(fs, path, mode);
}

int
nufs_truncate(const char *path, off_t size)
{
        printf("truncate(%s, %ld bytes)\n", path, size);
        return fs_truncate(fs, path, size);
}

// this is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
int
nufs_open(const char *path, struct fuse_file_info *fi)
{
        printf("open(%s)\n", path);
        return 0;
}

// Actually read data
int
nufs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
        printf("read(%s, %ld bytes, @%ld)\n", path, size, offset);
        return fs_read(fs, path, buf, size, offset, fi);
}

// Actually write data
int
nufs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
        printf("write(%s, %ld bytes, @%ld)\n", path, size, offset);
        return fs_write(fs, path, buf, size, offset, fi);
}

// Update the timestamps on a file or directory.
int
nufs_utimens(const char* path, const struct timespec ts[2])
{
	int rv = fs_utimens(fs, path, ts);
	printf("utimens(%s, [%ld, %ld; %ld %ld]) -> %d\n",
	       path, ts[0].tv_sec, ts[0].tv_nsec, ts[1].tv_sec, ts[1].tv_nsec, rv);
	return rv;
}

int
nufs_link(const char* src, const char* dst) {
	printf("link(%s, %s)\n", src, dst);
	return fs_link(fs, src, dst);
}

void
nufs_init_ops(struct fuse_operations* ops)
{
        memset(ops, 0, sizeof(struct fuse_operations));
        ops->access   = nufs_access;
        ops->getattr  = nufs_getattr;
        ops->readdir  = nufs_readdir;
        ops->mknod    = nufs_mknod;
        ops->mkdir    = nufs_mkdir;
        ops->unlink   = nufs_unlink;
        ops->rmdir    = nufs_rmdir;
        ops->rename   = nufs_rename;
        ops->chmod    = nufs_chmod;
        ops->truncate = nufs_truncate;
        ops->open	  = nufs_open;
        ops->read     = nufs_read;
        ops->write    = nufs_write;
        ops->utimens  = nufs_utimens;
        ops->link     = nufs_link;
};

struct fuse_operations nufs_ops;

int
main(int argc, char *argv[])
{
        assert(argc > 2 && argc < 6);
        fs = init_fs(argv[--argc]);
        nufs_init_ops(&nufs_ops);
        return fuse_main(argc, argv, &nufs_ops, NULL);
}

