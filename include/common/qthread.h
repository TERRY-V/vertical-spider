/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qthread.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2015/01/29
**
*********************************************************************************************/

#ifndef __QTHREAD_H_
#define __QTHREAD_H_

#include "qglobal.h"

Q_BEGIN_NAMESPACE

#ifdef WIN32
typedef int32_t pthread_id_t;
typedef HANDLE pthread_t;
#else
typedef pthread_t pthread_id_t;
#endif

class QThread {
	public:
		inline QThread(bool iautofree=false) :
#ifdef WIN32
			id(0),
#endif
			handle(0),
			autofree(iautofree),
			running(0),
			signaled(0),
			finished(0),
			reserved(0),
			relaxsem(0)
		{}

		virtual ~QThread()
		{
#ifdef WIN32
			// MSDN states this is not necessary, however, without closing the handle 
			// debuggers shows an obvious handle leak here
			if(autofree)
				CloseHandle(handle);
#else
			// Though we require non-autofree threads to always call waitfor(), the 
			// statement below is provided to cleanup thread resources even if waitfor()
			// was not called.
			if(!autofree&&running)
				pthread_detach(handle);
#endif
		}

		pthread_id_t get_id()
		{
#ifdef WIN32
			return static_cast<int32_t>(id);
#else
			return handle;
#endif
		}

		bool get_running()
		{return running!=0;}

		bool get_finished()
		{return finished!=0;}

		bool get_signaled()
		{return signaled!=0;}

		int32_t start()
		{
#ifdef WIN32
			handle=(HANDLE)_beginthreadex(NULL, 0, _threadproc, this, 0, &id);
			if(handle==0) {
				Q_FATAL("QThread: CreateThread() failed!");
				return -1;
			}
#else
			pthread_t temp_handle;
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, autofree?PTHREAD_CREATE_DETACHED:PTHREAD_CREATE_JOINABLE);
			if(pthread_create(autofree?&temp_handle:&handle, &attr, _threadproc, this)!=0) {
				Q_FATAL("QThread: CreateThread() failed!");
				return -1;
			}
			pthread_attr_destroy(&attr);
#endif
			return 0;
		}

		void signal()
		{
			signaled=1;
			relaxsem.post();
		}

		void waitfor()
		{
			if(autofree)
				Q_FATAL("Can not waitfor() on an autofree thread!");
#ifdef WIN32
			WaitForSingleObject(handle, INFINITE);
			CloseHandle(handle);
#else
			pthread_join(handle, NULL);
#endif
			handle=0;
		}

	protected:
		virtual void execute()=0;

		virtual void cleanup()
		{}

		bool relax(int32_t msecs)
		{return relaxsem.wait(msecs);}

#ifdef WIN32
		static uint32_t __stdcall _threadproc(void* arg)
		{
#else
		static void* _threadproc(void* arg)
		{
#endif
			QThread* ptr_this=reinterpret_cast<QThread*>(arg);

			try {
				ptr_this->execute();
			} catch(...) {
				ptr_this->_threadepilog();
				throw;
			}

			ptr_this->_threadepilog();
			return 0;
		}

		void _threadepilog()
		{
			cleanup();

			finished=1;
			if(autofree)
				this->~QThread();
		}

	protected:
#ifdef WIN32
		uint32_t id;
#endif
		pthread_t handle;
		int32_t autofree;
		int32_t running;
		int32_t signaled;
		int32_t finished;
		int32_t reserved;		// for priorities
		QTimedSem relaxsem;
};

Q_END_NAMESPACE

#endif // __QTHREAD_H_
