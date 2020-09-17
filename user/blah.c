// Actually a cat function 

#include "libc.h"

int main(int argc, char** argv) {
   int fd = open(argv[0],0);
    int length = len(fd);
    char buf[length];
    read(fd,buf,length);
    printf("%s \n", buf);
    exit(0);
}