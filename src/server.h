#ifndef __SERVER_H
#define __SERVER_H
#include <memory>
#include <iostream>
#include <vector>
#include <sys/unistd.h>



using namespace std;
class Rkernel;
class Client;
class ServerHandle
{
public:
    ServerHandle();
    void Accept(int mask);
    void FirstRead(int mask);
    bool Start();
    shared_ptr<Client> findClient(int fd);
    void Stop();
    
    const int mfd() const { return fd; };

private:
    int fd = 0;
    shared_ptr<Rkernel> kernel;
    shared_ptr<vector<shared_ptr<Client>>> Clients;
};

#endif