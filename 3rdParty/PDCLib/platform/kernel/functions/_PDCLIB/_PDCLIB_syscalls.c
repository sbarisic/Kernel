#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "_PDCLIB_glue.h"
#include <errno.h>

// Memory

void * _PDCLIB_allocpages( size_t n )
{
    errno = ENOTSUP;
    return NULL;
}

void _PDCLIB_freepages(void * p, size_t n)
{
	return;
}

// Process

void _PDCLIB_Exit(int status)
{
	errno = ENOTSUP;
}

// File

bool _PDCLIB_open(_PDCLIB_fd_t * pFd, const _PDCLIB_fileops_t ** pOps,
	char const * const filename, unsigned int mode)
{
	errno = ENOTSUP;
	return false;
}


int _PDCLIB_rename(const char * old, const char * new)
{
	errno = ENOTSUP;
	return false;
}

// Stream

static bool readf(_PDCLIB_fd_t self, void * buf, size_t length,
	size_t * numBytesRead)
{
	errno = ENOTSUP;
	return false;
}

static bool writef(_PDCLIB_fd_t self, const void * buf, size_t length,
	size_t * numBytesWritten)
{
	errno = ENOTSUP;
	return false;
}

static bool seekf(_PDCLIB_fd_t self, int_fast64_t offset, int whence,
	int_fast64_t* newPos)
{
	errno = ENOTSUP;
	return false;
}

static void closef(_PDCLIB_fd_t self)
{
	errno = ENOTSUP;
}

const _PDCLIB_fileops_t _PDCLIB_fileops = {
	.read = readf,
	.write = writef,
	.seek = seekf,
	.close = closef,
};