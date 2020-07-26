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
    virtual void Parse(Client &) = 0;
};
class ServerHandle
{
    friend Chat;

public:
    ServerHandle(Processer *);
    void Accept(int mask);
    void FirstRead(int mask);
    bool Start();
    shared_ptr<Client> &findClient(int fd);
    void Stop();
    bool dropClient(int fd);

    const int mfd() const { return fd; };

private:
    int fd = 0;
    Processer *process;
    shared_ptr<Rkernel> kernel;
    shared_ptr<vector<shared_ptr<Client>>> Clients;
};


#endif