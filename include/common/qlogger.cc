#include "qlogger.h"

Q_BEGIN_NAMESPACE

struct QLoggerInfo loggerInfo[] = {
	{0x00, "DEBUG", NULL, 1, "1.0.0"},
	{0x01, "INFO", NULL, 1, "1.0.0"},
	{0x02, "ALERT", NULL, 1, "2.0.0"},
	{0x03, "WARNING", NULL, 1, "1.0.0"},
	{0x04, "ERROR", NULL, 1, "1.0.0"}
};

QLogger* QLogger::logger_=NULL;

QLogger* QLogger::getInstance(const char* logPath, const char* prefix, uint32_t messageSize)
{
	if(logger_==NULL) {
		if((logger_=new QLogger())==NULL)
			return NULL;

		if(logger_->init(logPath, prefix, messageSize))
			return NULL;
	}

	return logger_;
}

void QLogger::release()
{
	if(logger_!=NULL) {
		delete logger_;
		logger_=NULL;
	}
}

int32_t QLogger::log(int32_t level, const char* file, int32_t line, const char* func, bool bPrintScreen, const char* format, ...)
{
	if(message_==NULL||messageSize_<=0)
		return -1;

#ifdef __multi_thread
	logMutex_.lock();
#endif

	va_list arg;
	va_start(arg, format);
	int32_t ret=vsnprintf(message_, messageSize_, format, arg);
	va_end(arg);
	if(ret<=0) {
#ifdef __multi_thread
		logMutex_.unlock();
#endif
		return -2;
	}

	time_t now=time(NULL);
	struct tm* ptm=localtime(&now);

	// Log for the first time?
	if(logFp_==NULL) {
		q_snprintf(logFilePath_, sizeof(logFilePath_), "%s/%s%04d-%02d-%02d.log", logPath_, logPrefix_, ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday);
		do {
			logFp_=fopen(logFilePath_, "a");
			if(logFp_)
				break;
			q_sleep(1000);
		} while(logFp_==NULL);
	}

	// A new day has come???
	if(dayNow_!=ptm->tm_mday) {
		if(logFp_) {
			fclose(logFp_);
			logFp_=NULL;
		}

		q_snprintf(logFilePath_, sizeof(logFilePath_), "%s/%s%04d-%02d-%02d.log", logPath_, logPrefix_, ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday);
		do {
			logFp_=fopen(logFilePath_, "a");
			if(logFp_)
				break;
			q_sleep(1000);
		} while(logFp_==NULL);

		dayNow_=ptm->tm_mday;
	}

	// Write log
	if(logFp_!=NULL) {
#ifdef __multi_thread
		fprintf(logFp_, "[%04d-%02d-%02d %02d:%02d:%02d] TID = %lu %-5s: %s (%s:%d) %s\n", \
				ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, \
				q_thread_id(), \
				loggerInfo[level].str, \
				func, \
				file, \
				line, \
				message_);
#else
		fprintf(logFp_, "[%04d-%02d-%02d %02d:%02d:%02d] %-5s: %s (%s:%d) %s\n", \
				ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, \
				loggerInfo[level].str, \
				func, \
				file, \
				line, \
				message_);
#endif
		fflush(logFp_);
		ret=0;
	} else {
		ret=-1;
	}

	// Print screen?
	if(bPrintScreen) {
#if 0
		Q_DEBUG("[%04d-%02d-%02d %02d:%02d:%02d] %-5s %s (%s:%d) %s", \
				ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec,
				loggerInfo[level].str, func, file, line, message_);
#else
		Q_INFO("%s (%s:%d) %s", func, file, line, message_);
#endif
	}

#ifdef __multi_thread
	logMutex_.unlock();
#endif
	return ret;
}

QLogger::QLogger() : 
	message_(NULL), 
	messageSize_(0), 
	logFp_(NULL), 
	dayNow_(0)
{
	logPath_[0]=0;
	logPrefix_[0]=0;
	logFilePath_[0]=0;
}

QLogger::~QLogger()
{
	if(message_!=NULL)
		q_delete_array<char>(message_);

	if(logFp_!=NULL)
		fclose(logFp_);
}

int32_t QLogger::init(const char* logPath, const char* logPrefix, uint32_t messageSize)
{
	messageSize_=messageSize;
	message_=q_new_array<char>(messageSize_);
	if(message_==NULL)
		return -1;

	time_t now=time(NULL);
	struct tm* ptm=localtime(&now);

	dayNow_=ptm->tm_mday;

	if(logPath) {
		int32_t len=strlen(logPath);
		if(len==0) {
			strcpy(logPath_, ".");
		} else if(logPath[len-1]=='/') {
			strncpy(logPath_, logPath, len-1);
			logPath_[len-1]='\0';
		} else {
			strcpy(logPath_, logPath);
		}
	} else {
		strcpy(logPath_, ".");
	}

	if(mkdir(logPath_))
		return -2;

	if(logPrefix) {
		int32_t len=strlen(logPrefix);
		if(len>0) {
			if(logPrefix[len-1]=='_') {
				strcpy(logPrefix_, logPrefix);
			} else {
				q_snprintf(logPrefix_, sizeof(logPrefix_), "%s_", logPrefix);
			}
		}
	}

	return 0;
}

int32_t QLogger::mkdir(const char* dirPath)
{
	char dirName[DEFAULT_LOG_PATH_SIZE]={0};
	char* dirNamePtr=dirName;

	char* dirPathPtr=(char*)dirPath;
	int32_t len=(int32_t)::strlen(dirPath);

	if(*dirPathPtr=='\\'||*dirPathPtr=='/')
		*dirNamePtr++=*dirPathPtr++;
	if(*dirPathPtr=='\0')
		return 0;

	Q_FOREVER {
		while(*dirPathPtr) {
			if(*dirPathPtr=='\\'||*dirPathPtr=='/')
				break;
			*dirNamePtr++=*dirPathPtr++;
		}
		*dirNamePtr='\0';

#ifdef WIN32
		if(_mkdir(dirName)) {
			if(errno!=EEXIST)
				return -1;
		}
#else
		if(::mkdir(dirName, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH)) {
			if(errno!=EEXIST)
				return -1;
		}
#endif

		if(*dirPathPtr)
			*dirNamePtr++=*dirPathPtr++;

		if(dirPathPtr-dirPath>=len)
			break;
	}
	return 0;
}

Q_END_NAMESPACE
