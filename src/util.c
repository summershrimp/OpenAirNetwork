#include <fcntl.h>
#include <errno.h>

int an_make_nonblock(int fd) {
    if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0)|O_NONBLOCK) == -1) {
        return errno;
    }
    return 0;
}

