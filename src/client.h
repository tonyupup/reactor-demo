#ifndef __CLIENT_H
#define __CLIENT_H

#include <iostream>
#include "rkernel.h"
#include <unistd.h>

using namespace std;

class Client
{
public:
    struct RemoteIP
    {
        /* data */
        char ip[64];
        int port;
    } remoteIp;

    explicit Client(shared_ptr<Rkernel> &pl) : _fd(0), kptr(pl){};
    Client &operator=(int fd)
    {
        _fd = fd;
        return *this;
    };
    bool addEvent(void (Client::*p)(int), int mask);
    bool delEvent(int mask);
    void firstRead(int fd);
    void cclose();
    void onMessage(int fd);
    operator int() const;
    friend ostream &operator<<(ostream &cout, const Client &c)
    {
        cout << "Fd:" << c._fd << ",RemoteIP(" << c.remoteIp.ip << ":" << c.remoteIp.port << ")";
        return cout;
    }

private:
    int _fd;
    weak_ptr<Rkernel> kptr;
};

#endif