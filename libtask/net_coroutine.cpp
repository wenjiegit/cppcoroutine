#include "net_coroutine.h"
#include "cppco.h"

#include <memory.h>
#include <mutex>

namespace cpp_coroutine {
static std::mutex net_g_mutex;
static bool is_running;

static void onwork() {
    while(true) {

    };
}

void net_init() {
    std::lock_guard<std::mutex> locker(net_g_mutex);
    if (!is_running) {
        is_running = true;
        cppco::coroutine_create(onwork);
    }
}


listen_coroutine::listen_coroutine():_fd(INVALID_SOCKET) {

}

listen_coroutine::~listen_coroutine() {

}

int listen_coroutine::createfd(bool is_tcp, std::string hostip, int port) {
	int fd, n, proto;
	struct sockaddr_in sa;
	socklen_t sn;
	unsigned int ip;

    if (is_running) {
        return INVALID_SOCKET;
    }

    if (_fd != INVALID_SOCKET) {
        return _fd;
    }

	proto = is_tcp ? SOCK_STREAM : SOCK_DGRAM;
	memset(&sa, 0, sizeof sa);
	sa.sin_family = AF_INET;
	if(!hostip.empty()){
		if(netlookup(hostip, &ip) < 0){
			return INVALID_SOCKET;
		}
		memmove(&sa.sin_addr, &ip, 4);
	}
	sa.sin_port = htons(port);
	if((fd = socket(AF_INET, proto, 0)) < 0){
		return INVALID_SOCKET;
	}
	
	/* set reuse flag for tcp */
	if(is_tcp && getsockopt(fd, SOL_SOCKET, SO_TYPE, (void*)&n, &sn) >= 0){
		n = 1;
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&n, sizeof n);
	}

	if(bind(fd, (struct sockaddr*)&sa, sizeof sa) < 0){
		close(fd);
		return INVALID_SOCKET;
	}

	if(proto == SOCK_STREAM) {
        listen(fd, 16);
    }

	fdnoblock(fd);
    _fd = fd;

	return fd;
}

int listen_coroutine::acceptfd() {
    int accept_fd = INVALID_SOCKET;
	int one;
	struct sockaddr_in sa;
	unsigned char *ip;
	socklen_t len;
	
    if (is_running) {
        return INVALID_SOCKET;
    }

    if (_fd == INVALID_SOCKET) {
        return _fd;
    }

	fdwait(_fd, READ_WAIT);

	len = sizeof sa;
	if((accept_fd = accept(_fd, (sockaddr*)&sa, &len)) < 0){
		return INVALID_SOCKET;
	}

	fdnoblock(accept_fd);
	one = 1;
	setsockopt(accept_fd, IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof one);

    return accept_fd;
}

void listen_coroutine::fdwait(int fd, int wait_way) {

}

int listen_coroutine::fdnoblock(int fd)
{
	return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL)|O_NONBLOCK);
}

void listen_coroutine::onwork() {

}

int netlookup(std::string& hostname, unsigned int *ip)
{
	struct hostent *he;

	if(parseip(hostname, ip) >= 0)
		return 0;
	
	/* BUG - Name resolution blocks.  Need a non-blocking DNS. */
	if((he = gethostbyname(hostname.c_str())) != 0){
		*ip = *(uint32_t*)he->h_addr;
		return 0;
	}

	return -1;
}

#define CLASS(p) ((*(unsigned char*)(p))>>6)
int parseip(std::string& hostname, uint32_t *ip)
{
	unsigned char addr[4];
	int i, x;
	const char* hostip_sz = hostname.c_str();
    char p[80];

    strcpy(p, hostip_sz);
	for(i=0; i<4 && *p; i++){
		x = strtoul(p, &p, 0);
		if(x < 0 || x >= 256)
			return -1;
		if(*p != '.' && *p != 0)
			return -1;
		if(*p == '.')
			p++;
		addr[i] = x;
	}

	switch(CLASS(addr)){
	case 0:
	case 1:
		if(i == 3){
			addr[3] = addr[2];
			addr[2] = addr[1];
			addr[1] = 0;
		}else if(i == 2){
			addr[3] = addr[1];
			addr[2] = 0;
			addr[1] = 0;
		}else if(i != 4)
			return -1;
		break;
	case 2:
		if(i == 3){
			addr[3] = addr[2];
			addr[2] = 0;
		}else if(i != 4)
			return -1;
		break;
	}
	*ip = *(uint32_t*)addr;
	return 0;
}

}