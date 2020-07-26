#include "chatRoom.h"
#include "server.h"
#include "client.h"
#include <unordered_map>
#include <cstring>
#include <arpa/inet.h>
#include "rkernel.h"

const uint8_t *ParseError = (const uint8_t *)"Parse Error";
const ssize_t PARSEERROR_SIZE = 11;

const uint8_t *NoAuthError = (const uint8_t *)"No Auth";
const ssize_t NOAUTHERROR_SIZE = 8;
void Chat::Parse(Client &c)
{
    ChatClient *pm = nullptr;
    if (nullptr == (pm = dynamic_cast<ChatClient *>(&c)))
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
                if (shared_ptr<ServerHandle> ptr = this->server.lock())
                {
                    Clients[*pm] = ptr->findClient(*pm);
                }
                c.addEvent(&Client::onWriteable, R_WRITABLE);
            }
            break;
        }
        case ChatOperator::LOGOUT:
        {
            pm->authed = false;
            pm->delEvent(R_READABLE);
            pm->delEvent(R_WRITABLE);
            Clients[*pm].reset();
            if (shared_ptr<ServerHandle> ptr = this->server.lock())
            {
                ptr->dropClient(*pm);
            }
            // pm->cclose();
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
            for (auto cpair = Clients.begin(); cpair != Clients.end(); ++cpair, fd++)
            {
                if (cpair->expired())
                    continue;
                union {
                    int chi;
                    char chs[4];
                } ubuf = {0};

                shared_ptr<Client> pos = nullptr;
                if (nullptr != (pos = cpair->lock()))
                {
                    if (pm == pos.get())
                        continue;
                    auto *p = dynamic_cast<ChatClient *>(pos.get());
                    if (p != nullptr)
                    {

                        ubuf.chi = htonl(fd);
                        pm->adata2wbuf((uint8_t *)ubuf.chs, 4);
                        pm->adata2wbuf((const uint8_t *)"$", 1);
                        pm->adata2wbuf((const uint8_t *)p->account, strlen(p->account));
                        pm->adata2wbuf((const uint8_t *)"$", 1);
                    }
                    else
                    {
                        cpair->reset();
                    }
                }
            }
            pm->adata2wbuf((const uint8_t *)"\r\n", 2);
            // pm->clearrbuf();
            c.addEvent(&Client::onWriteable, R_WRITABLE);
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
            auto p = Clients.at(i);
            if (p.expired())
            {
                shared_ptr<Client> pc;
                if (pc = p.lock())
                {
                    auto &pcc = dynamic_cast<ChatClient &>(*pc);
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