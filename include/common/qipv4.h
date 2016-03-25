/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qipv4.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/08/15
**
*********************************************************************************************/

#ifndef __QIPV4_H_
#define __QIPV4_H_

#include "qglobal.h"

Q_BEGIN_NAMESPACE

// IP地址
class QIPv4Addr
{
	public:
		inline QIPv4Addr() : 
			ldata(0)
		{}

		inline QIPv4Addr(uint32_t ldata_) : 
			ldata(ldata_)
		{}

		inline QIPv4Addr(const char* ip_)
		{
			int32_t a, b, c, d;
			sscanf(ip_, "%d.%d.%d.%d", &a, &b, &c, &d);
			data[0]=uint8_t(a);
			data[1]=uint8_t(b);
			data[2]=uint8_t(c);
			data[3]=uint8_t(d);
		}

		inline QIPv4Addr(const QIPv4Addr& ip_)
		{ldata=ip_.ldata;}

		inline void set_ip(const char* ip_)
		{
			int32_t a, b, c, d;
			sscanf(ip_, "%d.%d.%d.%d", &a, &b, &c, &d);
			data[0]=uint8_t(a);
			data[1]=uint8_t(b);
			data[2]=uint8_t(c);
			data[3]=uint8_t(d);
		}

		inline void set_ip(uint32_t ldata_)
		{ldata=ldata_;}

		inline uint32_t get_ip() const
		{return ldata;}

		inline std::string to_string() const
		{
			char tmp[16]={0};
			q_snprintf(tmp, sizeof(tmp), "%d:%d:%d:%d", (uint8_t)data[0], (uint8_t)data[1], (uint8_t)data[2], (uint8_t)data[3]);
			return tmp;
		}

		uint8_t& operator[](int32_t i)
		{return data[i];}

	protected:
		union {
			uint8_t data[4];
			uint32_t ldata;
		};
};

// IP peer
class QIPv4Peer: public noncopyable
{
	public:
		inline QIPv4Peer() :
			ip_(uint32_t(0xffffffff)),
			host_(),
			port_(0)
		{}

		QIPv4Peer(QIPv4Addr ip, const char* host, int32_t port) :
			ip_(ip),
			host_(host),
			port_(port)
		{}

		void set_ip(QIPv4Addr ip)
		{ip_=ip;}

		void set_host(const std::string& host)
		{host_=host;}

		void set_port(int32_t port)
		{port_=port;}

		QIPv4Addr get_ip()
		{
			if(ip_.get_ip()==(uint32_t)0xffffffff && !host_.empty())
				ip_=get_host_by_name(host_);
			return ip_;
		}

		std::string get_host()
		{
			if(!host_.empty())
				return host_;
			host_=get_host_by_addr(ip_);
			return host_;
		}

		int32_t get_port() const
		{return port_;}

		std::string to_string() const
		{
			char str[1<<7]={0};
			if(!host_.empty())
				q_snprintf(str, sizeof(str)-1, "%s:%d", host_.c_str(), port_);
			else
				q_snprintf(str, sizeof(str)-1, "%s:%d", ip_.to_string().c_str(), port_);
			return str;
		}

	private:
		QIPv4Addr get_host_by_name(const std::string& host)
		{
			QIPv4Addr ip;
			/*
			 * hostent结构体分析
			 *
			 * struct hostent {
			 * 	char* h_name;		// 正式的主机名称
			 * 	char** h_aliases;	// 指向主机名称的其它别名
			 * 	int h_addrtype;		// 地址的类型, 通常是AF_INET
			 * 	int h_length;		// 地址的长度
			 * 	char** h_addr_list;	// 从名称服务器取得该主机的所有地址
			 * };
			 *
			 */
			struct hostent* hp=::gethostbyname(host.c_str());
			if(hp==NULL) {
				switch(errno) {
					case HOST_NOT_FOUND:
						Q_DEBUG("QIPv4Peer: get_host_by_name error, host not found!");
						break;
					case NO_ADDRESS:
						Q_DEBUG("QIPv4Peer: get_host_by_name error, no ip address!");
						break;
					default:
						Q_DEBUG("QIPv4Peer: get_host_by_name unknown error!");
						break;
				}
			}
			if(hp&&hp->h_addrtype==AF_INET)
				ip.set_ip(inet_ntoa(*(struct in_addr*)hp->h_addr_list[0]));
			return ip;
		}

		std::string get_host_by_addr(QIPv4Addr ip)
		{
			std::string host;
			uint32_t data=ip_.get_ip();
			struct hostent* hp=::gethostbyaddr((char*)&data, sizeof(data), AF_INET);
			if(hp==NULL) {
				switch(errno) {
					case HOST_NOT_FOUND:
						Q_DEBUG("QIPv4Peer: get_host_by_addr error, host not found!");
						break;
					case NO_ADDRESS:
						Q_DEBUG("QIPv4Peer: get_host_by_addr error, no ip address!");
						break;
					default:
						Q_DEBUG("QIPv4Peer: get_host_by_addr unknown error!");
						break;
				}
			} else {
				host=hp->h_name;
			}
			return host;
		}

	protected:
		QIPv4Addr ip_;
		std::string host_;
		int32_t port_;
};

Q_END_NAMESPACE

#endif // __QIPV4_H_
