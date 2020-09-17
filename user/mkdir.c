#include "libc.h"

int main(int argc, char** argv) {   
    // printf("argv 0 %s\n", argv[0]); 
    // printf("argv 1 %s\n", argv[1]); 
    //0 - filename, 1 - directory
    createDirectory(argv[0], argv[1]);
    exit(0);
}
