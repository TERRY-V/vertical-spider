/********************************************************************************************
**
** Copyright (C) 2010-2015 Terry Niu (Beijing, China)
** Filename:	qmemcachedclient.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2015/04/06
**
*********************************************************************************************/

#ifndef __QMEMCACHEDCLIENT_H_
#define __QMEMCACHEDCLIENT_H_

#include <libmemcached/memcached.h>
#include "qglobal.h"

Q_BEGIN_NAMESPACE

// libMemcached is an open source C/C++ client library and tools for the memcached server (http://memcached.org/).
// It has been designed to be light on memory usage, thread safe, and provide full access to server side methods.
//
// http://docs.libmemcached.org/
//
// How to install memcached server on ubuntu linux?
// (apt-get install memcached)
//
// How to connect memcached server with C/C++ api?
// (apt-get install libmemcached-dev, -lmemcached)
//
// How to start memcached server?
// memcached -d -m 128 -p 11111 -u root
//
// -p 监听端口
// -l 连接的IP地址, 默认是本机
// -d start 启动memcached服务
// -d restart 重启memcached服务
// -d stop|shutdown 关闭正在运行的memcached服务
// -d install 安装memcached服务
// -d uninstall 卸载memcached服务
// -u 以用户身份运行 (仅在以root运行的时候有效)
// -m 最大内存使用, 单位MB, 默认64MB
// -M 内存耗尽时返回错误, 而不是删除项
// -c 最大同时连接数, 默认是1024
// -f 块大小增长因子, 默认是1.25-n 最小分配空间，key+value+flags默认是48
// -h 显示帮助

// 分布式缓存之KV数据库memcache
class QMemcachedClient: public noncopyable {
	public:
		inline QMemcachedClient() :
			memcache(NULL),
			servers(NULL)
		{}

		virtual ~QMemcachedClient()
		{
			memcached_free(memcache);
			memcached_server_list_free(servers);
		}

		// @函数名: 初始化函数
		int32_t init()
		{
			memcache=memcached_create(NULL);
			if(memcache==NULL) {
				Q_DEBUG("QMemcachedClient: memcached_create error!");
				return -1;
			}
			return 0;
		}

		// @函数名: 设置ip和端口号(分布式缓存)
		int32_t server_append(const char* ip, uint16_t port)
		{
			memcached_return rc=MEMCACHED_SUCCESS;

			servers=memcached_server_list_append(NULL, ip, port, &rc);
			if(servers==NULL) {
				Q_DEBUG("QMemcachedClient: memcached_server_list_append error!");
				return -1;
			}

			rc=memcached_server_push(memcache, servers);
			if(rc!=MEMCACHED_SUCCESS) {
				Q_DEBUG("QMemcachedClient: memcached_server_push error (%s)!", memcached_strerror(memcache, rc));
				return -2;
			}

			return 0;
		}

		// @函数名: 服务器清零
		int32_t server_reset_all()
		{
			memcached_servers_reset(memcache);
			return 0;
		}

		// @函数名: 获取服务器数量
		uint32_t server_count()
		{
			return memcached_server_list_count(servers);
		}

		// @函数名: KV设置函数
		int32_t set_Key_Value(const char* key, int32_t key_len, const char* value, int32_t value_len)
		{
			if(key==NULL||value==NULL||key_len<=0||value_len<=0)
				return -1;

			memcached_return rc=MEMCACHED_SUCCESS;
			rc=memcached_set(memcache, key, key_len, value, value_len, 0, 0);
			if(rc!=MEMCACHED_SUCCESS) {
				Q_DEBUG("QMemcachedClient: memcached_set error (%s)!", memcached_strerror(memcache, rc));
				return -2;
			}

			return 0;
		}

		// @函数名: KV获取函数
		int32_t get_Key_Value(const char* key, int32_t key_len, char*& value, int32_t& value_len)
		{
			if(key==NULL||key_len<=0)
				return -1;

			memcached_return rc=MEMCACHED_SUCCESS;
			size_t mem_value_len=0;

			value=memcached_get(memcache, key, key_len, &mem_value_len, 0, &rc);
			if(rc!=MEMCACHED_SUCCESS) {
				Q_DEBUG("QMemcachedClient: memcached_get error (%s)!", memcached_strerror(memcache, rc));
				return -2;
			}

			value_len=static_cast<int32_t>(mem_value_len);
			return 0;
		}

		// @函数名: KV更新函数
		int32_t replace_Key_Value(const char* key, int32_t key_len, const char* value, int32_t value_len)
		{
			if(key==NULL||value==NULL||key_len<=0||value_len<=0)
				return -1;

			memcached_return rc=MEMCACHED_SUCCESS;
			rc=memcached_replace(memcache, key, key_len, value, value_len, 0, 0);
			if(rc!=MEMCACHED_SUCCESS) {
				Q_DEBUG("QMemcachedClient: memcached_replace error (%s)!", memcached_strerror(memcache, rc));
				return -2;
			}

			return 0;
		}

		// @函数名: KV删除函数
		int32_t delete_Key(const char* key, int32_t key_len)
		{
			if(key==NULL||key_len<=0)
				return -1;

			memcached_return rc=MEMCACHED_SUCCESS;
			rc=memcached_delete(memcache, key, key_len, 0);
			if(rc!=MEMCACHED_SUCCESS) {
				Q_DEBUG("QMemcachedClient: memcached_delete error (%s)!", memcached_strerror(memcache, rc));
				return -2;
			}

			return 0;
		}

	protected:
		memcached_st* memcache;
		memcached_server_st* servers;
};

Q_END_NAMESPACE

#endif // __QMEMCACHEDCLIENT_H_
