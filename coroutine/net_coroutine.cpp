#include "net_coroutine.h"
#include "cppco.h"
#include "net_base.h"
#include "colog/colog.h"

#include <memory.h>
#include <mutex>
#include <sys/epoll.h>
#include <unordered_map>
#include <string.h>
#include <arpa/inet.h>

namespace cpp_coroutine {
extern int s_epfd;
extern std::unordered_map<int, Task_S*> s_epoll_task_map;//fd-->Task_S*
extern std::shared_ptr<task_coroutine> get_coroutine();

listen_coroutine::listen_coroutine():_fd(INVALID_SOCKET)
    ,_current_epoll_state(0) {

}

listen_coroutine::~listen_coroutine() {

}

int listen_coroutine::createfd(bool is_tcp, std::string hostip, int port) {
	int fd, n, proto;
	struct sockaddr_in sa;
	socklen_t sn;
	unsigned int ip;

    if (_fd != INVALID_SOCKET) {
		//CO_LOG(LOG_INFO, "createfd has inited...");
        return _fd;
    }

	proto = is_tcp ? SOCK_STREAM : SOCK_DGRAM;
	memset(&sa, 0, sizeof sa);
	sa.sin_family = AF_INET;
	if(!hostip.empty()){
		if(netlookup(hostip, &ip) < 0){
			//CO_LOGF(LOG_INFO, "netlookup hostip:%s", hostip.c_str());
			return INVALID_SOCKET;
		}
		memmove(&sa.sin_addr, &ip, 4);
	}
	sa.sin_port = htons(port);
	if((fd = socket(AF_INET, proto, 0)) < 0){
		//CO_LOG(LOG_INFO, "create socket error");
		return INVALID_SOCKET;
	}
	
	/* set reuse flag for tcp */
	if(is_tcp && getsockopt(fd, SOL_SOCKET, SO_TYPE, (void*)&n, &sn) >= 0){
		n = 1;
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&n, sizeof n);
	}

	if(bind(fd, (struct sockaddr*)&sa, sizeof sa) < 0){
		close(fd);
		//CO_LOG(LOG_INFO, "bind socket error");
		return INVALID_SOCKET;
	}

	if(proto == SOCK_STREAM) {
        listen(fd, 16);
    }

	fdnoblock(fd);
    _fd = fd;
	_info.ip = hostip;
	_info.port = port;

    //CO_LOGF(LOG_INFO, "bind socket, ip:%s, port:%d", hostip.c_str(), port);
	return fd;
}

void listen_coroutine::accept_wait() {
    if (_current_epoll_state == 0) {
	    struct epoll_event ev;
    
        ev.data.fd = _fd;
	    ev.events = EPOLLIN;
		_current_epoll_state = EPOLLIN;
		//CO_LOGF(LOG_INFO, "epoll op:0x%08x, event:0x%08x, _fd:%d, s_epfd:%d", 
		//    EPOLL_CTL_ADD, EPOLLIN, _fd, s_epfd);
		epoll_ctl(s_epfd, EPOLL_CTL_ADD, _fd, &ev);
    }

    auto running_task = get_coroutine()->get_runing_task();
    s_epoll_task_map.insert(std::pair<int,Task_S*>(_fd, running_task));
	get_coroutine()->taskswitch();
    return;
}

std::shared_ptr<net_conn> listen_coroutine::accept_conn() {
    int accept_fd = INVALID_SOCKET;
	int one;
	struct sockaddr_in sa;
	socklen_t len;

    if (_fd == INVALID_SOCKET) {
        return nullptr;
    }

    accept_wait();

	len = sizeof sa;
	if((accept_fd = accept(_fd, (sockaddr*)&sa, &len)) < 0){
		return nullptr;
	}
    //CO_LOGF(LOG_INFO, "accept ok, accept_fd:%d", accept_fd);
	fdnoblock(accept_fd);
	one = 1;
	setsockopt(accept_fd, IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof one);

    sockaddr_in sin;
	ADDR_INFO remote_info;

    memcpy(&sin, &sa, sizeof(sin));
    remote_info.ip = inet_ntoa(sin.sin_addr);
	remote_info.port = sin.sin_port;

    auto accept_net_conn = std::make_shared<net_coroutine>(accept_fd, remote_info, _info);

    return accept_net_conn;
}

void listen_coroutine::close_conn() {
	struct epoll_event ev;

    ev.data.fd = _fd;
	ev.events = EPOLLIN;
    epoll_ctl(s_epfd, EPOLL_CTL_DEL, _fd, &ev);
    close(_fd);
	_fd = INVALID_SOCKET;
}

