#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
int main(void) {
    int fd_in = open("/tmp/fifo1in", O_RDONLY),size;
    int fd_out = open("/tmp/fifo1out", O_WRONLY);
    char buf[1450];
    if(fd_in == -1) {
        perror("fd");
        return 0;
    }
    write(fd_out,"Test boardcast", 14);
    while(true) {
        size = read(fd_in, buf, sizeof(buf));
        if(size == 0) {
            puts("size = 0, wait...");
            sleep(1);
            continue;
        }
        buf[size] = 0;
        printf("size:%d\n", size);
        puts(buf);
    }
}
