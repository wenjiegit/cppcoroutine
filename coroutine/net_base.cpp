#include "net_base.h"
#include "task_coroutine.h"
#include "task_pub.h"
#include "colog/colog.h"

#include <unordered_map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <string>
#include <fcntl.h>

namespace cpp_coroutine {

#define EVENT_LIST_COUNT 256

int s_epfd = 0;
std::unordered_map<int, Task_S*> s_epoll_task_map;//fd-->Task_S*

extern std::shared_ptr<task_coroutine> get_coroutine();

void net_init() {
    if (s_epfd) {
        return;
    } 
    s_epfd = epoll_create(EVENT_LIST_COUNT);
    if (s_epfd <= 0) {
        exit(0);
    }
    //CO_LOGF(LOG_INFO, "epoll handle=%d, add:0x%08x, mod:0x%08x, del:0x%08x", 
    //    s_epfd, EPOLL_CTL_ADD, EPOLL_CTL_MOD, EPOLL_CTL_DEL);

    get_coroutine()->taskcreate(net_run);

    return;
}

void net_run() {
    struct epoll_event ev_list[EVENT_LIST_COUNT];
    //CO_LOG(LOG_INFO, "net_run is starting...");

    while(true) {
        int yield_ret = 0;
        long long ms = 0;

        do {
            yield_ret = get_coroutine()->taskyield();
        } while(yield_ret > 0);

        bool sleep_empty = get_coroutine()->_sleep_task_map.empty();

        if (sleep_empty) {
            ms = 0;
        } else {
            auto now_ts = now_ms();
            auto first_iter = get_coroutine()->_sleep_task_map.begin();
            Task_S* t = (Task_S*)(first_iter->second);

            if (now_ts >= t->alarmtime) {
                ms = 0;
            } else {
                ms = 10;
            }
        }
        int ev_count = epoll_wait(s_epfd, ev_list, EVENT_LIST_COUNT, ms);
        for(int index = 0; index < ev_count; index++) {
            if (((ev_list[index].events & EPOLLIN) == EPOLLIN) || ((ev_list[index].events & EPOLLOUT) == EPOLLOUT)) {
                int read_fd = ev_list[index].data.fd;
                if (read_fd < 0) {
                    continue;
                }
                //CO_LOGF(LOG_INFO, "epoll active fd:%d", read_fd);
                auto iter = s_epoll_task_map.find(read_fd);
                if (iter != s_epoll_task_map.end()) {//active recv/send task
                    Task_S* net_task_item = (Task_S*)(iter->second);
                    get_coroutine()->taskready(net_task_item);
                }
            }
        }
    };
}

int fdnoblock(int fd) {
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL)|O_NONBLOCK);
}

int netlookup(const std::string& hostname, unsigned int *ip)
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
int parseip(const std::string& hostname, uint32_t *ip)
{
	unsigned char addr[4];
	int i, x;
	const char* hostip_sz = hostname.c_str();
    char data[80];
    char* p = data;

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