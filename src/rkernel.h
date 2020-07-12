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
class Rkernel
{

public:
    explicit Rkernel(int maxSize = MaxEventSize)
    {
        this->eventapi = make_shared<Epoll>(maxSize);
        this->events = make_shared<vector<FileEvent>>(maxSize);
        this->fevents = make_shared<vector<FiredEvent>>(maxSize);
        this->setsize = maxSize;
    }
    template <typename _CT>
    bool createEvent(int fd, int mask, void (_CT::*proc)(int), _CT *data)
    {
        if (fd >= this->setsize || fd < 0)
        {
            throw "fd error";
        }
        FileEvent *e = &events->at(fd);
        if (eventapi->addEvent(fd, e, mask))
        {
            e->mask |= mask;
            if (mask & R_READABLE)
                e->rfileProc = bind(proc, data, placeholders::_1); //proc;
            if (mask & R_WRITABLE)
                e->wfileProc = bind(proc, data, placeholders::_1);
            // e->data = data;

            if (fd > maxfd)
                maxfd = fd;
            return true;
        }
        return false;
    };
    void deleteEvent(int fd, int delmask)
    {
        if (fd < 0)
            throw "fd error";
        FileEvent *e = events->data() + fd;
        if (e->mask & R_NONE)
            return;
        eventapi->delEvent(fd, e, delmask);
        e->mask &= (~delmask);
        if (fd == maxfd && e->mask == R_NONE)
        {
            for (int j = maxfd - 1; j >= 0; j--)
            {
                if ((*events)[j].mask != R_NONE)
                    break;
                maxfd = j;
            }
        }
    };
    void stop(){};
    int processEvents()
    {
        timeval tv = {10, 0};
        int ret = eventapi->select(maxfd, fevents->data(), &tv);
        if (ret > 0)
        {
            for (int i = 0; i < ret; i++)
            {
                FiredEvent fe = (*fevents)[i];
                FileEvent *e = &(events->at(fe.fd));
                if (fe.mask & R_READABLE)
                    e->rfileProc(fe.fd);
                // bind(e->rfileProc,e->data,placeholders::_1)(R_READABLE);
                if (fe.mask & R_WRITABLE)
                    e->wfileProc(fe.fd);
            }
        }
        return ret;
    };
    int mainLoop()
    {
        while (1)
            processEvents();
    };

private:
    int setsize;
    int maxfd;
    shared_ptr<Epoll> eventapi;
    shared_ptr<vector<FileEvent>> events;
    shared_ptr<vector<FiredEvent>> fevents;
};

//Rkernel::Rkernel(int maxSize)

//     template <class _CT>
//     bool Rkernel::createEvent(int fd, int mask, void (_CT::*proc)(int), _CT *data)
// {

// }
// void Rkernel::deleteEvent(int fd, int delmask)
// {

// }

// int Rkernel::processEvents()
// {
// }
// int Rkernel::mainLoop()
// {

// }

#endif