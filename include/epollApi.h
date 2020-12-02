#ifndef __EPOLLAPI_H
#define __EPOLLAPI_H

#include <time.h>
#include <sys/unistd.h>
#include <sys/types.h>
#include <memory>
#include <vector>

#ifdef __APPLE__
#include <sys/event.h>
#else
#include <sys/epoll.h>
#endif

using namespace std;

struct FileEvent;
struct FiredEvent;

class Epoll
{

public:
    explicit Epoll(int maxSize)
    {
#ifdef __APPLE__
        epfd = kqueue();
        if (epfd<0){
            throw "bad create kqueue fd";
        }
        events = make_shared<vector<struct kevent>>(maxSize);
#else
        epfd = epoll_create(maxSize);
        if (epfd<0){
            throw "bad create kqueue fd";
        }
        events = make_shared<vector<struct kevent>>(maxSize);
#endif

    }
    bool addEvent(int fd, FileEvent *, int mask);
    void delEvent(int fd, FileEvent *, int delmask);
    int select(int maxfd, FiredEvent *, struct timeval *tv);
    void stopEpoll() {  };

private:
    int epfd;
    // unique_ptr<int> epfd;
#ifdef __APPLE__
    shared_ptr<vector<struct kevent> > events;
#else
    shared_ptr<vector<struct epoll_event>> events;
#endif
};

#endif