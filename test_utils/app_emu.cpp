#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
int main(void) {
    int fd = open("/tmp/fifo1in", O_RDONLY),size;
    char buf[1450];
    if(fd == -1) {
        perror("fd");
        return 0;
    }
    while(true) {
        size = read(fd, buf, sizeof(buf));
        if(size == 0) {
            puts("size = 0, wait...");
            sleep(1);
        }
        buf[size] = 0;
        printf("size:%d\n", size);
        puts(buf);
    }
}
