#ifndef __CLIENT_H
#define __CLIENT_H

#include <iostream>
#include <memory>
#include <exception>
#include <cstring>
using namespace std;

#include "rkernel.h"
//class Rkernel;
class Chat;
class Client
{
    friend Chat;

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

    template <typename _T>
    bool addEvent(void (_T::*func)(int), int mask)
    {
        if (shared_ptr<Rkernel> p = kptr.lock())
        {
            return p->createEvent(_fd, mask,
                                  bind(func, dynamic_cast<typename remove_reference<_T>::type *>(this), placeholders::_1));
        }
        return false;
    }
    bool delEvent(int mask);
    void cclose();
    virtual void onMessage(int fd);
    virtual void onWriteable(int fd)
    {
        return;
    };
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
    bool authed = false;
    weak_ptr<Rkernel> kptr;
};

class ChatClient : public Client
{
    friend Chat;

public:
    using Client::operator=;

    explicit ChatClient(shared_ptr<Rkernel> &pk) : Client(pk), rbufsize(1024), wbufsize(1024) //默认构造
    {
        if (nullptr == (rbuf = new uint8_t[rbufsize]))
        {
            throw bad_alloc();
        }
        if (nullptr == (wbuf = new uint8_t[wbufsize]))
        {
            throw bad_alloc();
        }
    };

    ChatClient(const ChatClient &c) = delete; // : Client(c), buf(new uint8_t[1024]) //复制构造函数
    // {
    //     memcpy(buf, c.buf, 1024);
    // }
    ChatClient &operator=(const ChatClient &c) = delete; //赋值运算符

    ChatClient(ChatClient &&c) : Client(move(c)) //移动构造函数
    {
        wbufsize = c.wbufsize;
        rbufsize = c.rbufsize;
        windex = c.windex;
        rindex = c.rindex;

        if (rbuf != nullptr)
        {
            delete[] rbuf;
        }
        if (wbuf != nullptr)
        {
            delete[] wbuf;
        }

        rbuf = c.rbuf;
        wbuf = c.wbuf;

        c.rbuf = nullptr;
        c.wbuf = nullptr;
    }
    ChatClient &operator=(ChatClient &&c) //移动复制运算符
    {
        Client(move(c));
        if (rbuf != nullptr)
        {
            delete[] rbuf;
        }
        if (wbuf != nullptr)
        {
            delete[] wbuf;
        }
        rbuf = c.rbuf;
        wbuf = c.wbuf;

        c.wbuf = nullptr;
        c.rbuf = nullptr;

        wbufsize = c.wbufsize;
        rbufsize = c.rbufsize;

        windex = c.windex;
        rindex = c.rindex;

        return *this;
    }
    virtual void onMessage(int fd) override;
    virtual void onWriteable(int fd) override;

    void Writeable(int fd) noexcept
    {
        return;
    }
    bool adata2wbuf(const uint8_t *, ssize_t n);
    bool adata2rbuf(const uint8_t *, ssize_t n);
    inline void clearwbuf() noexcept
    {
        this->windex = 0;
    }
    inline void clearrbuf() noexcept
    {
        this->rindex = 0;
    }
    virtual ~ChatClient() override
    {
        if (rbuf != nullptr)
        {
            delete[] rbuf;
        }
        if (wbuf != nullptr)
            delete[] wbuf;
    }
    bool auth(int fd);

private:
    uint8_t *rbuf, *wbuf;
    ssize_t wbufsize, rbufsize;
    ssize_t rindex, windex;
    uint32_t currOp;
    char account[20];
};
#endif