#include "libc.h"

int main(int argc, char** argv) {   
    //printf("%s\n", argv[0]); 
    //char* name = argv[0]; 
    createFile(argv[0], argv[1]);
    exit(0);
}