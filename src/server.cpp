#include "server.h"
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include "rkernel.h"
#include "client.h"
#include "networking.h"
#include <regex>
ServerHandle::ServerHandle(Processer *p)
{
    this->process = p;
    this->Clients = make_shared<vector<shared_ptr<Client>>>(1024);
    this->kernel = make_shared<Rkernel>(1024);
    addrinfo hints, *servinfo, *p;
    char _port[6];
    int s = -1, rv;
    for (int i = 1920; i < 65535; i++)
    {
        snprintf(_port, 6, "%d", i);
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET6;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        // this->fd = socket(, SOCK_STREAM, 0);
        if ((rv = getaddrinfo(nullptr, _port, &hints, &servinfo)) != 0)
        {
            cout << "getaddinfo error" << endl;
            // throw "getaddinfo error";
            continue;
        }
        for (p = servinfo; p != NULL; p = p->ai_next)
        {
            if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
                continue;

            if (NetworkHelper::anetListen(s, p->ai_addr, p->ai_addrlen, 511) == -1)
            {
                s = -1;
                continue;
            }
            fd = s;
            cout << i << endl;
            goto out;
        }
    }
out:
    NetworkHelper::setNoblock(this->fd, true);
    freeaddrinfo(servinfo);
    this->kernel->createEvent(this->fd, R_READABLE, bind(&ServerHandle::Accept, this, placeholders::_1));
}
void ServerHandle::Stop()
{
    close(fd);
    kernel->stop();
};
shared_ptr<Client> &ServerHandle::findClient(int fd)
{
    return this->Clients->at(fd);
}

bool ServerHandle::dropClient(int fd)
{
    shared_ptr<Client> pc;
    if (nullptr != (pc = Clients->at(fd)))
    {
        (*Clients)[fd] = nullptr;
        return true;
    }
    return false;
}
bool ServerHandle::Start()
{
    if (this->kernel != nullptr)
        this->kernel->mainLoop();
    return false;
}

void ServerHandle::Accept(int fd)
{

    auto c = make_shared<ChatClient>(kernel);
    if (-1 == (*c = NetworkHelper::anetTcpAccept(fd, (char *)c->remoteIp.ip, 64, &(c->remoteIp.port))))
    {
        cout << "Accept error" << endl;
        return;
    }
    cout << *c << endl;
    NetworkHelper::setNoblock(*c, true);
    Clients->at(*c) = c; //may be renew
    kernel->createEvent(*c, R_READABLE, bind(&Client::onMessage, Clients->at(*c), placeholders::_1));
}
