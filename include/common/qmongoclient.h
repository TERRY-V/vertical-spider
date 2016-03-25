/********************************************************************************************
**
** Copyright (C) 2010-2015 Terry Niu (Beijing, China)
** Filename:	qmongoclient.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2016/02/23
**
*********************************************************************************************/

#ifndef __QMONGOCLIENT_H_
#define __QMONGOCLIENT_H_

#include "qglobal.h"
#include "qdatetime.h"
#include "qfunc.h"
#include "mongo/client/dbclient.h"
#include "boost/date_time/posix_time/posix_time.hpp"

#define MONGO_OK  (0)
#define MONGO_ERR (-1)

#define MONGO_DEFAULT_URI ("mongodb://localhost:27017")

using namespace mongo;
using namespace boost::posix_time;

Q_BEGIN_NAMESPACE

// MongoDB客户端
// https://github.com/mongodb/mongo-cxx-driver/wiki/Tutorial
// yum install boost boost-devel
// 编译时需要加参数-I ~/mongo-client-install/include/ ~/mongo-client-install/lib/libmongoclient.a -lboost_system -lboost_thread -lboost_regex
class QMongoClient {
	public:
		// @函数名: Mongo客户端构造函数
		QMongoClient(const char* uri = MONGO_DEFAULT_URI);

		// @函数名: Mongo客户端析构函数
		virtual ~QMongoClient();

		// @函数名: 设置集合名称
		void setCollection(const char* collection);

		// @函数名: 获取集合名称
		const char* getCollection() const;

		// @函数名: 查询文档(全遍历)
		int32_t selectAll();

		// @函数名: 插入文档(键值对)
		int32_t insert(uint64_t key, const char* value);

		// @函数名: 统计集合总文档数
		unsigned long long count();

		// @函数名: 删除集合
		int32_t dropCollection();

		// @函数名: 查询图片ID是否存在
		bool exists(const char* imgid);

		// @函数名: 查询图片并获取图片信息
		bool select(const char* imgid, std::string& imgInfo);

		// @函数名: 新增图片
		int32_t insert(const char* imgid, const char* imgInfo);

		// @函数名: 图片更新函数
		int32_t update(const char* imgid, const char* imgInfo);

		// @函数名: 删除图片函数
		int32_t remove(const char* imgid);

		// @函数名: 创建图片字段索引
		int32_t createIndex();

	private:
		// @函数名: 获取datetime值
		unsigned long long dt(const char* time);

	protected:
		mongo::client::GlobalInstance instance_;
		mongo::ConnectionString cs_;
		mongo::DBClientBase* conn_;

		std::string	uri_;
		std::string	collection_;
		std::string	errmsg_;
};

Q_END_NAMESPACE

#endif // __QMONGOCLIENT_H_
