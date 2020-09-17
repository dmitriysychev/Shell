#include "bobfs.h"
#include "libk.h"
#include "heap.h"
#include "debug.h"

uint8_t zero_1024[BobFS::BLOCK_SIZE];

Bitmap::Bitmap(BobFS* fs, uint32_t offset) :
        fs(fs), offset(offset)
{
}

void Bitmap::clear(void) {
    fs->device->writeAll(offset,zero_1024,BobFS::BLOCK_SIZE);
}

void Bitmap::set(int32_t index) {
    uint32_t wordOffset = (index/32)*4;
    uint32_t bitIndex = index%32;

    uint32_t word;

    fs->device->readAll(offset+wordOffset,&word,sizeof(word));

    word |= (1 << bitIndex);

    fs->device->writeAll(offset+wordOffset,&word,sizeof(word));
}

void Bitmap::clear(int32_t index) {
    uint32_t wordOffset = (index/32) * 4;
    uint32_t bitIndex = index%32;

    uint32_t word;

    fs->device->readAll(offset+wordOffset,&word,sizeof(word));

    word &= ~(1 << bitIndex);

    fs->device->writeAll(offset+wordOffset,&word,sizeof(word));
}

int32_t Bitmap::find(void) {
    for (uint32_t i=0; i<BobFS::BLOCK_SIZE; i += 4) {
        uint32_t word;
        fs->device->readAll(offset+i,&word,sizeof(word));
        if ((~word) != 0) {
            uint32_t mask = 1;
            for (uint32_t j=0; j<32; j++) {
                if ((word & mask) == 0) {
                    word |= mask;
                    fs->device->writeAll(offset+i,&word,sizeof(word));
                    return i * 8 + j;
                }
                mask = mask * 2;
            }
        }
    }
    return -1;
}







//////////
// Node //
//////////

Node::Node(BobFS* fs, uint32_t inumber) {
    this->fs = fs;
    this->inumber = inumber;
    offset = fs->inodesBase + inumber * SIZE;
}

uint16_t Node::getType(void) {
    uint16_t x;
    fs->device->readAll(offset+0, &x, 2);
    return x;
}

bool Node::isDirectory(void) {
    return getType() == DIR_TYPE;
}

bool Node::isFile(void) {
    return getType() == FILE_TYPE;
}

void Node::setType(uint16_t type) {
    fs->device->writeAll(offset+0, &type, 2);
}

void Node::setSize(uint32_t size) {
    fs->device->writeAll(offset+4, &size, 4);
}

bool streq(const char* a, const char* b) {
    int i = 0;

    while (true) {
        char x = a[i];
        char y = b[i];
        if (x != y) return false;
        if (x == 0) return true;
        i++;
    }
}

uint32_t Node::getDirect(void) {
    uint32_t x;
    fs->device->readAll(offset+8, &x, 4);
    return x;
}

void Node::setDirect(uint32_t direct) {
    fs->device->writeAll(offset+8, &direct, 4);
}

uint32_t Node::getIndirect(void) {
    uint32_t x;
    fs->device->readAll(offset+12, &x, 4);
    return x;
}

void Node::setIndirect(uint32_t indirect) {
    fs->device->writeAll(offset+12, &indirect, 4);
}

Node* Node::findNode(const char* name) {


    uint32_t sz = getSize();
    uint32_t offset = 0;

    while (offset < sz) {
        uint32_t ichild;
        readAll(offset,&ichild,4);
        offset += 4;
        uint32_t len;
        readAll(offset,&len,4);
        offset += 4;
        char* ptr = (char*) malloc(len+1);
        readAll(offset,ptr,len);
        offset += len;
        ptr[len] = 0;

        auto cmp = streq(name,ptr);
        free(ptr);

        if (cmp) {
            Node* child = Node::get(fs,ichild);
            return child;
        }
    }

    return nullptr;
}

uint16_t Node::getLinks(void) {
    uint16_t x;
    fs->device->readAll(offset+2, &x, 2);
    return x;
}

uint32_t Node::getSize(void) {
    uint32_t x;
    fs->device->readAll(offset+4, &x, 4);
    return x;
}

/**
 * LS FUNCTIONALITY
 */
