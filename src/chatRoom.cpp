#include "chatRoom.h"
#include "server.h"
#include "client.h"
#include <unordered_map>
#include <cstring>
#include <arpa/inet.h>
#include "rkernel.h"
#include "networking.h"
#include <functional>

const uint8_t *ParseError = (const uint8_t *)"Parse Error";
const ssize_t PARSEERROR_SIZE = 11;

const uint8_t *NoAuthError = (const uint8_t *)"No Auth";
const ssize_t NOAUTHERROR_SIZE = 8;

void Chat::Start() noexcept
{
    this->server->Start(this);
}

void Chat::ConnectHandle(int fd)
{
    auto c = make_shared<ChatClient>(this->server->kernel);
    if (-1 == (*c = NetworkHelper::anetTcpAccept(fd, (char *)c->remoteIp.ip, 64, &(c->remoteIp.port))))
    {
        cout << "Accept error" << endl;
        return;
    }
    cout << *c << endl;
    NetworkHelper::setNoblock(*c, true);
    clients.at(*c) = c; //may be renew
    server->kernel->createEvent(*c, R_READABLE, bind(&Chat::OnMessage, this, placeholders::_1));
}
void Chat::OnMessage(int fd)
{
    shared_ptr<Client> pc = clients[fd];
    ChatClient *pr = nullptr;
    if (nullptr != (pr = dynamic_cast<ChatClient *>(pc.get())))
    {

        int n = NetworkHelper::anetRead(fd, (char *)(pr->rbuf + pr->rindex), 1024 - pr->rindex);

        if (!n || !(errno & EINTR || errno & EWOULDBLOCK))
        {
            pr->delEvent(R_READABLE);
            pr->cclose();
            online.remove_if([&](const weak_ptr<Client> &wpc) -> bool {
                if (wpc.expired())
                    return true;
                auto ptr = wpc.lock();
                return ptr == pc;
            });
            return;
        }
        pr->rindex += n;
        try
        {
            Parse(pc);
        }
        catch (exception &e)
        {
            fprintf(stderr, "Process Clients %d error %s", (int)*pc, e.what());
            pc->cclose();
        }
    }
}

