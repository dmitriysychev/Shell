#ifndef _MACHINE_H_
#define _MACHINE_H_

#include "stdint.h"
#include "config.h"

extern "C" void resetEIP(void);

extern "C" int inb(int port);
extern "C" int inl(int port);
extern "C" void outb(int port, int val);
extern "C" void outl(int port, int val);

extern "C" uint64_t rdmsr(uint32_t id);
extern "C" void wrmsr(uint32_t id, uint64_t value);

extern "C" void vmm_on(uint32_t pd);
extern "C" void invlpg(uint32_t va);

extern "C" void apitHandler_(void);
extern "C" void spuriousHandler_(void);
extern "C" void pageFaultHandler_(void);
extern "C" void sysHandler_(void);

extern "C" uint32_t cli(void);
extern "C" uint32_t sti(void);
extern "C" uint32_t getFlags();
extern "C" uint32_t restore(uint32_t flags);

extern "C" void* memcpy(void *dest, const void* src, size_t n);
extern "C" void* bzero(void *dest, size_t n);

extern bool disable();
extern void enable(bool wasDisabled);
extern "C" void switchToUser(uint32_t pc, uint32_t esp, uint32_t eax);

extern "C" [[noreturn]] void switchToUser(uint32_t pc, uint32_t esp, uint32_t eax);

extern "C" void ltr(uint32_t);

extern uint32_t tssDescriptorBase;
extern long kernelSS;

#endif
