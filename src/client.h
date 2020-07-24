#ifndef __CLIENT_H
#define __CLIENT_H

#include <iostream>
#include <memory>
#include <exception>
#include <cstring>

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
    virtual ~Client() = default;
    bool addEvent(void (Client::*p)(int), int mask);
    bool delEvent(int mask);
    void cclose();
    virtual void onMessage(int fd);
    operator int() const;
    friend ostream &operator<<(ostream &cout, const Client &c)
    {
        cout << "FD:";
        cout.setf(ios::right);
        cout.width(4);
        cout << c._fd;
        cout.unsetf(ios::right);
        cout << ",RemoteIP(" << c.remoteIp.ip << ":" << c.remoteIp.port << ")";
        return cout;
    }

private:
    int _fd;
    weak_ptr<Rkernel> kptr;
};

class MClient : public Client
{
public:
    using Client::operator=;

    explicit MClient(shared_ptr<Rkernel> &pk) : Client(pk) //默认构造
    {
        if (nullptr == (buf = new uint8_t[1024]))
        {
            throw bad_alloc();
        }
    };

    MClient(const MClient &c) : Client(c), buf(new uint8_t[1024]) //复制构造函数
    {
        memcpy(buf, c.buf, 1024);
    }
    MClient &operator=(const MClient &c) = delete; //赋值运算符

    MClient(MClient &&c) : Client(move(c)), buf(c.buf) //移动构造函数
    {
        c.buf = nullptr;
    }
    MClient &operator=(MClient &&c) //移动复制运算符
    {
        Client(move(c));
        buf = c.buf;
        c.buf = nullptr;
        return *this;
    }
    virtual void onMessage(int fd) override;
    virtual ~MClient() override
    {
        if (buf != nullptr)
        {
            delete[] buf;
        }
    }
    bool auth(int fd);

private:
    uint8_t *buf;
};
#endif