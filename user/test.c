#include "libc.h"

/**
 * Follow the same format for command line:
 * streq (<cmd>, buf)
 * if strings are equal, execl("sbin/<cmd>")
*/

char* result;
char* buf;
char* exe;
char myBuf[256][256];
char* temp;

int main(int argc, char** argv) {
shell:
     
    printf("shell@firefly:$ ");
    buf = (char*) malloc(256);
    for (int i = 0; i < strlen(buf); i++) {
        printf("%c ", buf[i]);
    }
    readln(buf);
    buf[strlen(buf)] = '\0';
    char* str = buf;
    char delim[] = " ";
   
    int idx = 0;

    char *ptr = strtok(str, delim);
    while(ptr != 0) {
        //printf("%s\n", ptr);
        memcpy(myBuf[idx], ptr, strlen(ptr));
        idx++;
        ptr = strtok(0, delim);
    }
    result = "";
    exe = (char*) malloc(6);
    memcpy(exe, "/sbin/\0", 8);
    //printf("n = %s\n", exe);
    temp = myBuf[0];
    result = strcat(exe,temp);
    //printf("n = %s\n", myBuf[0]);
    if (!streq("exit", myBuf[0])) {
        exit(0);
    }else if (streq("clear",  myBuf[0]) == 0) {
        for (int i = 0; i < 50; i++) {
            printf("\n");
        }
        for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
             myBuf[i][j] = 0;
        }
       
    }
        goto shell;
    }else if (streq("echo", myBuf[0]) == 0) {
        for (int i = 0; i < strlen(myBuf[0]); i++) {
            printf("%s ", myBuf[i+1]);
        }
        printf("\n");
        goto shell;
    }
    
    int id = fork();
    if (id == 0) {
        if (streq(myBuf[0], "cat") == 0) {
            execl("/sbin/blah",myBuf[1], myBuf[2], myBuf[3], 0);
        }
        else if (streq(myBuf[0], "ls") == 0) {
            execl("/sbin/list",myBuf[1], myBuf[2], myBuf[3], 0);
        }
        else {
            execl(result,myBuf[1], myBuf[2], myBuf[3], 0);
        }
        
        printf("Uknown command: %s\n", exe);
    } else {
        uint32_t status;
        wait(id,&status);
        //  printf("I am in parent now\n");
    }
        for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 256; j++) {
                myBuf[i][j] = 0;
            }
        
        }
    free(exe);
    goto shell;
    // #include "libc.h"

// /**
//  * Follow the same format for command line:
//  * streq (<cmd>, buf)
//  * if strings are equal, execl("sbin/<cmd>")
// */


// int main(int argc, char** argv) {
//     //printf("***This should be printed as I am testing it\n");
    
// shell:
//     printf("shell@firefly:$ ");
//     char buf[256];
//     readln(buf);
//     buf[strlen(buf)] = '\0';
//     char* str = buf;
//     char* sbin = "/sbin/\0";
//     //printf("n = %s\n", str);
//     strcat(sbin, str);
//     //printf("Resulting string is %s\n\n", sbin);
//     if (!streq("\n", str)){
//         goto shell;
//     }
//     if (!streq("shell", str)) {
//         int id = fork();
//         if (id == 0) {
//             //printf("child id is %d\n", id);
//             int rc = execl("/sbin/test","a","b","c",0);
//             printf("*** came back, rc = %d\n",rc);
//         } else {
//              uint32_t status;
//              wait(id,&status);
//             // printf("*** In parent\n");
//         }
        
//     } else if (!streq("exit", str)) {
//         exit(0);
//     } else if (!streq("help", str)) {
//         printf("\n### FireFly shell version 1.0 (x86-i386 qemu based)###\n");
//         printf("exit: exits the current shell process\n");
//         printf("clear: clears the screen\n");
//         printf("shell: runs a child shell\n\n");
//         goto shell;
//     } else if (streq("clear", str) == 0) {
//         for (int i = 0; i < 50; i++) {
//             printf("\n");
//         }
//         goto shell;
//     }else if (streq("echo", str) == 0) {
//         int test = streq("echo", str);
//         printf("%d\n", test);
//         goto shell;
//     }else{
//         printf("%s: command not found\n", str);
//     }
//     goto shell;
    
//     return 0;
// }

    
    return 0;
}
