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
        addEvent(&MClient::Parse, R_READABLE);
        addEvent(&MClient::InputParse, R_READABLE, STDIN_FILENO);

        kernelApi->mainLoop();
    }
    void Parse(int fd)
    {
        int i = 0;
        i = NetworkHelper::anetRead(fd, (char *)rbuf, BUF_SIZE);
        cout << rbuf << endl;
        cout << "\n>>> " << flush;
    }
    void InputParse(int fd)
    {
        int inputLength = 0;
        int i = 0;
        do
        {
            i = NetworkHelper::anetRead(fd, (char *)inputBuf, BUF_SIZE);
            inputLength += i;
        } while (i != 0);
        if (!inputLength)
            return;
        int spaceindex = 0;
        while (spaceindex < inputLength && inputBuf[spaceindex++] != ' ')
            ;
        // inputBuf[spaceindex] = '\0';

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
            cout << "Bad Command" << endl;
            inputLength = 0;
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
            while (spaceindex < inputLength && inputBuf[spaceindex++] == ' ')
                ;
            int i = spaceindex - 1;
            while (spaceindex < inputLength && inputBuf[spaceindex++] != ' ')
                ;
            // inputBuf[spaceindex - 1] = '\0';
            if (i == spaceindex || i + 1 == spaceindex)
                goto error;
            adata2wbuf((const uint8_t *)inputBuf + i, spaceindex - i - 1);
            adata2wbuf((const uint8_t *)"$", 1);

            while (spaceindex < inputLength && inputBuf[spaceindex++] == ' ')
                ;
            i = spaceindex - 1;
            while (spaceindex < inputLength && inputBuf[spaceindex++] != ' ')
                ;
            // inputBuf[spaceindex - 1] = '\0';
            if (i == spaceindex)
                goto error;
            adata2wbuf((const uint8_t *)inputBuf + i, spaceindex - i - 1);
            adata2wbuf((const uint8_t *)"$", 1);
            adata2wbuf((const uint8_t *)"\r\n", 2);

            addEvent(&MClient::OnWrite, R_WRITABLE);
            break;
        }
        case ChatOperator::LISTALL:
        {
            adata2wbuf((const uint8_t *)"\r\n", 2);
            addEvent(&MClient::OnWrite, R_WRITABLE);
            break;
        }
        case ChatOperator::SENDMSG:
        {
            while (spaceindex < inputLength && inputBuf[spaceindex++] == ' ') //跳过空格
                ;
            int chStart = spaceindex - 1; //记录字符开始位置
            while (spaceindex < inputLength && inputBuf[spaceindex++] != ' ')
                ;                            //指向非空格下一个位置
            inputBuf[spaceindex - 1] = '\0'; //空格替换为'\0'
            if (chStart == spaceindex || chStart + 1 == spaceindex)
                goto error;
            int num = atoi((const char *)inputBuf + chStart);
            ubuf.chi = htonl(num);
            adata2wbuf((const uint8_t *)ubuf.chs, 4);
            adata2wbuf((const uint8_t *)"$", 1);
            spaceindex--;
            while (spaceindex < inputLength && inputBuf[spaceindex++] == ' ')
                ; //跳过空格
            chStart = spaceindex;
            char endpoint = inputBuf[spaceindex - 1] == '"' ? '"' : ' ';

            while (spaceindex < inputLength && inputBuf[spaceindex++] != endpoint)
                ;
            adata2wbuf(inputBuf + chStart, spaceindex - chStart);
            adata2wbuf((const uint8_t *)"$\r\n", 3);

            addEvent(&MClient::OnWrite, R_WRITABLE);
            break;
        }
        default:
        {
        error:
            cout << "bad command" << endl;
            break;
        }
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
    bool addEvent(void (MClient::*p)(int), int mask, int fd = -1)
    {
        return this->kernelApi->createEvent(fd >= 0 ? fd : _fd, mask, bind(p, this, placeholders::_1));
    };
    void delEvent(int mask, int fd = -1)
    {
        return this->kernelApi->deleteEvent(fd >= 0 ? fd : _fd, mask);
    }
    void OnWrite(int)
    {
        int i = 0;
        do
        {
            i = send(_fd, wbuf, windex - i, 0);
        } while (i < windex && i != -1);
        if (i < 0)
            throw exception();
        windex = 0;
        delEvent(R_WRITABLE);
    }
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