#ifndef __EPOLLAPI_H
#define __EPOLLAPI_H

#include <sys/epoll.h>
#include <time.h>
#include <malloc.h>
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
        epfd = epoll_create(maxSize);
        events = make_shared<vector<epoll_event>>(maxSize);
    }
    bool addEvent(int fd, FileEvent *, int mask);
    void delEvent(int fd, FileEvent *, int delmask);
    int select(int maxfd, FiredEvent *, struct timeval *tv);
    void stopEpoll() { close(epfd); };

private:
    int epfd;
    // unique_ptr<int> epfd;
    shared_ptr<vector<epoll_event>> events;
    // struct epoll_event *events = nullptr;
};

#endif