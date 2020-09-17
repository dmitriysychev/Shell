#include "sys.h"
#include "stdint.h"
#include "idt.h"
#include "debug.h"
#include "machine.h"
#include "semaphore.h"
#include "libk.h"
#include "kernel.h"
#include "bobfs.h"
#include "elf.h"
#include "threads.h"
#include "future.h"
#include "vmm.h"

// added uart
#include "u8250.h"

// Program takes 2 arugments.  Hello and goodbye
// WHen starts running, stack should look like:
    //-Top of stack: return address
    /**
     * Top of stack: return address
     * next: numerical value of 2 (for two arguments)
     *
     *
     *
     *
     *
     *  Main: takes 2 arguments (arg c which is an int, char ** argv which is an array of an array of characters)
     *  So next needs to be a pointer to a pointer of characters (a pointer to the beginning of an array of strings)
     *  That means it has to point to be a pointer to a pointer of characters (the next X number of spaces on the stack.  this array ends in 0)
     *
     *  Every pointer is 4 bytes wide
     *
     *
     *  How do we construct this?
     */


int getAvailableFile() {
    Thread* thread = active();

    for (int i = 0; i < 100; i++) {
        if (thread->fdt[i].opened == 0) {
            return i;
        }
    }
    return 0;
}

int open (char *fn) {
    BobFS* fs = getFs();
    Node* file = BobFS::root(fs);

    Thread* thread = active();
    if (thread->fdt[0].opened == 0) {
        thread->fdt[0].opened = 1;
        thread->fdt[0].offset = 0;
        thread->fdt[0].node = file->newFile("stdin");

        thread->fdt[1].opened = 1;
        thread->fdt[1].offset = 0;
        thread->fdt[1].node = file->newFile("stdout");

        thread->fdt[2].opened = 1;
        thread->fdt[2].offset = 0;
        thread->fdt[2].node = file->newFile("stderr");

        for (int i = 3; i < 100; i++) {
            thread->fdt[i].opened = 0;
        }
    }

    int charIndex = 1;
    int slashIndex = 2;
    int maxLen = K::strlen(fn);

    while (charIndex < maxLen) {
        while (slashIndex < maxLen && fn[slashIndex] != '/') {
            slashIndex++;
        }

        char arr[slashIndex - charIndex + 1];
        memcpy(arr, fn + charIndex, slashIndex - charIndex);
        arr[slashIndex - charIndex] = '\0';
        //Debug::printf("Found file name %s\n", arr);
        slashIndex++;
        charIndex = slashIndex;
        file = file->findNode(arr);
    }

    int fileIndex = getAvailableFile();
    thread->fdt[fileIndex].node = file;
    thread->fdt[fileIndex].offset = 0;
    thread->fdt[fileIndex].opened = 1;

    return fileIndex;
}

