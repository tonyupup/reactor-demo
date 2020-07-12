#include "client.h"
#include "server.h"
#include <unistd.h>

Client::operator int() const
{
    return this->_fd;
}
bool Client::addEvent(void (Client::*func)(int), int mask)
{
    if (shared_ptr<Rkernel> p = kptr.lock())
    {
        return p->createEvent(_fd, mask, bind(func, this, placeholders::_1));
    }
    return false;
}
bool Client::delEvent(int mask)
{
    if (shared_ptr<Rkernel> p = kptr.lock())
    {
        p->deleteEvent(_fd, mask);
        return true;
    }
    return false;
}
void Client::firstRead(int fd)
{
    delEvent(R_READABLE);
    char buf[10] = {0};
    read(fd, buf, 10);
    cout << buf;
    addEvent(&Client::onMessage, R_READABLE);
}
void Client::onMessage(int fd)
{
    char x[10] = {0};
    int n = NetworkHelper::anetRead(fd, x, 10);
    if (!n && (errno & EAGAIN))
    {
        delEvent(R_READABLE);
        cclose();
    }
    cout << x << endl;
}
void Client::cclose()
{
    cout << "Client" << *this << "Closed" << endl;
    close(this->_fd);
}