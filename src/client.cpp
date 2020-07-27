#include "client.h"
#include "server.h"
#include <unistd.h>
#include "networking.h"
#include "rkernel.h"
#include "memory"


Client::operator int() const
{
    return this->_fd;
}
// bool Client::addEvent(void (Client::*func)(int), int mask)
// {
//     if (shared_ptr<Rkernel> p = kptr.lock())
//     {
//         return p->createEvent(_fd, mask, bind(func, this, placeholders::_1));
//     }
//     return false;
// }
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

void ChatClient::onMessage(int fd)
{
    
    // cclose();
}

void ChatClient::onWriteable(int fd)
{
    int sendd = 0;
    while (this->windex != sendd)
    {
        sendd += write(*this, this->wbuf + sendd, this->windex - sendd);
    }
    this->windex = 0;
    delEvent(R_WRITABLE);
}

bool ChatClient::adata2wbuf(const uint8_t *data, ssize_t n)
{
    if (n + this->windex >= this->wbufsize)
    {
        ssize_t afsize = (n + this->windex) * 1.5;
        uint8_t *newbuf;
        if (nullptr == (newbuf = (uint8_t *)realloc(wbuf, afsize)))
            throw bad_alloc();
        else
        {
            this->wbuf = newbuf;
            this->wbufsize = afsize;
        }
    }
    memcpy(this->wbuf + this->windex, data, n);
    this->windex += n;
    return true;
}
bool ChatClient::adata2rbuf(const uint8_t *data, ssize_t n)
{
    if (n + this->rindex >= this->rbufsize)
    {
        ssize_t afsize = (n + this->rindex) * 1.5;
        uint8_t *newbuf;
        if (nullptr == (newbuf = (uint8_t *)realloc(rbuf, afsize)))
            throw bad_alloc();
        else
        {
            this->rbuf = newbuf;
            this->rbufsize = afsize;
        }
    }
    memcpy(this->rbuf + this->rindex, data, n);
    this->rindex += n;
    return true;
}