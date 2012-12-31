/* Double-buffering cp.
 * To be used only on files
 */

#include <aio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "yield.h"

#define BUF_SIZE (32 * 1024)

static struct mydata {
	char *path_src;
	char *path_dst;
	int fildes_src;
	int fildes_dst;
	long total_bytes;
	long read_bytes;
	long next_request;
} _tmp_data;

void buffer_worker(void *unused_arg)
{
	char buffer[BUF_SIZE];
	off_t real_read_bytes;
	struct aiocb aio;

	do {

		// First part: read
		aio.aio_fildes = _tmp_data.fildes_src;
		aio.aio_offset = _tmp_data.next_request;
		aio.aio_buf = buffer;
		aio.aio_nbytes = BUF_SIZE;
#ifdef _POSIX_PRIORITIZED_IO
		aio.aio_reqprio = 0;
#endif
		aio.aio_sigevent.sigev_notify = SIGEV_NONE;
		_tmp_data.next_request += BUF_SIZE;

		aio_read(&aio);
		yield();

		while(EINPROGRESS == aio_error(&aio)) {
			yield();
		}

		if(aio_error(&aio)) {
			perror("aio_error");
		}

		real_read_bytes = aio_return(&aio);
		_tmp_data.read_bytes += real_read_bytes;
		aio.aio_fildes = _tmp_data.fildes_dst;
		aio.aio_nbytes = real_read_bytes;

		aio_write(&aio);
		yield();

		while(EINPROGRESS == aio_error(&aio)) {
			yield();
		}

		if(aio_error(&aio)) {
			perror("aio_error");
		}

	} while (_tmp_data.read_bytes < _tmp_data.total_bytes);

	printf("Got to end!\nFinishing thread.\n");
}

void copy_double_buffered(char *src, char *dst)
{
	struct stat s;

	_tmp_data.path_src = src;
	_tmp_data.path_dst = dst;
	_tmp_data.read_bytes = 0L;
	_tmp_data.next_request = 0L;

	if(-1 == (_tmp_data.fildes_src = open(src, O_RDONLY))) {
		perror("open src");
		exit(0);
	}
	if(-1 == (_tmp_data.fildes_dst = open(dst, O_WRONLY|O_CREAT, 0664))) {
		perror("open dst");
		close(_tmp_data.fildes_src);
		exit(0);
	}
	fstat(_tmp_data.fildes_src, &s);

	_tmp_data.total_bytes = s.st_size;

	sthread_init(2);
	sthread_func(1, buffer_worker, NULL);
	sthread_func(2, buffer_worker, NULL);
	sthread_start();

	printf("FINISHED. Total bytes copied: %ld\n", _tmp_data.read_bytes);

	close(_tmp_data.fildes_src);
	close(_tmp_data.fildes_dst);
}

int check_args(int argc, char *argv[])
{
	struct stat s;
	int code, i;

	if(argc < 3) {
		return 0;
	}

	for(i = 1; i < 3; i++) {
		code = stat(argv[i], &s);
		if(code == -1) {
			perror("stat");
			return 0;
		}

		if(!S_ISREG(s.st_mode))
			return 0;
	}
	return 1;
}

void usage(char *arg)
{
	printf("Usage: %s <source> <destination>.\nBoth must be files.\n", arg);
}

int main(int argc, char *argv[])
{
	if(!check_args(argc, argv))
	{
		usage(argv[0]);
		exit(0);
	}

	copy_double_buffered(argv[1], argv[2]);
}

