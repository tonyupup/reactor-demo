#include "epollApi.h"
#include "rkernel.h"

bool Epoll::addEvent(int fd, FileEvent *stats, int mask)
{
    if (fd < 0)
        return false;
    struct kevent e = {0};
    // struct  e = {0};
    //查询已有状态，如果为R_None 则op为ADD 否则 MOD
    // int op = stats->mask == R_NONE ? EVFIL : EV;

    // mask |= stats->mask; //merge mask
    // e.data.fd = fd;
    // e.events = 0;
    if (mask & R_READABLE)
    {
        EV_SET(&e, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
        if (kevent(this->epfd, &e, 1, NULL, 0, NULL) == -1)
            return false;
    }
    if (mask & R_WRITABLE)
    {
        EV_SET(&e, fd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
        if (kevent(this->epfd, &e, 1, NULL, 0, NULL) == -1)
            return false;
    }
    return true;
}
void Epoll::delEvent(int fd, FileEvent *stats, int delmask)
{
    struct kevent ke = {0};
    // e.data.fd = fd;
    // e.events = 0;
    // int mask = stats->mask & (~delmask);
    if (delmask & R_READABLE)
    {
        EV_SET(&ke, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
        kevent(this->epfd, &ke, 1, NULL, 0, NULL);
    }
    if (delmask & R_WRITABLE)
    {
        EV_SET(&ke, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
        kevent(this->epfd, &ke, 1, NULL, 0, NULL);
    }
}

int Epoll::select(int maxfd, FiredEvent *fired, struct timeval *tvp)
{
    int retval = 0;
    if (tvp != NULL)
    {
        struct timespec timeout;
        timeout.tv_sec = tvp->tv_sec;
        timeout.tv_nsec = tvp->tv_usec * 1000;
        retval = kevent(epfd, NULL, 0, this->events->data(), MaxEventSize,
                        &timeout);
    }
    else
    {
        retval = kevent(epfd, NULL, 0, this->events->data(), MaxEventSize,
                        NULL);
    }
    if (retval > 0)
    {
        int j = 0;
        for (; j < retval; j++)
        {
            int mask = 0;
            struct kevent e = events->at(j);
            if (e.filter  == EVFILT_READ)
                mask |= R_READABLE;
            if (e.filter ==EVFILT_WRITE)
                mask |= R_WRITABLE;
            fired[j].fd = e.ident;
            fired[j].mask = mask;
        }
    }
    return retval;
}