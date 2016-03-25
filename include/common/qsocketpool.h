/********************************************************************************************
**
** Copyright (C) 2010-2015 Terry Niu (Beijing, China)
** Filename:	qsocketpool.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2015/02/26
**
*********************************************************************************************/

#ifndef __QSOCKETPOOL_H_
#define __QSOCKETPOOL_H_

#include "qglobal.h"
#include "qqueue.h"

Q_BEGIN_NAMESPACE

// Socket客户端连接类
class QClientSocket {
		friend class QSocketPool;

	public:
		QClientSocket() :
			client_sock_(-1),
			is_dead_(true),
			is_idle_(false)
		{}

		virtual ~QClientSocket()
		{close();}

		int32_t init(char* server_ip, uint16_t server_port, int32_t timeout=3000)
		{
			if(connect_to_server(server_ip, server_port, timeout)) {
				this->is_dead_=true;
				this->is_idle_=false;
				return -1;
			}

			this->is_dead_=false;
			this->is_idle_=true;
			return 0;
		}

		void close()
		{
			q_close_socket(client_sock_);
			this->is_dead_=true;
			this->is_idle_=false;
		}

		void set_dead(bool dead=true)
		{this->is_dead_=dead;}

		bool is_dead() const
		{return is_dead_;}

		void set_idle(bool idle=true)
		{this->is_idle_=idle;}

		bool is_idle() const
		{return is_idle_;}

		int32_t send_data(const char* data, int32_t data_len)
		{return q_sendbuf(client_sock_, const_cast<char*>(data), data_len);}

		int32_t recv_data(char* data, int32_t data_len)
		{return q_recvbuf(client_sock_, data, data_len);}

	private:
		int32_t connect_to_server(char* server_ip, uint16_t server_port, int32_t timeout=3000)
		{
			if(q_connect_socket(client_sock_, server_ip, server_port)) {
				Q_DEBUG("QSocketPool: socket connect error!");
				return -1;
			}

			int32_t flag=1;
			if(setsockopt(client_sock_, IPPROTO_TCP, TCP_NODELAY, (void*)&flag, sizeof(flag))==-1) {
				Q_DEBUG("QClientSocket: setsockopt error!");
				return -2;
			}

			if(q_set_overtime(client_sock_, timeout)) {
				Q_DEBUG("QClientSocket: socket set timeout error!");
				return -3;
			}

			return 0;
		}

	protected:
		Q_SOCKET_T	client_sock_;		// SOCKET连接套接字

		bool8_t		is_dead_;		// SOCKET连接是否有效
		bool8_t		is_idle_;		// SOCKET连接是否空闲
};

// SOCKET连接池类(长连接)
class QSocketPool {
	public:
		// @函数名: Socket连接池构造函数
		// @参数01: Socket服务端IP地址
		// @参数02: Socket服务端Port端口号
		// @参数03: 预创建socket连接数
		// @参数04: 最大支持socket连接数
		explicit QSocketPool(const char* server_ip, uint16_t server_port, int32_t timeout=3000, int32_t now_size=8, int32_t max_size=100) :
			server_port_(server_port),
			timeout_(timeout),
			now_size_(now_size),
			max_size_(max_size)
		{
			strcpy(this->server_ip_, server_ip);
			call_back_=this;
		}

		// @函数名: Socket连接池析构函数
		virtual ~QSocketPool()
		{
			for(int32_t i=0; i<now_size_; ++i)
				delete socket_pool_[i];

			if(socket_pool_)
				q_delete_array<QClientSocket*>(socket_pool_);
		}

		// @函数名: Socket连接池初始化函数
		// @参数01: 是否激活心跳机制
		// @参数02: 心跳机制起始时间(默认为毫秒)
		// @参数06: 心跳机制间隔时间(默认为毫秒)
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t init(bool heart_beat=true, int32_t heart_beat_now=1000, int32_t heart_beat_interval=10000)
		{
			heart_beat_=heart_beat;
			heart_beat_now_=heart_beat_now;
			heart_beat_interval_=heart_beat_interval;

			if(this->init_connection()) {
				Q_DEBUG("QSocketPool: QSocketPool init connections error!");
				return -1;
			}

			return 0;
		}

