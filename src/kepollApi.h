#ifndef __KEPOLLAPI_H
#define __KEPOLLAPI_H

#include <sys/event.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include <memory>
#include <vector>

using namespace std;

struct FileEvent;
struct FiredEvent;

class Epoll
{

public:
    explicit Epoll(int maxSize)
    {
        epfd = kqueue();
        events = make_shared<vector<struct kevent>>(maxSize);
    }
    bool addEvent(int fd, FileEvent *, int mask);
    void delEvent(int fd, FileEvent *, int delmask);
    int select(int maxfd, FiredEvent *, struct timeval *tv);
    void stopEpoll() {  };

private:
    int epfd;
    // unique_ptr<int> epfd;
    shared_ptr<vector<struct kevent>> events;
    // struct epoll_event *events = nullptr;
};

#endif
