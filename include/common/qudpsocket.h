/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qudpsocket.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/05/03
**
*********************************************************************************************/

#ifndef __QUDPSOCKET_H_
#define __QUDPSOCKET_H_

#include "qglobal.h"

Q_BEGIN_NAMESPACE

// UDP通讯客户端
class QUdpClient {
	public:
		// @函数名: UDP通讯客户端初始化函数
		// @参数01: socket句柄
		// @参数02: UDP服务端IP地址
		// @参数03: UDP服务端port
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t init(void*& handle, char* server_ip, uint16_t server_port)
		{
			if(server_ip==NULL||*server_ip==0)
				return -1;

			strcpy(m_server_ip, server_ip);
			m_server_port=server_port;

			Q_SOCKET_T* socket=q_new<Q_SOCKET_T>();
			if(socket==NULL) {
				Q_DEBUG("QUdpClient: socket is null!");
				return -2;
			}

			if(q_init_socket()) {
				Q_DEBUG("QUdpClient: socket init error!");
				return -3;
			}

			handle=(void*)socket;
			return 0;
		}

		// @函数名: 数据包发送函数(单次发送)
		// @参数01: socket句柄
		// @参数02: 待发送数据
		// @参数03: 待发送数据长度
		// @参数04: 设置是否使用广播通信
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t send(void* handle, char* send_buf, int32_t send_len, bool broadcast=false)
		{
			struct sockaddr_in s_addr;
			s_addr.sin_family=AF_INET;
			s_addr.sin_addr.s_addr=inet_addr(m_server_ip);
			s_addr.sin_port=htons(m_server_port);

			Q_SOCKET_T* socket=reinterpret_cast<Q_SOCKET_T*>(handle);
			if((*socket=::socket(AF_INET, SOCK_DGRAM, 0))==-1) {
				Q_DEBUG("QUdpClient: socket create error!");
				return -1;
			}

			if(broadcast) {
				int32_t yes=1;
				if(setsockopt(*socket, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes))==-1) {
					Q_DEBUG("QUdpSocket: set broadcast error!");
					return -2;
				}
			}

			if((sendto(*socket, send_buf, send_len, 0, (struct sockaddr*)&s_addr, sizeof(s_addr))!=send_len)) {
				Q_DEBUG("QUdpClient: socket sendto error!");
				return -3;
			}

			q_close_socket(*socket);

			return 0;
		}

		// @函数名: 数据包交互函数(单次发送和接收)
		// @参数01: socket句柄
		// @参数02: 待发送数据
		// @参数03: 待发送数据长度
		// @参数04: 待接收数据内存
		// @参数05: 待接收数据最大内存空间
		// @参数06: 接收数据的实际长度
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t communicate(void* handle, char* send_buf, int32_t send_len, char* recv_buf, int32_t max_recv_size, int32_t& recv_len)
		{
			struct sockaddr_in s_addr;
			s_addr.sin_family=AF_INET;
			s_addr.sin_addr.s_addr=::inet_addr(m_server_ip);
			s_addr.sin_port=htons(m_server_port);

			socklen_t addr_len=sizeof(s_addr);

			Q_SOCKET_T* socket=reinterpret_cast<Q_SOCKET_T*>(handle);
			if((*socket=::socket(AF_INET, SOCK_DGRAM, 0))==-1) {
				Q_DEBUG("QUdpClient: socket create error!");
				return -1;
			}

			if((::sendto(*socket, send_buf, send_len, 0, (struct sockaddr*)&s_addr, sizeof(s_addr))!=send_len)) {
				Q_DEBUG("QUdpClient: socket sendto error!");
				return -2;
			}

			recv_len=::recvfrom(*socket, recv_buf, max_recv_size-1, 0, (struct sockaddr*)&s_addr, &addr_len);
			if(recv_len<0) {
				Q_DEBUG("QUdpClient: recv socket error!");
				return -3;
			}

			recv_buf[recv_len]=0;

			q_close_socket(*socket);

			return 0;
		}

		// @函数名: 关闭客户端连接
		// @参数01: socket连接句柄
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t close(void* handle)
		{
			Q_SOCKET_T* socket=reinterpret_cast<Q_SOCKET_T*>(handle);
			q_delete<Q_SOCKET_T>(socket);
			return 0;
		}

	protected:
		char m_server_ip[16];		// UDP服务端IP
		uint16_t m_server_port;		// UDP服务端端口号
};

// UDP通讯服务端
class QUdpServer {
	public:
		// @函数名: 初始化UDP通讯服务端
		// @参数01: 监听端口
		// @参数02: 最大接收工作线程数
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t init(uint16_t listen_port, int32_t worker_num=10)
		{
			m_listen_port=listen_port;
			m_worker_num=worker_num;

			if(q_init_socket()) {
				Q_DEBUG("QUdpServer: socket init error!");
				return -1;
			}

			if((m_listen=::socket(AF_INET, SOCK_DGRAM, 0))==-1) {
				Q_DEBUG("QUdpServer: socket create error!");
				return -2;
			}

			struct sockaddr_in s_addr;
			s_addr.sin_family=AF_INET;
			s_addr.sin_addr.s_addr=INADDR_ANY;
			s_addr.sin_port=htons(m_listen_port);

			if(::bind(m_listen, (struct sockaddr*)&s_addr, sizeof(s_addr))==-1) {
				Q_DEBUG("QUdpServer: socket bind error!");
				return -3;
			}

			return 0;
		}

		// @函数名: 启动服务
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t start()
		{
			for(int32_t i=0; i<m_worker_num; ++i) {
				m_success_flag=0;

				if(q_create_thread(us_daemon_thread, this)) {
					Q_DEBUG("QUdpServer: create thread error!");
					return -1;
				}

				while(m_success_flag==0)
					q_sleep(1);

				if(m_success_flag<0) {
					Q_DEBUG("QUdpServer: thread start error!");
					return -2;
				}
			}

			return 0;
		}

		// @函数名: 关闭socket服务端
		int32_t close()
		{
			q_close_socket(m_listen);
			return 0;
		}

	private:
		static Q_THREAD_T us_daemon_thread(void* argv)
		{
			QUdpServer* us=static_cast<QUdpServer*>(argv);

			char buffer[1<<10]={0};
			int32_t recv_len=0;
			int32_t send_len=0;

			struct sockaddr_in c_addr;
			socklen_t addr_len=sizeof(c_addr);

			us->m_success_flag=1;

			Q_FOREVER {
				memset(buffer, 0, sizeof(buffer));
				recv_len=::recvfrom(us->m_listen, buffer, sizeof(buffer)-1, 0, (struct sockaddr*)&c_addr, &addr_len);
				if(recv_len<0) {
					Q_DEBUG("QUdpServer: recvfrom error!");
					continue;
				}

				Q_DEBUG("Thread [%llu] recive from [%s:%d] message: %s", q_thread_id(), inet_ntoa(c_addr.sin_addr), ntohs(c_addr.sin_port), buffer);

				send_len=::sendto(us->m_listen, buffer, recv_len, 0, (struct sockaddr*)&c_addr, addr_len);
				if(send_len<0||send_len!=recv_len) {
					Q_DEBUG("QUdpServer: sendto error!");
					continue;
				}
			}

			us->m_success_flag=-1;
			return 0;
		}

	protected:
		uint16_t m_listen_port;		// 服务端监听端口号
		int32_t m_worker_num;		// 服务端工作线程数

		Q_SOCKET_T m_listen;		// 服务端监听描述符
		int32_t m_success_flag;		// 线程状态标识
};

Q_END_NAMESPACE

#endif // __QUDPSOCKET_H_