		// @函数名: 获取socket空闲连接
		QClientSocket* get_socket_connection()
		{
			mutex_.lock();
			for(int32_t i=0; i<now_size_; ++i) {
				if(socket_pool_[i]->is_dead()) {
					socket_pool_[i]->close();
					if(socket_pool_[i]->init(server_ip_, server_port_, timeout_))
						continue;
				}

				if(socket_pool_[i]->is_idle()) {
					socket_pool_[i]->set_idle(false);
					mutex_.unlock();
					return socket_pool_[i];
				}
			}
			mutex_.unlock();
			return NULL;
		}

		// @函数名: 释放socket连接
		int32_t release_socket_connection(QClientSocket* socket_client)
		{
			mutex_.lock();
			socket_client->set_idle(true);
			mutex_.unlock();
			return 0;
		}

	private:
		// @函数名: 初始化socket连接池
		int32_t init_connection()
		{
			socket_pool_=q_new_array<QClientSocket*>(max_size_);
			if(socket_pool_==NULL) {
				Q_DEBUG("QSocketPool: new socket_pool_ error!");
				return -1;
			}

			for(int32_t i=0; i<now_size_; ++i) {
				socket_pool_[i]=q_new<QClientSocket>();
				if(socket_pool_[i]==NULL) {
					Q_DEBUG("QSocketPool: new QClientSocket error!");
					return -2;
				}

				if(socket_pool_[i]->init(server_ip_, server_port_, timeout_)) {
					Q_DEBUG("QSocketPool: init client_socket error!");
					continue;
					//return -3;
				}
			}

			if(heart_beat_) {
				heart_beat_timer_.set_now(heart_beat_now_);
				heart_beat_timer_.set_interval(heart_beat_interval_);
				if(heart_beat_timer_.start(heart_beat)) {
					Q_DEBUG("QSocketPool: heart beat start error!");
					return -4;
				}
			}

			return 0;
		}

		// @函数名: 心跳机制函数
		static void heart_beat(int32_t signo)
		{
			char heart_beat_send[12];
			char heart_beat_recv[12];

			*(uint64_t*)(heart_beat_send)=*(uint64_t*)"^-^!^-^!";
			*(uint32_t*)(heart_beat_send+8)=0xBBBBBBBB;

			for(int32_t i=0; i<call_back_->now_size_; ++i)
			{
				try {
					if(call_back_->socket_pool_[i]->is_dead_)
						throw -1;

					if(call_back_->socket_pool_[i]!=NULL && call_back_->socket_pool_[i]->is_idle() && !call_back_->socket_pool_[i]->is_dead())
					{
						if(call_back_->socket_pool_[i]->send_data(heart_beat_send, sizeof(heart_beat_send)))
							throw -2;

						if(call_back_->socket_pool_[i]->recv_data(heart_beat_recv, sizeof(heart_beat_recv)))
							throw -3;

						if(*(uint64_t*)heart_beat_recv!=*(uint64_t*)"^-^!^-^!")
							throw -4;

						if(*(uint64_t*)(heart_beat_recv+8)!=0xEEEEEEEE)
							throw -5;

						Q_DEBUG("----- HEART BEAT: i = %02d (OK)", i);
					}
				} catch(const int32_t err) {
					call_back_->socket_pool_[i]->close();

					Q_DEBUG("----- HEART BEAT: i = %02d (BAD), now reconnect......", i);
					if(call_back_->socket_pool_[i]->init(call_back_->server_ip_, call_back_->server_port_, call_back_->timeout_)) {
						Q_DEBUG("----- HEART BEAT: i = %02d (BAD), reconnect error!", i);
					} else {
						Q_DEBUG("----- HEART BEAT: i = %02d (OK)", i);
					}
				}
			}
		}

	protected:
		char			server_ip_[16];		// socket服务器IP地址
		uint16_t		server_port_;		// socket服务端最大支持连接数
		int32_t			timeout_;		// socket服务端超时时间

		int32_t			now_size_;		// socket连接池实际连接数
		int32_t			max_size_;		// socket连接池支持的最大连接数

		bool8_t			heart_beat_;		// 是否激活心跳包机制
		int32_t			heart_beat_now_;	// 心跳起始时间
		int32_t			heart_beat_interval_;	// 心跳间隔时间
		QWatchdog		heart_beat_timer_;	// 心跳计时器

		QClientSocket**		socket_pool_;		// socket连接池队列信息
		QMutexLock		mutex_;			// socket连接池互斥锁

		static QSocketPool*	call_back_;		// QSocketPool类回调静态指针成员
};

Q_END_NAMESPACE

#endif // __QSOCKETPOOL_H_
