#ifndef __NETWORKING_H
#define __NETWORKING_H

#include <unistd.h>
#include <sys/socket.h>

using namespace std;
class NetworkHelper
{
public:
    static bool setNoblock(int fd, bool non_block);
    static int anetListen(int s, struct sockaddr *sa, socklen_t len, int backlog);
    static int anetGenericAccept(int s, struct sockaddr *sa, socklen_t *len);
    static int anetTcpAccept(int s, char *ip, size_t ip_len, int *port);
    static int  anetRead(int fd, char *buf, int count);
};

#endif