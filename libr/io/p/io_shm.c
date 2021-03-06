/* radare - LGPL - Copyright 2008-2012 pancake<nopcode.org> */

#include "r_io.h"
#include "r_lib.h"
#include <sys/types.h>

#ifdef __ANDROID__
#undef __UNIX__
#define __UNIX__ 0
#endif

#if __UNIX__ && !defined (__QNX__)
#include <sys/ipc.h>
#include <sys/shm.h>

typedef struct {
	int fd;
	int id;
	ut8 *buf;
	ut32 size;
} RIOShm;
#define RIOSHM_FD(x) (((RIOShm*)x)->fd)

#define SHMATSZ 32*1024*1024; /* 32MB : XXX not used correctly? */

static int shm__write(RIO *io, RIODesc *fd, const ut8 *buf, int count) {
	RIOShm *shm;
	if (fd == NULL || fd->data == NULL)
		return -1;
	shm = fd->data;
	if (shm->buf != NULL) {
        	(void)memcpy (shm->buf+io->off, buf, count);
		return count;
	}
	return -1;
}

static int shm__read(RIO *io, RIODesc *fd, ut8 *buf, int count) {
	RIOShm *shm;
	if (fd == NULL || fd->data == NULL)
		return -1;
	shm = fd->data;
	if (io->off > shm->size)
		io->off = shm->size;
	memcpy (buf, shm->buf+io->off , count);
        return count;
}

static int shm__close(RIODesc *fd) {
	int ret;
	if (fd == NULL || fd->data == NULL)
		return -1;
	ret = shmdt (((RIOShm*)(fd->data))->buf);
	free (fd->data);
	fd->data = NULL;
	return ret;
}

static ut64 shm__lseek(RIO *io, RIODesc *fd, ut64 offset, int whence) {
	RIOShm *shm;
	if (fd == NULL || fd->data == NULL)
		return -1;
	shm = fd->data;
	switch (whence) {
	case SEEK_SET:
		return offset;
	case SEEK_CUR:
		if (io->off+offset>shm->size)
			return shm->size;
		return io->off + offset;
	case SEEK_END:
		return 0xffffffff;
	}
	return io->off;
}

static int shm__plugin_open(RIO *io, const char *pathname) {
	return (!memcmp (pathname, "shm://", 6));
}

static inline int getshmid (const char *str) {
	return atoi (str);
}

static inline int getshmfd (RIOShm *shm) {
	return (int)(size_t)shm->buf;
}

static RIODesc *shm__open(RIO *io, const char *pathname, int rw, int mode) {
	if (!memcmp (pathname, "shm://", 6)) {
		RIOShm *shm = R_NEW (RIOShm);
		const char *ptr = pathname+6;
		shm->id = getshmid (ptr);
		shm->buf = shmat (shm->id, 0, 0);
		shm->fd = getshmfd (shm);
		shm->size = SHMATSZ;
		if (shm->fd != -1) {
			eprintf ("Connected to shared memory 0x%08x\n", shm->id);
			return r_io_desc_new (&r_io_plugin_shm, shm->fd, pathname, rw, mode, shm);
		}
		eprintf ("Cannot connect to shared memory (%d)\n", shm->id);
		free (shm);
	}
	return NULL;
}

static int shm__init(RIO *io) {
	return R_TRUE;
}

struct r_io_plugin_t r_io_plugin_shm = {
        //void *plugin;
	.name = "shm",
        .desc = "shared memory resources (shm://key)",
        .open = shm__open,
        .close = shm__close,
	.read = shm__read,
        .plugin_open = shm__plugin_open,
	.lseek = shm__lseek,
	.system = NULL, // shm__system,
	.init = shm__init,
	.write = shm__write,
};

#else
struct r_io_plugin_t r_io_plugin_shm = {
        //void *plugin;
	.name = "shm",
        .desc = "shared memory resources (not for w32)",
};
#endif

#ifndef CORELIB
struct r_lib_struct_t radare_plugin = {
	.type = R_LIB_TYPE_IO,
	.data = &r_io_plugin_shm
};
#endif
