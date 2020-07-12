#ifndef __SERVER_H
#define __SERVER_H
#include <memory>
#include <iostream>
#include <vector>
#include <sys/unistd.h>

#include "rkernel.h"
#include "client.h"

using namespace std;

class ServerHandle
{
public:
    ServerHandle();
    void Accept(int mask);
    void FirstRead(int mask);
    bool Start();
    Client &findClient(int fd);
    void Stop()
    {
        close(fd);
        kernel->stop();
    };
    const int mfd() const { return fd; };

private:
    int fd = 0;
    shared_ptr<Rkernel> kernel;
    shared_ptr<vector<shared_ptr<Client>>> Clients;
};

#endif