#ifndef __CHATROOM_H
#define __CHATROOM_H
#include "server.h"
#include "client.h"
#include "memory"
#include <map>
#include <vector>
#include <forward_list>

enum ChatOperator
{
    LOGIN = 1,
    LOGOUT,
    LISTALL,
    SENDMSG
};
// class ServerHandle:
// class Client;

// template <> bool operator==(weak_ptr<Client> &p1,const weak_ptr<Client> &p2)
// {
//     return true;
// }

class Chat : public Processer
{
public:
    Chat() : server(make_shared<ServerHandle>()), cid(0), clients(1024){};
    Chat(shared_ptr<ServerHandle> s) : server(s), cid(0), clients(1024) {}

    virtual void Parse(shared_ptr<Client>) override;
    virtual void ConnectHandle(int fd) override;
    void OnMessage(int fd);
    void Start() noexcept;

private:
    shared_ptr<ServerHandle> server;
    vector<shared_ptr<Client>> clients;
    forward_list<weak_ptr<Client>> online;
    int cid;
};

#endif