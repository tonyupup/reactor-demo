#ifndef __CHATROOM_H
#define __CHATROOM_H
#include "server.h"
#include "client.h"
#include "memory"
#include <map>
#include <vector>

enum ChatOperator
{
    LOGIN = 1,
    LOGOUT,
    LISTALL,
    SENDMSG
};
// class ServerHandle:
// class Client;

class Chat : public Processer
{
public:
    virtual void Parse(Client &c) override;
    Chat(shared_ptr<ServerHandle> s) : server(s), cid(0), Clients(1024) {}

private:
    weak_ptr<ServerHandle> server;
    vector<weak_ptr<Client>> Clients;
    int cid;
};

#endif