#ifndef SERVER_PICKER_H_
#define SERVER_PICKER_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 

//#include "../include/common/qglobal.h"
#define DOWNLOAD_DEFAULT_TIMEOUT 10000

struct ServerInfo {
	char	ip[16];		/* remote server ip */
	int32_t	port;		/* remote server port */
	int32_t timeout;	/* remote server timeout */

	ServerInfo(int32_t timeout_=DOWNLOAD_DEFAULT_TIMEOUT) :
		timeout(timeout_)
	{}

	ServerInfo(const char* ip_, int32_t port_, int32_t timeout_=DOWNLOAD_DEFAULT_TIMEOUT) :
		port(port_),
		timeout(timeout_)
	{
		strcpy(ip, ip_);
	}
};

class ServerPicker
{
public:
    //ServerPicker();
    /**
     *  ServerPicker 初始化函数 包括加载下载的ip和端口
     */
    int init();
    /**
     * getServerInfo 获取一个下载进程的server ip Port
     * @return    True: success
     */
    bool getServerInfo(ServerInfo& server);
private:
    ServerInfo* all_servers[10];
    //可以用来下载的主机数目
    unsigned int server_num;
};

#endif
