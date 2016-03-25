/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qredisclient.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/12/12
**
*********************************************************************************************/

#ifndef __QREDISCLIENT_H_
#define __QREDISCLIENT_H_

#include <hiredis/hiredis.h>
#include "qglobal.h"

Q_BEGIN_NAMESPACE

#define DEFAULT_REDIS_HOST	("localhost")
#define DEFAULT_REDIS_PORT	(6379)

// Redis is an open source, BSD licensed, advanced key-value cache and store. It is often 
// referred to as a data structure server since keys can contain strings, hashes, lists, sets, 
// sorted sets, bitmaps and hyperloglogs.

// How to install redis server on ubuntu linux?
// apt-get install redis-server

// How to install the development packet on ubuntu linux?
// apt-get install libhiredis-dev

// How to compile the redis client?
// -lhiredis

class QRedisClient: public noncopyable {
	public:
		inline QRedisClient() :
			connect_(0)
		{}

		virtual ~QRedisClient()
		{
			if(connect_) {
				redisFree(connect_);
				connect_ = NULL;
			}
		}

		// @函数名: redis服务器连接函数
		// @参数01: redis服务器IP地址
		// @参数02: redis服务器端口号地址, 默认为6379
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t connect(const char* host = DEFAULT_REDIS_HOST, int16_t port = DEFAULT_REDIS_PORT)
		{
			// redis连接超时时间(默认为1.5秒)
			struct timeval timeout = {1, 500000};
#if 0
			connect_ = redisConnect(host, port);
#else
			connect_ = redisConnectWithTimeout(host, port, timeout);
#endif
			if(connect_==NULL||connect_->err) {
				if(connect_) {
					Q_INFO("QRedisClient: connection error, (%s)", connect_->errstr);
					redisFree(connect_);
				} else {
					Q_INFO("QRedisClient: connection error, can't allocate redis context!");
				}
				return -1;
			}
			return 0;
		}

		// PING server
		int32_t ping()
		{
			redisReply* reply_ = (redisReply*)redisCommand(connect_, "PING");
			if(reply_==NULL) {
				redisFree(connect_);
				return -1;
			}

			Q_INFO("PING: %s", reply_->str);

			freeReplyObject(reply_);
			return 0;
		}

		// Set a key
		int32_t set(const char* key, const char* value)
		{
			redisReply* reply_ = (redisReply*)redisCommand(connect_, "SET %s %s", key, value);
			if(reply_ == NULL) {
				redisFree(connect_);
				return -1;
			}

			if(!(reply_->type == REDIS_REPLY_STATUS && !strcasecmp(reply_->str, "OK"))) {
				freeReplyObject(reply_);
				redisFree(connect_);
				return -2;
			}

			freeReplyObject(reply_);
			return 0;
		}

		// Get a key
		std::string get(const char* key)
		{
			redisReply* reply_ = (redisReply*)redisCommand(connect_, "GET %s", key);
			std::string str = reply_->str;
			freeReplyObject(reply_);
			return str;
		}

		// exists a key
		int32_t exists(const char* key)
		{
			redisReply* reply_ = (redisReply*)redisCommand(connect_, "exists %s", key);
			if(reply_ == NULL) {
				redisFree(connect_);
				return -1;
			}

			if(reply_->integer==0)
				return -2;

			freeReplyObject(reply_);
			return 0;
		}

		// del a key
		int32_t del(const char* key)
		{
			redisReply* reply_ = (redisReply*)redisCommand(connect_, "del %s", key);
			if(reply_ == NULL) {
				redisFree(connect_);
				return -1;
			}

			freeReplyObject(reply_);
			return 0;
		}

		// Set expire
		int32_t expire(const char* key, int32_t timeout)
		{
			redisReply* reply_ = (redisReply*)redisCommand(connect_, "expire %s %d", key, timeout);
			if(reply_ == NULL) {
				redisFree(connect_);
				return -1;
			}

			if(!(reply_->type == REDIS_REPLY_STATUS && !strcasecmp(reply_->str, "OK"))) {
				freeReplyObject(reply_);
				redisFree(connect_);
				return -2;
			}

			freeReplyObject(reply_);
			return 0;
		}

		// Set a query
		int32_t query(const char* command)
		{
			redisReply* reply_ = (redisReply*)redisCommand(connect_, command);
			if(reply_ == NULL) {
				redisFree(connect_);
				return -1;
			}

			if(!(reply_->type == REDIS_REPLY_STATUS && !strcasecmp(reply_->str, "OK"))) {
				freeReplyObject(reply_);
				redisFree(connect_);
				return -2;
			}

			freeReplyObject(reply_);
			return 0;
		}

	protected:
		redisContext* connect_;
};

Q_END_NAMESPACE

#endif // __QREDISCLIENT_H_
