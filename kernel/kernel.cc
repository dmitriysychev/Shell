#include "debug.h"
#include "ide.h"
#include "bobfs.h"
#include "elf.h"
#include "machine.h"

BobFS* globalFs;

Node* checkFile(const char* name, Node* node) {
    if (node == nullptr) {
        Debug::panic("*** 0 %s is null\n",name);
    }
    if (node->isDirectory()) {
        Debug::panic("*** 0 %s is a directory\n",name);
    }
    //Debug::printf("*** 0 file %s is ok\n",name);
    return node;
}

Node* getFile(Node* node, const char* name) {
    return checkFile(name,node->findNode(name));
}

Node* checkDir(const char* name, Node* node) {
    if (node == nullptr) {
        Debug::panic("*** 0 %s is null\n",name);
    }
    if (!node->isDirectory()) {
        Debug::panic("*** 0 %s is not a directory\n",name);
    }
    //Debug::printf("*** 0 directory %s is ok\n",name);
    return node;
}

Node* getDir(Node* node, const char* name) {
    return checkDir(name,node->findNode(name));
}

BobFS* getFs() {
    return globalFs;
}

void kernelMain(void) {
    Ide* d = new Ide(3);
    auto fs = BobFS::mount(d);
    auto root = BobFS::root(fs);
    auto sbin = getDir(root,"sbin");
    auto init = getFile(sbin,"init");
    globalFs = fs;

    uint32_t e = ELF::load(init);
    //Debug::printf("%s:%d: %x\n", __FILE__, __LINE__, e);
    //Debug::panic("About to run\n");
    switchToUser(e,0xeffff000,0);

    Debug::panic("*** 1000000000 implement switchToUser in machine.S\n");
}

