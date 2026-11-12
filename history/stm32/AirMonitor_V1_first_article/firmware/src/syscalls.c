/* No filesystem or console transport is configured on this board.
 * snprintf writes to memory; these explicit stubs reject accidental I/O. */
#include <errno.h>
#include <sys/types.h>
int _close(int fd){(void)fd;errno=EBADF;return -1;}
off_t _lseek(int fd,off_t offset,int whence){(void)fd;(void)offset;(void)whence;errno=ESPIPE;return (off_t)-1;}
ssize_t _read(int fd,void *buffer,size_t length){(void)fd;(void)buffer;(void)length;errno=EBADF;return -1;}
ssize_t _write(int fd,const void *buffer,size_t length){(void)fd;(void)buffer;(void)length;errno=EBADF;return -1;}
