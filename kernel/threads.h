#ifndef _threads_h_
#define _threads_h_

#include "atomic.h"
#include "queue.h"
#include "heap.h"
#include "sys.h"
#include "future.h"

extern void threadsInit();

class AddressSpace;

class Thread {
public:
    Thread* next = nullptr;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t leaveMeAlone = 0;
    OpenFile fdt[100];
    OpenSemaphore semaphoreTable[100];
    OpenChildren childTable[100];
    Future<int> futureTable[100];
    AddressSpace *addressSpace;
    Future<int> *future;
    virtual void start() = 0;
    virtual uint32_t interruptEsp() = 0;
    Thread();
    virtual ~Thread() {};
};

template <typename T>
class ThreadImpl : public Thread {
    T work;
public:
    long stack[2048];
    virtual void start() override {
        work();
    }
    virtual uint32_t interruptEsp() override {
        return (uint32_t) &stack[2046];
    }
    ThreadImpl(T work) : work(work) {}
    virtual ~ThreadImpl() {
    }
};

extern void stop();

extern void entry();

extern void yield();
extern void block(Thread*);
extern void stop();
extern Thread* active();
extern void schedule(Thread* t);
extern Thread* doNotDisturb();
extern void reaper();

template <typename T>
void thread(T work) {
    reaper();
    auto thread = new ThreadImpl<T>(work);
    long *topOfStack = &thread->stack[2045];
    //if ((((unsigned long) topOfStack) % 16) == 8) topOfStack --;
    topOfStack[0] = 0x200;       // sti
    topOfStack[1] = 0;           // cr2
    topOfStack[2] = (long)entry;
    thread->esp = (long) topOfStack;
    schedule(thread);
}

class Disable {
    bool was;
public:
    Disable() : was(disable()) {}
    ~Disable() { enable(was); }
};

#endif
