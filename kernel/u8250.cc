#include "u8250.h"
#include "machine.h"
#include "debug.h"
/* 8250 */
#define CR 0x0D
#define BS 0x08
void U8250::put(char c) {
    while (!(inb(0x3F8+5) & 0x20));
    outb(0x3F8,c);
}

char U8250::get() {
    while (!(inb(0x3F8+5) & 0x01)) {
    }
    char x = inb(0x3F8);
    return x;
}

int U8250::instr(char* bufPt, unsigned short max) {
    int length = 0;
    char ch;
    ch = get();
    if (ch == CR)
        return -1;
    while (ch != CR) {
        if (ch == '\033') { // if the first value is esc
            ch = get(); // skip the [
            switch(ch = get()) { // the real value
                case 'A':
                    // code for arrow up
                   
                    break;
                case 'B':
                    // code for arrow down
                    break;
                case 'C':
                    // code for arrow right
                    break;
                case 'D':
                    // code for arrow left
                    break;
            }
        }
        else if (ch == 127) {
            if (length > 0) {
                bufPt--;
                length--;
                //*bufPt = '\0';
                put('\b');
                put(' ');
                put('\b');
                //put(127);
                // Debug::printf("Length is %d\n", length);
            } 
            
        } else if (length < max) {
            *bufPt = ch;
            bufPt++;
            length++;
            put(ch);
        }
       
        ch = get();
    }
    //put(ch);
    *bufPt = 0;
    return 0;
}
