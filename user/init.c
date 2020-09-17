#include "libc.h"

void one(int fd) {
    printf("*** fd = %d\n",fd);
    printf("*** len = %d\n",len(fd));

    cp(fd,2);
}

int main(int argc, char** argv) {

    int id = fork();

    if (id < 0) {
        printf("fork failed");
    } else if (id == 0) {
        /* child */
        printf("*** in child\n");
        //char[2]= ['a', '\0' ]
        int rc = execl("/sbin/shell","a","b","c",0);
        printf("*** execl failed, rc = %d\n",rc);
    } else {
        /* parent */
        uint32_t status;
        wait(id,&status);
        printf("*** Returned back to init.c, see you later!\n");
    }

    shutdown();
    return 0;
}