void Chat::Parse(shared_ptr<Client> c)
{
    ChatClient *pm = nullptr;
    if (nullptr == (pm = dynamic_cast<ChatClient *>(c.get())))
    {
        cout << "Can not parse" << endl;
        return;
    }
    int lindex = 0;     // pm->index;
    if (pm->rindex < 4) //operator size must gether 4
    {
        return;
    }
    do
    {
        uint32_t op;
        union {
            uint32_t op;
            char ch[4];
        } opv;

        memcpy(opv.ch, pm->rbuf + lindex, 4);
        op = ntohl(opv.op);
        lindex += 4;

        //auth check
        if (op != ChatOperator::LOGIN && !pm->authed)
        {
            pm->clearrbuf();
            pm->adata2wbuf(NoAuthError, NOAUTHERROR_SIZE);
            pm->adata2wbuf((const uint8_t *)"\r\n", 2);
            pm->addEvent(&ChatClient::onWriteable, R_WRITABLE);
            break;
        }
        switch (op)
        {
        case ChatOperator::LOGIN:
        {
            pm->currOp = ChatOperator::LOGIN;
            if (pm->authed)
            {
                pm->clearrbuf();
                return;
            }
            char account[20] = {0};
            char passwd[20] = {0};
            int i = 0;
            while (lindex < pm->rindex && '$' != (account[i++] = pm->rbuf[lindex++]))
                ;
            if (i > 0)
                account[i - 1] = '\0';
            if (lindex >= pm->rindex)
                return;
            i = 0;
            while (lindex < pm->rindex && '$' != (passwd[i++] = pm->rbuf[lindex++]))
                ;
            if (i > 0)
                passwd[i - 1] = '\0';
            if (lindex >= pm->rindex)
                return;
            if (strncmp((char *)((pm->rbuf) + lindex), "\r\n", 2))
                sprintf((char *)((pm->wbuf) + pm->windex), "Parse Login operator Failed at %d\r\n", lindex);
            else
            {
                lindex += 2;
                if (lindex >= pm->rindex)
                    pm->rindex = 0;
                sprintf((char *)((pm->wbuf) + pm->windex), "Login Ok\r\n"); //, lindex);
                pm->windex += 9;
                pm->authed = true;
                memcpy(pm->account, account, 20);
                this->online.push_front(c);
                c->addEvent(&Client::onWriteable, R_WRITABLE);
            }
            break;
        }
        case ChatOperator::LOGOUT:
        {
            pm->authed = false;
            pm->delEvent(R_READABLE);
            pm->delEvent(R_WRITABLE);
            online.remove_if([&](const weak_ptr<Client> &wpc) -> bool {
                if (wpc.expired())
                    return false;
                auto ptr = wpc.lock();
                return ptr == c;
            });
            return;
        }
        case ChatOperator::LISTALL:
        {
            if (pm->rindex < (lindex + 2))
                return;

            if (strncmp((char *)((pm->rbuf) + lindex), "\r\n", 2))
            {
                pm->adata2wbuf((const uint8_t *)ParseError, PARSEERROR_SIZE);
                pm->addEvent(&Client::onWriteable, R_WRITABLE);
                pm->clearrbuf();
                return;
            }
            lindex += 2;
            //sprintf((char *)((pm->wbuf) + pm->windex), "Parse listall operator Failed at %d\r\n", lindex);
            if (!pm->authed)
            {
                pm->rindex = 0;
                return;
            }
            int fd = 0;
            online.remove_if([](weak_ptr<Client> &p) -> bool { return p.expired(); });
            for (auto cpair = online.begin(); cpair != online.end(); ++cpair, fd++)
            {
                if (shared_ptr<Client> pcs = cpair->lock())
                {
                    if (nullptr == pcs.get() || pm == pcs.get())
                        continue;
                    union {
                        int chi;
                        char chs[4];
                    } ubuf = {0};

                    auto *p = dynamic_cast<ChatClient *>(pcs.get());
                    if (p != nullptr)
                    {
                        ubuf.chi = htonl(fd);
                        pm->adata2wbuf((uint8_t *)ubuf.chs, 4);
                        pm->adata2wbuf((const uint8_t *)"$", 1);
                        pm->adata2wbuf((const uint8_t *)p->account, strlen(p->account));
                        pm->adata2wbuf((const uint8_t *)"$", 1);
                    }
                }
            }
            pm->adata2wbuf((const uint8_t *)"\r\n", 2);
            // pm->clearrbuf();
            c->addEvent(&Client::onWriteable, R_WRITABLE);
            break;
        }
        case ChatOperator::SENDMSG:
        {
            if (lindex + 4 >= pm->rindex) //jump only u id
                return;
            if ((pm->rbuf[lindex + 4]) != '$')
            {
                cout << "Parse Error";
                pm->adata2wbuf(ParseError, PARSEERROR_SIZE);
                pm->addEvent(&Client::onWriteable, R_WRITABLE);
                pm->clearrbuf();
                return;
            }

            union {
                uint32_t chi;
                char chs[4];
            } ubuf;
            memcpy(ubuf.chs, pm->rbuf + lindex, 4);
            lindex += 5;
            int i = ntohl(ubuf.chi);
            auto p = clients.at(i);
            if (p != nullptr)
            {
                // shared_ptr<ChatClient> pcc;
                auto &pcc = dynamic_cast<ChatClient &>(*p);
                i = 0;
                while (++i && lindex < pm->rindex && '$' != (pm->rbuf)[++lindex])
                    ;
                if (lindex >= pm->rindex)
                    return;

                pcc.adata2wbuf((const uint8_t *)"From:", 4);
                pcc.adata2wbuf((const uint8_t *)pm->account, strlen(pm->account));
                pcc.adata2wbuf((const uint8_t *)"$", 1);
                pcc.adata2wbuf(pm->rbuf + lindex - i, i);
                pcc.adata2wbuf((const uint8_t *)"$\r\n", 3);
                if (strncmp((char *)((pm->rbuf) + (++lindex)), "\r\n", 2))
                {
                    pcc.clearwbuf();
                    pm->adata2wbuf(ParseError, PARSEERROR_SIZE);
                    pm->addEvent(&Client::onWriteable, R_WRITABLE);
                    pm->clearrbuf();
                }
                else
                {
                    lindex += 2;
                    pcc.addEvent(&Client::onWriteable, R_WRITABLE);
                }
            }
            else
            {
                pm->adata2wbuf((const uint8_t *)"Not Found\r\n", 11);
                pm->addEvent(&Client::onWriteable, R_WRITABLE);
                return;
            }
            break;
        }
        default:
        {
            break;
        }
        }
    } while (lindex < pm->rindex);

    pm->clearrbuf();
}