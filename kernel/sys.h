#ifndef _SYS_H_
#define _SYS_H_

#include "bobfs.h"
#include "semaphore.h"
#include "future.h"

class SYS {
public:
    static void init(void);
};

struct OpenFile {
    int offset;
    Node* node;
    int opened;
};

struct OpenSemaphore {
    bool initialized;
    Semaphore* semaphore;
};

struct OpenChildren {
    bool connected;
    Future<int>* future;
};

#endif