void Node::ls() {
    uint32_t sz = getSize();
    uint32_t offset = 0;

    while (offset < sz) {
        uint32_t ichild;
        readAll(offset,&ichild,4);
        offset += 4;
        uint32_t len;
        readAll(offset,&len,4);
        offset += 4;
        char* ptr = (char*) malloc(len+1);
        readAll(offset,ptr,len);
        offset += len;
        ptr[len] = 0;

        Node* child = Node::get(fs,ichild);
        if (child->isDirectory()) {
            Debug::printf("Directory: ");
        } else {
            Debug::printf("File: ");
        }

        for (uint32_t i = 0; i < len; i++) {
            char test = ptr[i];
            Debug::printf("%c", test);
        }
        Debug::printf("\n");
    }
}


uint32_t Node::getBlockNumber(uint32_t blockIndex) {
    if (blockIndex == 0) {
        uint32_t x = getDirect();
        if (x == 0) {
            x = fs->allocateBlock();
            setDirect(x);
        }
        return x;
    } else {
        blockIndex -= 1;
        if (blockIndex >= BobFS::BLOCK_SIZE/4) return 0;
        uint32_t i = getIndirect();
        if (i == 0) {
            i = fs->allocateBlock();
            if (i == 0) return 0;
            setIndirect(i);
        }
        uint32_t x;
        const uint32_t xOffset = i * BobFS::BLOCK_SIZE + blockIndex*4;
        fs->device->readAll(xOffset,&x,sizeof(x));
        if (x == 0) {
            x = fs->allocateBlock();
            fs->device->writeAll(xOffset,&x,sizeof(x));
        }
        return x;
    }
}

int32_t Node::write(uint32_t offset, const void* buffer, uint32_t n) {
    uint32_t blockIndex = offset / BobFS::BLOCK_SIZE;
    uint32_t start = offset % BobFS::BLOCK_SIZE;
    uint32_t end = start + n;
    if (end > BobFS::BLOCK_SIZE) end = BobFS::BLOCK_SIZE;
    uint32_t count = end - start;

    uint32_t blockNumber = getBlockNumber(blockIndex);

    fs->device->writeAll(
            blockNumber*BobFS::BLOCK_SIZE+start,
            buffer,
            count);

    uint32_t newSize = offset + count;
    uint32_t oldSize = getSize();
    if (newSize > oldSize) {
        setSize(newSize);
    }
    return count;
}

int32_t Node::writeAll(uint32_t offset, const void* buffer_, uint32_t n) {
    char* buffer = (char*) buffer_;
    uint32_t bytesWritten = 0;
    while (n > 0) {
        int32_t cnt = write(offset,buffer,n);
        if (cnt <= 0) {
            Debug::printf("%s:%d failed to write\n",__FILE__,__LINE__);
            Debug::shutdown();
        }

        bytesWritten += cnt;
        n -= cnt;
        offset += cnt;
        buffer += cnt;
    }

    return bytesWritten;
}

int32_t Node::read(uint32_t offset, void* buffer, uint32_t n) {
    uint32_t sz = getSize();

    if (sz <= offset) {
        // Having this print statement in here causes us to fail t0.  I have no fucking idea why.
        //Debug::printf("\n size < offset %d %d", sz, offset);
        return 0;
    }

    uint32_t remaining = sz - offset;
    if (remaining < n) n = remaining;

    uint32_t blockIndex = offset / BobFS::BLOCK_SIZE;
    uint32_t start = offset % BobFS::BLOCK_SIZE;
    uint32_t end = start + n;
    if (end > BobFS::BLOCK_SIZE) end = BobFS::BLOCK_SIZE;
    uint32_t count = end - start;

    uint32_t blockNumber = getBlockNumber(blockIndex);

    return fs->device->read(blockNumber*BobFS::BLOCK_SIZE+start, buffer, count);
}

int32_t Node::readAll(uint32_t offset, void* buffer_, uint32_t n) {

    char* buffer = (char*) buffer_;
    uint32_t bytesRead = 0;
    uint32_t cnt = 0;

    while (n > 0) {
        int32_t cnt = read(offset,buffer,n);
        bytesRead += cnt;
        if (cnt < 0) {
            Debug::printf("%s:%d failed to read\n",__FILE__,__LINE__);
            Debug::shutdown();
        }
        if (cnt == 0) {
            return bytesRead;
        }

        n -= cnt;
        offset += cnt;
        buffer += cnt;
    }
    if (bytesRead > cnt) {
        return bytesRead;
    }
    return cnt;
}

void Node::setLinks(uint16_t links) {
    fs->device->writeAll(offset+2, &links, 2);
}