ADDR_INFO listen_coroutine::get_addr() {
    return _info;
}

net_coroutine::net_coroutine(int fd):_fd(fd)
    ,_current_epoll_state(0) {

}

net_coroutine::net_coroutine(int fd, ADDR_INFO remote, ADDR_INFO local):_fd(fd)
    ,_current_epoll_state(0)
    ,_local_info(local)
	,_remote_info(remote) {

}

net_coroutine::~net_coroutine() {
	//CO_LOGF(LOG_INFO, "net coroutine is destructing, fd:%d", _fd);
    close_conn();
}

int net_coroutine::read_data(char* data_p, int data_size) {
    int rcv_len = 0;
    bool epoll_enable = false;

    do {
        rcv_len = read(_fd, data_p, data_size);
        if ((rcv_len < 0) && (errno == EAGAIN)) {
			epoll_enable = true;
			read_wait();
        } else {
            break;
        }
    } while(true);

    if (epoll_enable) {
		remove_epoll_event(EPOLLOUT);
	}
    return rcv_len;
}

int net_coroutine::write_data(char* data_p, int data_size) {
    int rcv_len = 0;
    int total_len = 0;
    bool epoll_enable = false;

    while (total_len < data_size) {
        do {
            rcv_len = write(_fd, data_p + total_len, data_size - total_len);
            if ((rcv_len < 0) && (errno == EAGAIN)) {
				epoll_enable = true;
				write_wait();
            } else {
                break;
            }
        } while(true);

        if (rcv_len < 0) {
			if (epoll_enable) {
				remove_epoll_event(EPOLLIN);
			}
            return rcv_len;
        }
        if (rcv_len == 0) {
            break;
        }
        total_len += rcv_len;
    };

    return total_len;
}

void net_coroutine::remove_epoll_event(int event_bits) {
	struct epoll_event ev;

    ev.data.fd = _fd;
	ev.events = event_bits;

	epoll_ctl(s_epfd, EPOLL_CTL_DEL, _fd, &ev);

	return;
}

void net_coroutine::read_wait() {
    int epoll_op_type = 0;
	struct epoll_event ev;

    ev.data.fd = _fd;
	ev.events = EPOLLIN;

    if (_current_epoll_state == 0) {
        epoll_op_type = EPOLL_CTL_ADD;
		_current_epoll_state = EPOLLIN;
	} else {
		epoll_op_type = EPOLL_CTL_MOD;
		_current_epoll_state = EPOLLIN;
	}
    epoll_ctl(s_epfd, epoll_op_type, _fd, &ev);
    
	auto running_task = get_coroutine()->get_runing_task();
    s_epoll_task_map.insert(std::pair<int,Task_S*>(_fd, running_task));

    //CO_LOGF(LOG_INFO, "read wait epoll, op:0x%08x, state:0x%08x, fd:0x%08x", 
    //    epoll_op_type, _current_epoll_state, _fd);
	get_coroutine()->taskswitch();
    return;
}

void net_coroutine::write_wait() {
    int epoll_op_type = 0;
	struct epoll_event ev;

    ev.data.fd = _fd;
	ev.events = EPOLLOUT;

    if (_current_epoll_state == 0) {
        epoll_op_type = EPOLL_CTL_ADD;
		_current_epoll_state = EPOLLOUT;
	} else {
		epoll_op_type = EPOLL_CTL_MOD;
		_current_epoll_state = EPOLLOUT;
	}
    epoll_ctl(s_epfd, epoll_op_type, _fd, &ev);
    
	auto running_task = get_coroutine()->get_runing_task();
    s_epoll_task_map.insert(std::pair<int,Task_S*>(_fd, running_task));

	get_coroutine()->taskswitch();
    return;
}

int net_coroutine::close_conn() {
    if (_current_epoll_state != 0) {
		struct epoll_event ev;
        ev.data.fd = _fd;
	    ev.events = _current_epoll_state;
		epoll_ctl(s_epfd, EPOLL_CTL_DEL, _fd, &ev);
		_current_epoll_state = 0;
	}
	s_epoll_task_map.erase(_fd);

	if (_fd > 0) {
        close(_fd);
		_fd = -1;
	}
	return 0;
}

ADDR_INFO net_coroutine::local_addr() {
	return _local_info;
}

ADDR_INFO net_coroutine::remote_addr() {
	return _remote_info;
}
}