extern "C" int sysHandler(uint32_t eax, uint32_t *frame) {
    switch (eax) {
        case 0: {
            // Exit
            /**
             * TODO delete stuff
             */
            Thread* thread = active();
            uint32_t *userStack = (uint32_t *) frame[3];
            int exitCode = userStack[1];

            thread->future->set(exitCode);
            stop();
            return -1;
        } break;
        case 1: {
            // Write
            uint32_t *userStack = (uint32_t *) frame[3];

            int fd = (int) userStack[1];
            char *buffer = (char *) userStack[2];
            uint32_t bytesToWrite = userStack[3];

            uint32_t bytesWritten = bytesToWrite;
            Thread* thread = active();
             if (fd <= 3) {
                 for (uint32_t i = 0; i < bytesToWrite; i++) {
                     Debug::printf("%c", buffer[i]);
                 }
                 bytesWritten = bytesToWrite;
             } else {
                Node* file = thread->fdt[fd].node;
                bytesWritten = file->writeAll(thread->fdt[fd].offset, buffer, bytesToWrite);
             }
            thread->fdt[fd].offset = thread->fdt[fd].offset + bytesWritten;
            return bytesWritten;
        } break;
        case 2: {
            // Fork

            // eip is at index 0
            // eip is pointing at the return address
            uint32_t eip = frame[0];
            uint32_t userStack = frame[3];



            // Steps:
            // 1. Create a thread
            // 2. Pass in esp, eip, and eax values into the new thread
            // 3. Within the the thread, change those values to the values that were passed in
            // 4. Copy the virtual address spaces.  Shared (shallow copy) Private (deep copy)
            // 5. Copy the file descriptor table + the semaphores table
            // 6. Switch to user mode?? Return 0??







            // How to get esp, eip, eax, and what do we assign them to?
            // How to copy virtual address spaces?
            // Call switchToUser at the end?
            // Do we need to copy the stack?
            // What's in the child descriptor table?  List of PIDs and what else?

            // For wait: Should we create a semaphore here, pass it to the thread, and if wait is called then we
            // lock the thread for the parent and unlock it in the child?

            Thread* parentThread = active();

            /**
             * TODO get rid of child tables
             * This is probably broken
             */
             int childId = 1;
            for (int i = 1; i < 100; i++) {
                if (parentThread->childTable[i].connected == 0) {
                    parentThread->childTable[i].connected = 1;
                    childId = i;
                }
            }

            // Create a semaphore
            // Assign a child id
            Semaphore semaphore { 0 };


            thread([userStack, parentThread, &semaphore, eip, childId]{

                Thread* thisThread = active();

                // Copy the file descriptor table and the semaphore table
                for (int i = 0; i < 100; i++) {
                    thisThread->fdt[i] = parentThread->fdt[i];
                    thisThread->semaphoreTable[i] = parentThread->semaphoreTable[i];
                }

                uint32_t *pd = parentThread->addressSpace->pd;


                // Create a new future and point to it from the parent thread
                auto future = new Future<int>;

                thisThread->future = future;
                parentThread->childTable[childId].future = future;

                // Loop through every PDI
                // Then loop through each PTI
                // Then allocate frames for each
                for (int i0 = 512; i0 < 960; i0++) {
                    uint32_t pde = pd[i0];

                    if (pde & 1) { // Need to check the present bit or is this sufficient?

                        uint32_t* pt = (uint32_t *) (pde & 0xfffff000);

                        for (uint32_t i1 = 0; i1 < 1024; i1++) {
                            uint32_t pte = pt[i1];

                            if (pte & 1) { // Same thing.  Present bit?
                                uint32_t pa = pte & 0xfffff000;
                                memcpy((void*)(i0 << 22 | i1 << 12), (void*)pa, 4096);
                            }
                        }
                    }
                }

                semaphore.up();

                // switchToUser this is why we need the esp, eip, and eax.  eax is 0
                // function params: (uint32_t pc, uint32_t esp, uint32_t eax);
                switchToUser(eip, userStack, 0);
            });

            // Semaphore down here
            semaphore.down();
            return childId;
        } break;
        case 3: {
            // Semaphore
            Thread* thread = active();
            uint32_t *userStack = (uint32_t *) frame[3];
            uint32_t initialValue = (uint32_t) userStack[1];

            for (int i = 0; i < 100; i++) {
                if (thread->semaphoreTable[i].initialized == 0) {
                    thread->semaphoreTable[i].semaphore = new Semaphore(initialValue);
                    thread->semaphoreTable[i].initialized = 1;
                    return i;
                }
            }

            return 0;
        }
        case 4: {
            // Up
            Thread* thread = active();
            uint32_t *userStack = (uint32_t *) frame[3];
            uint32_t semaphoreId = (uint32_t) userStack[1];
            if (thread->semaphoreTable[semaphoreId].initialized == 1) {
                thread->semaphoreTable[semaphoreId].semaphore->up();
                return 0;
            }

            Debug::panic("5");
            return -1;
        }
        case 5: {
            // Down
            Thread* thread = active();
            uint32_t *userStack = (uint32_t *) frame[3];
            uint32_t semaphoreId = (uint32_t) userStack[1];
            if (thread->semaphoreTable[semaphoreId].initialized == 1) {
                thread->semaphoreTable[semaphoreId].semaphore->down();
                return 0;
            }

            return -1;
        }
        case 6: {
            // Close
            uint32_t *userStack = (uint32_t *) frame[3];
            int fd = (int) userStack[1];
            Thread* thread = active();
            thread->fdt[fd].opened = 0;

            /**
             * TODO what should this return?
             * TODO semaphores and children
             */
            return 0;
        } break;
        case 7: {
            // Shutdown
            Debug::shutdown();
        } break;
        case 8: {
            // Wait

            uint32_t *userStack = (uint32_t *) frame[3];
            int childId = userStack[1];
            uint32_t* buffer = (uint32_t*) userStack[2];

            Thread* thread = active();
            *buffer = thread->childTable[childId].future->get();
            /**
             * TODO error checking
             */
            return (int)0;
        } break;
        case 9: {
            // Execl
            // Position 0 is the return address
            // Position 1 is the file location
            // Position 2 to n is the argumenets
            // Last arg is always 0
            // Count length + number of them and then copy them into kernel space and then copy back after you
            // swap the address space

            // Each arg is a pointer to an array of characters
            // Use memcopy


            // Need to get the file node, swap the address spaces, and then elf load that file
            // Erasing the old data is a security insurance

            // Have a copy of everything in kernel space


            // Swap the entire address space
            // autoTemp = me() -> addressSpace
            // currentThread addressSpace = new AddressSpace
            // Then you have to activate the address space (function in AddressSpace class)
            // Delete temp

            // Elf load - pass it a Node*

            // Need to put everything back onto the stack
            // Bottom of stack is thread private space (0xeffffff)
            // Call switch to user at the end of this
            uint32_t *userStack = (uint32_t *) frame[3];
            char* fileLocation = (char *) userStack[1];

            BobFS* fs = getFs();
            Node* file = BobFS::root(fs);

            int i = 1;
            int count = 0;
            char name[256];
            bzero(name, 256);

            while (fileLocation[i] != 0) {
                if (fileLocation[i] != '/') {
                    name[count] = fileLocation[i];
                    count++;
                } else {
                    if (fileLocation[i+1] == 0) {
                        return -1;
                    }

                    count = 0;
                    file = file->findNode(name);
                    if (file == nullptr) {
                        return -1; // couldn't find file/directory
                    }
                    bzero(name, count+1);
                    count = 0;
                }
                i++;
            }

            file = file->findNode(name);
            if (file == nullptr) {
                return -1;
            }

            uint32_t* esp = (uint32_t*) frame[3];

            // Make better
            int argCount = 0;
            while (esp[argCount+2]) {
                argCount++;
            }
            char* allArgs[argCount+1];
            int counts[argCount];
            for (int i = 0; i < argCount; i++) {
                char* arg = (char*) esp[i+2];

                if ((uint32_t) arg < 0x80000000) {
                    return -1;
                }
                counts[i] = 0;
                while (arg[counts[i]]) {
                    counts[i]++;
                }
                int newIndex = counts[i] + 1;
                allArgs[i] = new char[newIndex];
                memcpy( (void *) allArgs[i], arg, newIndex);
            }
            allArgs[argCount] = nullptr;

            Thread* thread = active();
            thread->addressSpace = new AddressSpace();
            thread->addressSpace->activate();

            uint32_t e = ELF::load(file);
            int isElf[1] = {0};
            file->read(0, (void*) isElf, 4);
            if (e < 0x80000000 || e >= 0xf0000000 || isElf[0] != 0x464c457f) {
                return -1;
            }


            uint32_t start = 0xeffff000;
            uint32_t offset = 0;
            uint32_t* argC = (uint32_t*) start;
            *argC = argCount;


            offset = offset + 4;
            char*** results = (char***)(start + offset);
            *results = (char**)(((uint32_t)results) + 4);
            char** argV = *results;
            offset= offset + (4*(argCount+2));
            for (int i = 0; i < argCount; i++) {
                char* arg = (char*) allArgs[i];
                argV[i] = (char*)(start+offset);
                memcpy((void*) (start+offset), arg, counts[i]+1);
                offset += counts[i] + 1;
            }

            switchToUser(e, 0xeffff000, 0);
        } break;
        case 10: {
            // Open
            uint32_t *userStack = (uint32_t *) frame[3];
            char *fn = (char *) userStack[1];
            int fileIndex = open(fn);
            return fileIndex;
        } break;
        case 11: {
            // Len
            uint32_t *userStack = (uint32_t *) frame[3];

            int fd = (int) userStack[1];
            Thread* thread = active();
            Node* node = thread->fdt[fd].node;

            if (thread->fdt[fd].opened != 1) {
                return -1;
            }

            return node->getSize();
        } break;
        case 12: {
            // Read
            uint32_t *userStack = (uint32_t *) frame[3];

            int fd = (int) userStack[1];
            char *buffer = (char *) userStack[2];
            uint32_t bytesToRead = userStack[3];
            Thread* thread = active();

            if (thread->fdt[fd].opened != 1) {
                return -1;
            }


            Node* file = thread->fdt[fd].node;
            uint32_t bytesRead = file->readAll(thread->fdt[fd].offset, buffer, bytesToRead);
            thread->fdt[fd].offset = thread->fdt[fd].offset + bytesRead;
            return bytesRead;
        }
        case 13: {
            // Seek
            uint32_t *userStack = (uint32_t *) frame[3];

            int fd = (int) userStack[1];
            uint32_t newOffset = (uint32_t) userStack[2];
            Thread* thread = active();
            thread->fdt[fd].offset = newOffset;

            return newOffset;
        } break;
        case 14: {
            //readline for shell to take input
            // readln (void* buf)
            uint32_t *userStack = (uint32_t *) frame[3];
            char* buffer = (char*) userStack[1];
            U8250 uart;
            int res = uart.instr(buffer, 256);
            // new line character
            if (res < 0) {
                *buffer = 10;
                buffer++;
                while(*buffer != 0) {
                    *buffer = 0;
                }
            }
            uart.put(10);
            return 1;
        } break;
        case 15: {
            // list for shell to read button inputs
            uint32_t *userStack = (uint32_t *) frame[3];
            char* fn = (char *) userStack[1];

            BobFS* fs = getFs();
            Node* file = BobFS::root(fs);

            int charIndex = 1;
            int slashIndex = 2;
            int maxLen = K::strlen(fn); // /etc = 4 chars

            while (charIndex < maxLen) {
                while (slashIndex < maxLen && fn[slashIndex] != '/') {
                    slashIndex++;
                }

                char arr[slashIndex - charIndex + 1];
                memcpy(arr, fn + charIndex, slashIndex - charIndex);
                arr[slashIndex - charIndex] = '\0';
                //Debug::printf("Found file name %s\n", arr);
                slashIndex++;
                charIndex = slashIndex;
                file = file->findNode(arr);
            }
            file->ls();
            return 1;

        } break;
        // } break;

        case 16: {
            //creates new file, touch command 
            uint32_t *userStack = (uint32_t*) frame[3]; 
            char* fname = (char*) userStack[1]; 
            char* target = (char*) userStack[2];
            BobFS* fs = getFs(); 
            Node* file = BobFS::root(fs);
            if (target != 0) {
                //Debug::printf("We are in target %s   %s\n", target, fname);
                file = file->findNode(target);
                file->newFile(fname);
            } else {
                file->newFile(fname);
            }
            return 1; 
        } 

        case 17: {
            //creates new directory, make directory command 
            uint32_t *userStack = (uint32_t*) frame[3]; 
            char* fname = (char*) userStack[1];
            char* target = (char*) userStack[2];
            BobFS* fs = getFs(); 
            Node* file = BobFS::root(fs);
            if (target != 0) {
                //Debug::printf("We are in target %s   %s\n", target, fname);
                file = file->findNode(target);
                file->newDirectory(fname);
            } else {
                file->newDirectory(fname);
            }

            return 1; 
        } break;
        default: {
            Debug::printf("\nunknown system call %d", eax);
        } break;
    }

    return 1;
}

void SYS::init(void) {
    IDT::trap(48,(uint32_t)sysHandler_,3);
}
