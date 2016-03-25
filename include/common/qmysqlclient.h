/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qmysqlclient.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2013/11/06
**
*********************************************************************************************/

#ifndef __QMYSQLCLIENT_H_
#define __QMYSQLCLIENT_H_

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include "qglobal.h"

Q_BEGIN_NAMESPACE

// MySQL数据库客户端连接池类
class QMySQLClient: public noncopyable {
	public:
		// @函数名: 获取MySQL数据库连接池对象
		// @参数01: MySQL数据库连接的url
		// @参数02: MySQL数据库用户名
		// @参数03: MySQL数据库密码
		// @返回值: MySQL数据库连接池对象
		static QMySQLClient *getInstance(const char* url, const char* user, const char* passwd);

		// @函数名: 获取MySQL数据库连接
		sql::Connection* getConnection();

		// @函数名: 回收MySQL数据库连接
		void releaseConnection(sql::Connection *conn);

	private:
		// @函数名: QMySQLClient构造函数
		// @参数01: MySQL数据库连接的url
		// @参数02: MySQL数据库用户名
		// @参数03: MySQL数据库密码
		// @参数04: MySQL数据库最大连接数
		explicit QMySQLClient(const std::string& url, const std::string& user, const std::string& passwd, int32_t maxSize);

		// @函数名: 析构函数
		virtual ~QMySQLClient();

		// @函数名: 初始化MySQL连接池, 创建连接对象
		// @参数01: MySQL数据库连接数
		void initConnection(int32_t initialSize);

		// @函数名: 创建MySQL数据库连接
		// @返回值: MySQL数据库连接对象
		sql::Connection* createConnection();

		// @函数名: 销毁MySQL数据库连接池
		void destroyQMySQLClient();

	protected:
		// 当前MySQL数据库连接数
		int32_t curSize;
		// 最大MySQL数据库连接数
		int32_t maxSize;
		// MySQL数据库连接IP地址和端口号
		std::string url;
		// MySQL数据库用户名
		std::string user;
		// MySQL数据库密码
		std::string passwd;
		// MySQL连接池容器链表
		std::list<sql::Connection*> connList;
		// 线程锁
		QMutexLock mMutex;
		// MySQL数据库连接池对象
		static QMySQLClient *connectionPool;
		// MySQL数据库驱动
		sql::Driver *driver;
};

Q_END_NAMESPACE

#endif // __QMYSQLCLIENT_H_
