#include "rkernel.h"
Rkernel::Rkernel(int maxSize)
{
    this->eventapi = make_shared<Epoll>(maxSize);
    this->events = make_shared<vector<FileEvent>>(maxSize);
    this->fevents = make_shared<vector<FiredEvent>>(maxSize);
    this->setsize = maxSize;
}
bool Rkernel::createEvent(int fd, int mask, function<void(int)> proc)
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
            e->rfileProc = proc; //proc;
        if (mask & R_WRITABLE)
            e->wfileProc = proc;
        // e->data = data;

        if (fd > maxfd)
            maxfd = fd;
        return true;
    }
    return false;
}
void Rkernel::deleteEvent(int fd, int delmask)
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
}

int Rkernel::processEvents()
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
}
int Rkernel::mainLoop()
{
    while (1)
        processEvents();
}