#ifndef __RKERNEL_H
#define __RKERNEL_H

#include <sys/epoll.h>
#include <memory>
#include <vector>
#include <functional>

#include "epollApi.h"
#include "networking.h"
// typedef void (BaseHandle::*FileProc)(int mask);

using namespace std;

const int MaxEventSize = 1024;

#define R_NONE 0
#define R_READABLE 1
#define R_WRITABLE 2
using EventFunc = function<void(int)>;
struct FileEvent
{
    int mask;
    EventFunc wfileProc;
    EventFunc rfileProc;
    void *data;
};
struct FiredEvent
{
    int fd;
    int mask;
};
class Epoll;
class Rkernel
{

public:
    explicit Rkernel(int maxSize = MaxEventSize);
    bool createEvent(int fd, int mask, function<void(int)>);
    void deleteEvent(int fd, int delmask);
    void stop(){};
    int processEvents();
    int mainLoop();

private:
    int setsize;
    int maxfd;
    shared_ptr<Epoll> eventapi;
    shared_ptr<vector<FileEvent>> events;
    shared_ptr<vector<FiredEvent>> fevents;
};

#endif