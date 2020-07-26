#include <iostream>
#include "src/rkernel.h"
#include <string>
#include <cstring>
#include "src/networking.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <netdb.h>

using namespace std;
class MClient
{
public:
    MClient(const char *username, const char *passwd) : name(username), passwd(passwd)
    {
        addrinfo hints, *servinfo, *p;
        char _port[6];
        int s = -1, rv;

        // snprintf(_port, 6, "%d", i);
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET6;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        // this->fd = socket(, SOCK_STREAM, 0);
        if ((rv = getaddrinfo("localhost", "1920", &hints, &servinfo)) != 0)
        {
            cout << "getaddinfo error" << endl;
            return;
        }
        for (p = servinfo; p != NULL; p = p->ai_next)
        {
            if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
                continue;
            if (-1==connect(s,p->ai_addr,p->ai_addrlen)){
                cout << "Connect failed" << endl;
                return;
            }
            _fd = s; 
            break;
        }
    }

private:
    string name, passwd;
    int _fd;
};


int main()
{
    auto c=MClient("nihao","passs");
}