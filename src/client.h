#ifndef __CLIENT_H
#define __CLIENT_H

#include <iostream>
#include <memory>

using namespace std;

class Rkernel;
class Client
{
public:
    struct RemoteIP
    {
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
    void cclose();
    void onMessage(int fd);
    operator int() const;
    friend ostream &operator<<(ostream &cout, const Client &c)
    {
        cout << "FD:";
        cout.setf(ios::right);
        cout.width(4);
        cout<< c._fd;
        cout.unsetf(ios::right);
        cout <<",RemoteIP(" << c.remoteIp.ip << ":" << c.remoteIp.port << ")";
        return cout;
    }

private:
    int _fd;
    weak_ptr<Rkernel> kptr;
};

#endif