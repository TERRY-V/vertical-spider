#include "qmysqlclient.h"

Q_BEGIN_NAMESPACE

QMySQLClient* QMySQLClient::connectionPool=NULL;

QMySQLClient* QMySQLClient::getInstance(const char* url, const char* user, const char* passwd)
{
	if(NULL==connectionPool)
		connectionPool=new(std::nothrow) QMySQLClient(url, user, passwd, 25);
	return connectionPool;
}

sql::Connection* QMySQLClient::getConnection()
{
	sql::Connection* conn=NULL;
	mMutex.lock();
	if(connList.size()) {
		conn=connList.front();
		connList.pop_front();
		// 如果连接已经关闭，删除后重新建立一个连接
		if(conn->isClosed()) {
			delete conn;
			conn=this->createConnection();
		}
		if(NULL==conn) --curSize;
		mMutex.unlock();
		return conn;
	} else {
		if(curSize<maxSize) {
			// 尚且可以创建新的连接
			conn=this->createConnection();
			if(conn) {
				++curSize;
				mMutex.unlock();
				return conn;
			} else {
				mMutex.unlock();
				return NULL;
			}
		} else {
			// 建立的连接数已经达到maxSize了
			mMutex.unlock();
			return NULL;
		}
	}
}

void QMySQLClient::releaseConnection(sql::Connection *conn)
{
	if(conn) {
		mMutex.lock();
		connList.push_back(conn);
		mMutex.unlock();
	}
}

QMySQLClient::QMySQLClient(const std::string& url, const std::string& user, const std::string& passwd, int32_t maxSize)
{
	this->url=url;
	this->user=user;
	this->passwd=passwd;
	this->maxSize=maxSize;
	this->curSize=0;
	try {
		this->driver=get_driver_instance();
	} catch(sql::SQLException& e) {
		std::cout<<"# ERR: SQLException in "<<__FILE__;
		std::cout<<"("<<__FUNCTION__<<") on line "<<__LINE__<<std::endl;
		std::cout<<"# ERR: "<<e.what();
		std::cout<<" (MYSQL error code: "<<e.getErrorCode();
		std::cout<<", SQLState "<<e.getSQLState()<<" )"<<std::endl;
	}
	this->initConnection(maxSize);
}

QMySQLClient::~QMySQLClient()
{
	this->destroyQMySQLClient();
}

void QMySQLClient::initConnection(int32_t initialSize)
{
	sql::Connection* conn=NULL;
	mMutex.lock();
	for(int32_t i=0; i!=initialSize; ++i) {
		conn=this->createConnection();
		if(conn) {
			connList.push_back(conn);
			++(this->curSize);
		} else {
			std::cout<<"Creating MySQL connection failed!"<<std::endl;
		}
	}
	mMutex.unlock();
}

sql::Connection* QMySQLClient::createConnection()
{
	sql::Connection* conn=NULL;
	try {
		// 创建MySQL连接
		conn=driver->connect(this->url, this->user, this->passwd);
		return conn;
	} catch(sql::SQLException& e) {
		return NULL;
	}
}

void QMySQLClient::destroyQMySQLClient()
{
	mMutex.lock();
	for(std::list<sql::Connection*>::iterator iter=connList.begin(); iter!=connList.end(); ++iter) {
		sql::Connection* conn=*iter;
		if(conn) {
			try {
				conn->close();
				delete conn;
			} catch(sql::SQLException& e) {
				std::cout<<"Close connection failed!"<<std::endl;
			}
		}
	}
	curSize=0;
	connList.clear();
	mMutex.unlock();
}

Q_END_NAMESPACE
