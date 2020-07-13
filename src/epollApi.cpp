#include "epollApi.h"
#include "rkernel.h"

bool Epoll::addEvent(int fd, FileEvent *stats, int mask)
{
    if (fd < 0)
        return false;
    struct epoll_event e = {0};
    //查询已有状态，如果为R_None 则op为ADD 否则 MOD
    int op = stats->mask == R_NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    mask |= stats->mask; //merge mask
    e.data.fd = fd;
    e.events = 0;
    if (mask & R_READABLE)
        e.events |= EPOLLIN;
    if (mask & R_WRITABLE)
        e.events |= EPOLLOUT;

    if (-1 == epoll_ctl(epfd, op, fd, &e))
    {
        return false;
    }
    return true;
}
void Epoll::delEvent(int fd, FileEvent *stats, int delmask)
{
    struct epoll_event e = {0};
    e.data.fd = fd;
    e.events = 0;
    int mask = stats->mask & (~delmask);
    if (mask & R_READABLE)
        e.events |= EPOLLIN;
    if (mask & R_WRITABLE)
        e.events |= EPOLLOUT;
    if (mask != R_NONE)
    {
        epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &e);
    }
    else
    {
        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &e);
    }
}

int Epoll::select(int maxfd, FiredEvent *fired, struct timeval *tv)
{
    int ret = epoll_wait(epfd, events->data(), maxfd, tv ? (tv->tv_sec * 1000 + tv->tv_usec / 1000) : -1);
    if (ret > 0)
    {
        int j = 0;
        for (; j < ret; j++)
        {
            int mask = 0;
            struct epoll_event e = events->at(j);
            if (e.events & EPOLLIN)
                mask |= R_READABLE;
            if (e.events & EPOLLOUT)
                mask |= R_WRITABLE;
            if (e.events & EPOLLERR)
                mask |= R_READABLE | R_WRITABLE;
            if (e.events & EPOLLHUP)
                mask |= R_READABLE | R_WRITABLE;
            fired[j].fd = e.data.fd;
            fired[j].mask = e.events;
        }
    }
    return ret;
}