void Node::linkNode(const char* name, Node* node) {
    node->setLinks(1+node->getLinks());
    uint32_t offset = getSize();
    writeAll(offset,&node->inumber,4);
    uint32_t len = strlen(name);
    writeAll(offset+4,&len,sizeof(len));
    writeAll(offset+4+sizeof(len),name,len);
}

Node* Node::newNode(const char* name, uint32_t type) {
    int32_t idx = fs->inodeBitmap->find();
    if (idx < 0) {
        Debug::printf("newNode, out of inodes\n");
    }
    Node* node = Node::get(fs,idx);
    node->setType(type);
    node->setSize(0);
    node->setLinks(0);
    node->setDirect(0);
    node->setIndirect(0);

    linkNode(name,node);
    return node;
}

Node* Node::newFile(const char* name) {
    return newNode(name, 2);
}

Node* Node::newDirectory(const char* name) {
    return newNode(name, 1);
}

void Node::dump(const char* name) {
    uint32_t type = getType();
    switch (type) {
        case DIR_TYPE:
            Debug::printf("*** 0 directory:%s(%d)\n",name,getLinks());
            {
                uint32_t sz = getSize();
                uint32_t offset = 0;

                while (offset < sz) {
                    uint32_t ichild;
                    readAll(offset,&ichild,4);
                    offset += 4;
                    uint32_t len;
                    readAll(offset,&len,4);
                    offset += 4;
                    char* ptr = (char*) malloc(len+1);
                    readAll(offset,ptr,len);
                    offset += len;
                    ptr[len] = 0;

                    Node* child = Node::get(fs,ichild);
                    child->dump(ptr);
                    free(ptr);
                }
            }
            break;
        case FILE_TYPE:
            Debug::printf("*** 0 file:%s(%d,%d)\n",name,getLinks(),getSize());
            break;
        default:
            Debug::panic("unknown i-node type %d\n",type);
    }
}


///////////
// BobFS //
///////////


BobFS::BobFS(Ide* device) {
    this->device = device;
    this->datablockBitmapBase = 1 * BLOCK_SIZE;
    this->inodeBitmapBase = 2 * BLOCK_SIZE;
    this->inodesBase = 3 * BLOCK_SIZE;
    this->datablocksBase = 3 * BLOCK_SIZE + Node::SIZE * BLOCK_SIZE * 8;
    this->dataBitmapBase = BLOCK_SIZE;
    dataBitmap = new Bitmap(this,dataBitmapBase);
    inodeBitmap = new Bitmap(this,inodeBitmapBase);
}

uint32_t BobFS::allocateBlock(void) {
    int32_t index = dataBitmap->find();
    if (index == -1) {
        Debug::printf("failed to allocate block\n");
    }
    uint32_t blockIndex = dataBase / BLOCK_SIZE + index;
    device->writeAll(blockIndex * BLOCK_SIZE, zero_1024, BLOCK_SIZE);
    return blockIndex;
}

BobFS::~BobFS() {
    delete dataBitmap;
    delete inodeBitmap;
}

Node* BobFS::root(BobFS* fs) {
    uint32_t rootiNodePointer;
    fs->device->readAll(8, &rootiNodePointer, 4);
    Node* newNode = new Node(fs, rootiNodePointer);
    newNode->setLinks(1);
    return newNode;
}

BobFS* BobFS::mount(Ide* device) {
    return new BobFS(device);
}


BobFS* BobFS::mkfs(Ide* device) {
    BobFS* fs = new BobFS(device);
    fs->currentiNodeNumber = 1;
    fs->currentBlockNumber = 132;

    /**
     * TODO write BOBFS to first 4 bytes
     */
    uint32_t firstPointer = 1;
    fs->device->write(8, &firstPointer, 4);

    // Define the root directory
    uint8_t type = 1;
    fs->device->write(fs->inodesBase + (fs->currentiNodeNumber * 16), &type, 2);

    uint8_t nLinks = 0;
    fs->device->write(fs->inodesBase + (fs->currentiNodeNumber * 16) + 2, &nLinks, 2);

    uint32_t size = 0;
    fs->device->write(fs->inodesBase + (fs->currentiNodeNumber * 16) + 4, &size, 4);

    uint32_t directPointer = 0;
    fs->device->write(fs->inodesBase + (fs->currentiNodeNumber * 16) + 8, &directPointer, 4);

    uint32_t indirectPointer = 0;
    fs->device->write(fs->inodesBase + (fs->currentiNodeNumber * 16) + 12, &indirectPointer, 4);

    return fs;
}
