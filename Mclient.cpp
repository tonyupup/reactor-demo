#include <iostream>
#include "src/rkernel.h"
#include <string>
#include <cstring>
#include "src/networking.h"
#include "src/server.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <netdb.h>

using namespace std;

const static int BUF_SIZE = 1024;
class MClient
{
    enum ChatOperator
    {
        LOGIN = 1,
        LOGOUT,
        LISTALL,
        SENDMSG
    };

public:
    MClient() : kernelApi(make_shared<Rkernel>()), _fd(-1)
    {
        rbuf = new uint8_t[BUF_SIZE];
        wbuf = new uint8_t[BUF_SIZE];
        inputBuf = new uint8_t[BUF_SIZE];

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
            if (-1 == connect(s, p->ai_addr, p->ai_addrlen))
            {
                cout << "Connect failed" << endl;
                return;
            }
            _fd = s;
            break;
        }
        NetworkHelper::setNoblock(_fd, true);
        NetworkHelper::setNoblock(STDIN_FILENO, true);
        freeaddrinfo(servinfo);
    }
    void Start()
    {

        kernelApi->createEvent(_fd, R_READABLE, bind(&MClient::Parse, this, placeholders::_1));
        kernelApi->createEvent(STDIN_FILENO, R_WRITABLE, bind(&MClient::InputParse, this, placeholders::_1));

        kernelApi->mainLoop();
    }
    void Parse(int fd)
    {
        int i = 0;
        i = NetworkHelper::anetRead(fd, (char *)rbuf, BUF_SIZE);
        cout << rbuf << endl;
    }
    void InputParse(int fd)
    {
        int inputindex = 0;
        int i = 0;
        do
        {
            i = NetworkHelper::anetRead(fd, (char *)inputBuf, BUF_SIZE);
            inputindex += i;
        } while (i != 0);
        if (!inputindex)
            return;
        int spaceindex = 0;
        while (spaceindex < inputindex && inputBuf[spaceindex++] != ' ')
            ;
        inputBuf[spaceindex - 1] = '\0';

        ChatOperator op;
        if (!strncmp((const char *)inputBuf, "SENDTO", 6))
        {
            op = ChatOperator::SENDMSG;
        }
        else if (!strncmp((const char *)inputBuf, "LOGIN", 5))
        {
            op = ChatOperator::LOGIN;
        }
        else if (!strncmp((const char *)inputBuf, "LOGOUT", 6))
        {
            op = ChatOperator::LOGOUT;
        }
        else if (!strncmp((const char *)inputBuf, "LISTALL", 7))
        {
            op = ChatOperator::LISTALL;
        }
        else
        {
            cout << "Bad Commend" << endl;
            inputindex = 0;
            return;
        }

        union {
            int chi;
            char chs[4];
        } ubuf;

        ubuf.chi = htonl(op);
        adata2wbuf((const uint8_t *)ubuf.chs, 4);

        switch (op)
        {
        case ChatOperator::LOGIN:
        {
            while (inputBuf[++spaceindex] != ' ')
                ;
            int i = spaceindex;
            while (spaceindex < inputindex && inputBuf[spaceindex++] == ' ')
                ;
            inputBuf[spaceindex - 1] = '\0';
            adata2wbuf((const uint8_t *)inputBuf + i, spaceindex - i - 1);
            adata2wbuf((const uint8_t *)"$", 1);

            while (inputBuf[++spaceindex] != ' ')
                ;
            i = spaceindex;
            while (spaceindex < inputindex && inputBuf[spaceindex++] != ' ')
                ;
            inputBuf[spaceindex - 1] = '\0';

            adata2wbuf((const uint8_t *)inputBuf + i, spaceindex - i - 1);
            adata2wbuf((const uint8_t *)"$", 1);
            adata2wbuf((const uint8_t *)"\r\n", 2);

            send(_fd, wbuf, windex, 0);
            break;
        }

        default:
            break;
        }
    }
    bool adata2wbuf(const uint8_t *data, ssize_t n)
    {
        if (n + this->windex >= this->wbufsize)
        {
            ssize_t afsize = (n + this->windex) * 1.5;
            uint8_t *newbuf;
            if (nullptr == (newbuf = (uint8_t *)realloc(wbuf, afsize)))
                throw bad_alloc();
            else
            {
                this->wbuf = newbuf;
                this->wbufsize = afsize;
            }
        }
        memcpy(this->wbuf + this->windex, data, n);
        this->windex += n;
        return true;
    }
    bool adata2rbuf(const uint8_t *data, ssize_t n)
    {
        if (n + this->rindex >= this->rbufsize)
        {
            ssize_t afsize = (n + this->rindex) * 1.5;
            uint8_t *newbuf;
            if (nullptr == (newbuf = (uint8_t *)realloc(rbuf, afsize)))
                throw bad_alloc();
            else
            {
                this->rbuf = newbuf;
                this->rbufsize = afsize;
            }
        }
        memcpy(this->rbuf + this->rindex, data, n);
        this->rindex += n;
        return true;
    }

private:
    shared_ptr<Rkernel> kernelApi;
    uint8_t *wbuf, *rbuf, *inputBuf;
    int windex = 0, wbufsize = BUF_SIZE;
    int rindex = 0, rbufsize = BUF_SIZE;
    int _fd;
};

int main()
{
    auto c = MClient();
    c.Start();
}