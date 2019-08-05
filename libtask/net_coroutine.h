#ifndef NET_COROUTINE_H
#define NET_COROUTINE_H
#include "net_base.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/poll.h>
#include <string>

namespace cpp_coroutine {

#define INVALID_SOCKET (-1)

#define READ_WAIT 1
#define WRITE_WAIT 2

int parseip(std::string& hostname, uint32_t *ip);
int netlookup(std::string& hostname, unsigned int *ip);

void net_init();

class listen_coroutine : public net_listen {
public:
    listen_coroutine();
    ~listen_coroutine();

    //createfd shall be called before others function.
    int createfd(bool is_tcp, std::string hostip, int port);

    //before call accept_conn, should call createfd to get fd.
    virtual int accept_conn();
    virtual void close_conn();
    virtual ADDR_INFO get_addr();

private:
    int fdnoblock(int fd);
    void fdwait(int fd, int wait_way);

private:
    int _fd;
};
}

#endif //NET_COROUTINE_H