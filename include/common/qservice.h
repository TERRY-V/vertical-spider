/********************************************************************************************
 **
 ** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
 ** Filename:	qservice.h
 ** Author:	TERRY-V
 ** Email:	cnbj8607@163.com
 ** Support:	http://blog.sina.com.cn/terrynotes
 ** Date:	2014/10/29
 **
 *********************************************************************************************/

#ifndef __QSERVICE_H_
#define __QSERVICE_H_

#include "qglobal.h"

#define SERVICE_PREFIX ((char*)"niuxl")

#define SERVICE_TYPE_COMMON ((char*)"common process")
#define SERVICE_TYPE_MASTER ((char*)"master process")
#define SERVICE_TYPE_WORKER ((char*)"worker process")

#define SERVICE_PIDFILE ((char*)"/var/run/niuxl.pid")

Q_BEGIN_NAMESPACE

// 服务抽象类, 实例化时必须继承此类
class QService {
	public:
		QService();

		virtual ~QService();

		int32_t main(int32_t argc, char** argv);

	private:
		int32_t daemonize();

		void createPidFile(void);

		int32_t setproctitle(int32_t argc, char** argv, char* prefix=SERVICE_PREFIX, char* type=SERVICE_TYPE_WORKER);

	protected:
		virtual int32_t init()=0;

		virtual int32_t run(int32_t argc, char** argv)=0;

		virtual int32_t destroy()=0;

		virtual void author();

		virtual void help();

		virtual void version();
};

Q_END_NAMESPACE

#endif // __QSERVICE_H_
