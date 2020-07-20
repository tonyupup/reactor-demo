#include "client.h"
#include "server.h"
#include <unistd.h>
#include "networking.h"
#include "rkernel.h"

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
void Client::onMessage(int fd)
{
    char x[10] = {0};
    int n = NetworkHelper::anetRead(fd, x, 10);
    if (!n && ((errno & EAGAIN) || !errno))
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