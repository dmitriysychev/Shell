#include "libc.h"

int putchar(int c) {
    char t = (char)c;
    return write(1,&t,1);
}

int puts(const char* p) {
    char c;
    int count = 0;
    while ((c = *p++) != 0) {
        int n = putchar(c); 
        if (n < 0) return n;
        count ++;
    }
    putchar('\n');
    
    return count+1;
}

int streq (const char* str1, const char* str2) {
	//your code goes here
	int i;
	for (i = 0; (str1[i] != '\0' || str1[i] != 101) && str2[i] != '\0'; i++) {
        //printf("%c\n", str2[i]);
		if (str1[i] != str2[i])
			return str1[i] - str2[i];
	}
    if (str1[i] == '\0') {
        //printf("%d this case\n", str1[i]);
        return (str2[i] == '\0') ? 0 : (str1[i] - str2[i]);
    }
	else if (str2[i] == '\0')
      //  printf("%d\n", str1[i]);
		return (str1[i] == '\0') ? 0 : (str1[i] == 101 ? 0 : (str1[i] - str2[i]));
    return 0;
}
int strlen(const char* src) {
    size_t i;
    for (i = 0; src[i] != '\0'; i++);
    return i;
}

char *strcat(char *dest, const char *src) {
    // char* result = dest;
    // dest+=strlen(dest);
    // while(*src != '\0'){
    //     *dest = *src; //changed dest to results
    //     dest++;
    //     src++;
    // }
    // *dest = '\0';

    // return result;
    char *ret = dest;
    while (*dest)
        dest++;
    while ((*dest++ = *src++))
        ;
    return ret;
}

char *strchr(const char*s, int c){
    while (*s != (char)c) {
        if (!*s++) {
            return 0;
        }
    }
    return (char*)s;
}

size_t strspn(const char *s1, const char *s2) {
    size_t ret = 0;
    while(*s1 && strchr(s2, *s1++))
        ret++;
    return ret;
}

size_t strcspn(const char *s1, const char *s2) {
    size_t ret = 0;
    while(*s1) {
        if (strchr(s2, *s1)) {
            return ret;
        }
        else {
            s1++;
            ret++;
        }
    }
    return ret;
}

char *strtok(char* str, const char* delim) {
    static char* p = 0;
    if (str)
        p = str;
    else if (!p) {
        return 0;
    }
    str=p+strspn(p, delim);
    p=str+strcspn(str, delim);
    if(p==str) 
        return p=0;
    p = *p ? *p = 0, p+1 : 0;
    return str;
}


void cp(int from, int to) {
    while (1) {
        char buf[100];
        ssize_t n = read(from,buf,100);
        if (n == 0) break;
        if (n < 0) {
            printf("*** %s:%d read error, fd = %d\n",__FILE__,__LINE__,from);
            break;
        }
        char *ptr = buf;
        while (n > 0) {
            ssize_t m = write(to,ptr,n);
            if (m < 0) {
                printf("*** %s:%d write error, fd = %d\n",__FILE__,__LINE__,to);
                break;
            }
            n -= m;
            ptr += m;
        }
    }
    // function to help with input stream
// int getchar(void) {
//     char c;
//     size_t n = read(0, c, 1);
//     return c;
// }

// int gets(const char* p) {
//     char c;
//     int count = 0;
//     while ((c = *p++) != 13) // 13 == enter kbd 
//     {  
//         int n = getchar();
//         if (n < 0) return n;
//         count++;
//     }
//     return count;
// }
}
