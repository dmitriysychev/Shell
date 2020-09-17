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
char history[1000];
int historyLength = 0;
char currentDirectory [100];
char currentDirectoryIndex = 1;

int main(int argc, char** argv) {
    
shell:

    currentDirectory[0] = '/';
     
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



    for (int i = 0; i < strlen(myBuf[0]); i++) {
        history[historyLength + i] = myBuf[0][i];
    }
    historyLength = historyLength + strlen(myBuf[0]);
    history[historyLength] = '\n';
    historyLength++;


    if (!streq("shell", str)) {
        int id = fork();
        if (id == 0) {
            //printf("child id is %d\n", id);
            int rc = execl("/sbin/shell","a","b","c",0);
            printf("*** came back, rc = %d\n",rc);
        } else {
             uint32_t status;
             wait(id,&status);
            // printf("*** In parent\n");
        }
    } else if (!streq("exit", str)) {
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
         for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 256; j++) {
                myBuf[i][j] = 0;
            }
         }
        printf("\n");
        goto shell;
    } else if (streq("pwd", myBuf[0]) == 0) {
        printf("%s \n", currentDirectory);
         for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 256; j++) {
                myBuf[i][j] = 0;
            }
         }
        goto shell;
     } else if (streq("history", myBuf[0]) == 0) {
        printf("%s", history);
         for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 256; j++) {
                myBuf[i][j] = 0;
            }
         }
        goto shell;
    }  else if (streq("cd", myBuf[0]) == 0) {


        // .. functionality
        if (myBuf[1][0] == '.' && myBuf[1][1] == '.') {
            int slashCount = 0;
            for (int i = 0; i < currentDirectoryIndex; i++) {
                if (currentDirectory[i + 0] == '/') {
                    slashCount++;
                }
            }


            int tempDirectoryIndex = 0;
            int currentSlashCount = 0;
            for (int i = 0; i < currentDirectoryIndex; i++) {
                if (currentDirectory[i + 0] == '/') {
                    currentSlashCount++;
                }

                if (currentSlashCount >= slashCount) {
                    currentDirectory[i] = 0;
                } else {
                    tempDirectoryIndex++;
                }
            }

            currentDirectory[0] = '/';
            currentDirectoryIndex = tempDirectoryIndex;

        // end .. functionality


        // Drill down functionality
        } else {
            for (int i = 0; i < strlen(myBuf[1]); i++) {

                if (i == 0 && currentDirectoryIndex != 1) {
                    currentDirectory[currentDirectoryIndex + i] = '/';
                    currentDirectoryIndex ++;
                }

                currentDirectory[currentDirectoryIndex + i] = myBuf[1][i];
            }
            currentDirectoryIndex = currentDirectoryIndex + strlen(myBuf[1]);
        }

         for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 256; j++) {
                myBuf[i][j] = 0;
            }
         }
        goto shell;
    }
    
    int id = fork();
    if (id == 0) {
        if (streq(myBuf[0], "cat") == 0) {

            if (myBuf[1][0] == '/' || !currentDirectory[1]) {
                execl("/sbin/blah",myBuf[1], myBuf[2], myBuf[3], 0);
            } else {

                char tempBuf[100];
                tempBuf[0] = '/';
                int tempBufSize = 1;
                for (int i = 1; i < strlen(myBuf[1]) + 1; i++) {
                    tempBuf[i] = myBuf[1][i - 1];
                    tempBufSize++;
                }

                char tempDirectory[100];
                int tempDirectorySize = 0;
                for (int i = 0; i < currentDirectoryIndex; i++) {
                    tempDirectory[i] = currentDirectory[i];
                    tempDirectorySize++;
                }

                for (int i = 0; i < tempBufSize; i++) {
                    tempDirectory[tempDirectorySize + i] = tempBuf[i];
                }
                tempDirectorySize += tempBufSize;

                for (int i = 0; i < tempDirectorySize; i++) {
                    myBuf[1][i] = tempDirectory[i];
                }

                execl("/sbin/blah",myBuf[1], myBuf[2], myBuf[3], 0);
            }


            execl("/sbin/blah",myBuf[1], myBuf[2], myBuf[3], 0);
        }
        else if (streq(myBuf[0], "ls") == 0) {

            if (myBuf[1][0] == '/') {
                execl("/sbin/list",myBuf[1], myBuf[2], myBuf[3], 0);
            } else {

                char tempBuf[100];
                int tempBufSize = 0;
                for (int i = 0; i < strlen(myBuf[1]); i++) {
                    tempBuf[i] = myBuf[1][i];
                    tempBufSize++;
                }

                char tempDirectory[100];
                int tempDirectorySize = 0;
                for (int i = 0; i < currentDirectoryIndex; i++) {
                    tempDirectory[i] = currentDirectory[i];
                    tempDirectorySize++;
                }

                for (int i = 0; i < tempBufSize; i++) {
                    tempDirectory[tempDirectorySize + i] = tempBuf[i];
                }
                tempDirectorySize += tempBufSize;

                for (int i = 0; i < tempDirectorySize; i++) {
                    myBuf[1][i] = tempDirectory[i];
                }


                execl("/sbin/list",myBuf[1], myBuf[2], myBuf[3], 0);
            }
        }
        
        else if(streq(myBuf[0], "mkdir") == 0) {
            
            char mkdirStr[20][20];
            int dirIdx = 0;
            char slashDelim[] = "/";
            char *p = strtok(myBuf[1], slashDelim);

           // parser for dir path with slash

            while(p != 0) {
                //printf("%s\n", p);
                memcpy(mkdirStr[dirIdx], p, strlen(p));
                dirIdx++;
                p = strtok(0, slashDelim);
            }
             
           // printf("%s %s\n", mkdirStr[1], mkdirStr[0]);
            if (streq(mkdirStr[1], "") == 0) {
                //printf("If case %s\n", result);
                execl(result, mkdirStr[0],0);
                printf("Exec failed\n");
                // createDirectory(mkdirStr[0],0);
            }
            else {
                //printf("Else case %s\n", result);
                // printf("mkdirStr[1] = %s %s\n", mkdirStr[1], mkdirStr[0]);
                execl(result, mkdirStr[1],mkdirStr[0],0);

            }

            for (int i = 0; i < 20; i++) {
                for (int j = 0; j < 20; j++) {
                   mkdirStr[i][j] = 0;
                }
            }

        } else if(streq(myBuf[0], "touch") == 0) {
            char touchStr[20][20];
            int touchIdx = 0;
            char slashDelim[] = "/";
            char *p = strtok(myBuf[1], slashDelim);

           // parser for dir path with slash

            while(p != 0) {
                //printf("%s\n", p);
                memcpy(touchStr[touchIdx], p, strlen(p));
                touchIdx++;
                p = strtok(0, slashDelim);
            }
            if (streq(touchStr[1], "") == 0) {
                //printf("If case %s\n", result);
                execl(result, touchStr[0],0);
                printf("Exec failed\n");
                // createDirectory(mkdirStr[0],0);
            }
            else {
                //printf("Else case %s\n", result);
                // printf("mkdirStr[1] = %s %s\n", mkdirStr[1], mkdirStr[0]);
                execl(result, touchStr[1],touchStr[0],0);

            }
        }
        else {
            execl(result,myBuf[1], myBuf[2], myBuf[3], 0);
        } 
        
        printf("Unknown command: %s\n", myBuf[0]);
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
    return 0;
}
