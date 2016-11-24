#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
int an_make_nonblock(int fd) {
    int flags, s;

    flags = fcntl (fd, F_GETFL, 0);
    if (flags == -1)
    {
        perror ("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;
    s = fcntl (fd, F_SETFL, flags);
    if (s == -1)
    {
        perror ("fcntl");
        return -1;
    }

    return 0; 
    return 0;
}

