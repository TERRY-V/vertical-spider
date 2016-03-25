/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filestr:	qlogger.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2013/12/06
**
*********************************************************************************************/

#ifndef __QLOGGER_H_
#define __QLOGGER_H_

#include "qglobal.h"

Q_BEGIN_NAMESPACE

#define LEVEL_DEBUG		(0x00)
#define LEVEL_INFO		(0x01)
#define LEVEL_ALERT		(0x02)
#define LEVEL_WARNING		(0x03)
#define LEVEL_ERROR		(0x04)

#define DEFAULT_LOG_PATH_SIZE	(1<<8)
#define DEFAULT_LOG_PREFIX_SIZE	(1<<5)

#define DEFAULT_LOG_PATH	("../log")
//#define DEFAULT_LOG_PATH	(NULL)
#define DEFAULT_LOG_PREFIX	(NULL)
#define DEFAULT_LOG_MESSAGE_SIZE (1<<10)

#define Q_LOGGER		QLogger::getInstance()->log
#define Q_LOG_LEVEL(level)	LEVEL_##level, __FILE__, __LINE__, __FUNCTION__

#define Q_LOG(level, ...) 	Q_LOGGER(Q_LOG_LEVEL(level), 1, __VA_ARGS__)

#define Q_LOG_DEBUG(...)	Q_LOGGER(Q_LOG_LEVEL(DEBUG), 1, __VA_ARGS__)
#define Q_LOG_INFO(...)		Q_LOGGER(Q_LOG_LEVEL(INFO), 1, __VA_ARGS__)
#define Q_LOG_ALERT(...)	Q_LOGGER(Q_LOG_LEVEL(ALERT), 1, __VA_ARGS__)
#define Q_LOG_WARNING(...)	Q_LOGGER(Q_LOG_LEVEL(WARNING), 1, __VA_ARGS__)
#define Q_LOG_ERROR(...)	Q_LOGGER(Q_LOG_LEVEL(ERROR), 1, __VA_ARGS__)

struct QLoggerInfo {
	int32_t			level;
	const char*		str;
	const char*		summary;
	int32_t			group;
	const char*		since;
};

class QLogger: public noncopyable {
	public:
		static QLogger* getInstance(const char* logPath=DEFAULT_LOG_PATH, const char* prefix=DEFAULT_LOG_PREFIX, uint32_t messageSize=DEFAULT_LOG_MESSAGE_SIZE);

		static void release();

		QLogger();

		virtual ~QLogger();

		int32_t init(const char* logPath, const char* logPrefix, uint32_t messageSize);

		int32_t log(int32_t level, const char* file, int32_t line, const char* func, bool bPrintScreen, const char* format, ...);

	private:
		int32_t mkdir(const char* dirPath);

	protected:
#ifdef __multi_thread
		QMutexLock	logMutex_;
#endif
		static QLogger*	logger_;

		char*		message_;
		uint32_t	messageSize_;

		FILE*		logFp_;
		int32_t		dayNow_;

		char		logPath_[DEFAULT_LOG_PATH_SIZE];
		char		logPrefix_[DEFAULT_LOG_PREFIX_SIZE];
		char		logFilePath_[DEFAULT_LOG_PATH_SIZE];
};

Q_END_NAMESPACE

#endif // __QLOGGER_H_
