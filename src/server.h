#ifndef __SERVER_H
#define __SERVER_H
#include <memory>
#include <iostream>
#include <vector>
#include <sys/unistd.h>

using namespace std;
class Rkernel;
class Client;
class Chat;

class Processer
{
public:
    virtual void Parse(shared_ptr<Client>) = 0;
    virtual void ConnectHandle(int fd) = 0;
};
class ServerHandle
{
    friend Chat;

public:
    ServerHandle();
    // void Accept(int mask);
    bool Start(Processer *);
    void Stop();

private:
    int fd = 0;
    shared_ptr<Rkernel> kernel;
};

#endif