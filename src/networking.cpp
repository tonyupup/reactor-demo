#include "networking.h"
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <string.h>
#include <sys/unistd.h>
#include <functional>
#include <arpa/inet.h>
#include <vector>
#include <memory>
#include <linux/tcp.h>
#include <linux/types.h>
#include <asm/byteorder.h>
#include <linux/ip.h>

bool NetworkHelper::setNoblock(int fd, bool non_block)
{
    //fix
    if (fd < 0)
        return false;
    int flags;
    /* Set the socket blocking (if non_block is zero) or non-blocking.
     * Note that fcntl(2) for F_GETFL and F_SETFL can't be
     * interrupted by a signal. */
    if ((flags = fcntl(fd, F_GETFL)) == -1)
    {
        throw "fcntl(F_GETFL): %s";
        // return ANET_ERR;
    }

    if (non_block)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    if (fcntl(fd, F_SETFL, flags) == -1)
    {
        throw "fcntl(F_SETFL,O_NONBLOCK): ";
        return false;
    }
    return true;
}
int NetworkHelper::anetListen(int s, struct sockaddr *sa, socklen_t len, int backlog)
{
    if (bind(s, sa, len) == -1)
    {
        // anetSetError(err, "bind: %s", strerror(errno));
        close(s);
        return -1;
    }

    if (listen(s, backlog) == -1)
    {
        // anetSetError(err, "listen: %s", strerror(errno));
        close(s);
        return -1;
    }
    return 0;
}

int NetworkHelper::anetGenericAccept(int s, struct sockaddr *sa, socklen_t *len)
{
    int fd;
    while (1)
    {
        fd = accept(s, sa, len);
        if (fd == -1)
        {
            if (errno == EINTR)
                continue;
            else
            {
                cout << "accept: " << strerror(errno) << endl;
                return -1;
            }
        }
        break;
    }
    return fd;
}
int NetworkHelper::anetTcpAccept(int s, char *ip, size_t ip_len, int *port)
{
    int fd;
    struct sockaddr_storage sa;
    socklen_t salen = sizeof(sa);
    if ((fd = anetGenericAccept(s, (struct sockaddr *)&sa, &salen)) == -1)
        return -1;

    if (sa.ss_family == AF_INET)
    {
        struct sockaddr_in *s = (struct sockaddr_in *)&sa;
        if (ip)
            inet_ntop(AF_INET, (void *)&(s->sin_addr), ip, ip_len);
        if (port)
            *port = ntohs(s->sin_port);
    }
    else
    {
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&sa;
        if (ip)
            inet_ntop(AF_INET6, (void *)&(s->sin6_addr), ip, ip_len);
        if (port)
            *port = ntohs(s->sin6_port);
    }
    return fd;
}
/*
    Read count bytes to buffer.
    Return:
        >0 normaly,recv count;
        -1 errno happend ,no data recv
        -2 err happend with data recv

*/
int NetworkHelper::anetRead(int fd, char *buf, int count)
{
    ssize_t nread, totlen = 0;
    while (totlen != count)
    {
        nread = read(fd, buf, count - totlen);
        if (nread == 0)
            return totlen;
        if (nread == -1)
        {
            if (errno & EAGAIN)
                break;
            if (totlen > 0)
                return totlen;
            else
                return -1;
        }
        totlen += nread;
        buf += nread;
    }
    return totlen;
}
// int NetworkHelper::SocketConnected(int sock)
// {
//     if (sock <= 0)
//         return 0;
//     struct tcp_info info;
//     int len = sizeof(info);
//     getsockopt(sock, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
//     if ((info.tcpi_state == TCP_ESTABLISHED))
//     {
//         //myprintf("socket connected\n");
//         return 1;
//     }
//     else
//     {
//         //myprintf("socket disconnected\n");
//         return 0;
//     }
// }
