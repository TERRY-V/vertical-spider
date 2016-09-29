/********************************************************************************************
**
** Copyright (C) 2010-2016 Terry Niu (Beijing, China)
** Filename:	qtcpsocket.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2012/11/03
**
*********************************************************************************************/

#ifndef __SPIDERSERVER_H_
#define __SPIDERSERVER_H_

#include "qglobal.h"
#include "qalgorithm.h"
#include "qallocator.h"
#include "qconfigreader.h"
#include "qdatetime.h"
#include "qdir.h"
#include "qdiskcache.h"
#include "qfile.h"
#include "qfunc.h"
#include "qlbscheduler.h"
#include "qlogger.h"
#include "qqueue.h"
#include "qremotemonitor.h"
#include "qservice.h"
#include "qvector.h"
#include "cJSON.h"
#include "qnetworkaccessmanager.h"
#include "qmd5.h"
#include "qtcpsocket.h"
#include "md5compress.h"
#include "urlprocessor.h"
#include "tinyxml2.h"

Q_BEGIN_NAMESPACE

using namespace tinyxml2;

#define SPIDER_OK                    (0)
#define SPIDER_ERR                   (-1)

#define SPIDER_ERR_HEAP_ALLOC        (-2)

#define SPIDER_ERR_SOCKET_INIT	     (-11)
#define SPIDER_ERR_SOCKET_CONNECTION (-12)

#define SPIDER_ERR_SOCKET_ACCEPT     (-13)
#define SPIDER_ERR_SOCKET_TIMEOUT    (-14)
#define SPIDER_ERR_SOCKET_RECV       (-15)
#define SPIDER_ERR_PACKET_HEADER     (-16)
#define SPIDER_ERR_PACKET_LENGTH     (-17)
#define SPIDER_ERR_SOCKET_SEND       (-18)

#define SPIDER_ERR_SOCKET_VERSION    (-19)
#define SPIDER_ERR_PROTOCOL_TYPE     (-20)
#define SPIDER_ERR_SOURCE_TYPE       (-21)
#define SPIDER_ERR_COMMAND_TYPE      (-22)
#define SPIDER_ERR_BUFFER_SIZE       (-23)
#define SPIDER_ERR_OPERATE_TYPE      (-24)
#define SPIDER_ERR_DATA_LENGTH       (-25)
#define SPIDER_ERR_WRITE_FILE        (-26)
#define SPIDER_ERR_UNKNOWN_PROTOCOL  (-27)

#define SPIDER_DEFAULT_HZ            (10)
#define SPIDER_DEFAULT_MIN_HZ        (1)
#define SPIDER_DEFAULT_MAX_HZ        (500)

#define SPIDER_DEFAULT_PIDFILE       ("/var/run/Spider.pid")
#define SPIDER_DEFAULT_CONFIG_FILE   ("../conf/init.conf")

#define SPIDER_DEFAULT_SERVER_IP     ("127.0.0.1")
#define SPIDER_DEFAULT_SERVER_PORT   (8088)
#define SPIDER_DEFAULT_MONITOR_PORT  (8078)
#define SPIDER_DEFAULT_SEND_PORT     (11111)
#define SPIDER_DEFAULT_SERVER_TIMEOUT (10000)

#define SPIDER_DEFAULT_PROTOCOL_TYPE (1)
#define SPIDER_DEFAULT_SOURCE_TYPE   (1)
#define SPIDER_DEFAULT_COMMAND_TYPE  (1)
#define SPIDER_DEFAULT_OPERATE_TYPE  (1)

#define SPIDER_DEFAULT_INVALID_SOCKET (-1)
#define SPIDER_DEFAULT_THREAD_NUM    (1)
#define SPIDER_DEFAULT_BUFFER_SIZE   (1<<20)
#define SPIDER_DEFAULT_THREAD_TIMEOUT (12000)

#define SPIDER_DEFAULT_CHUNK_SIZE    (100*1024)

#define SPIDER_DEFAULT_QUEUE_SIZE    (200)

#define SPIDER_DEFAULT_IP_SIZE       (16)
#define SPIDER_DEFAULT_NAME_SIZE     (1<<8)
#define SPIDER_DEFAULT_PATH_SIZE     (1<<8)

#define SPIDER_DEFAULT_LOG_PATH      ("../log/")
#define SPIDER_DEFAULT_LOG_PREFIX    (NULL)
#define SPIDER_DEFAULT_LOG_SIZE      (10<<10)
#define SPIDER_DEFAULT_LOG_SCREEN    (1)

#define SPIDER_HEADER_VERSION	     (*(uint64_t*)"YST1.0.0")
#define SPIDER_TAILER_FILE_MARK	     (*(uint64_t*)"@#@#@#\r\n")

#pragma pack(1)

// 任务信息结构体
struct taskInfo {
	int32_t method;
	std::string referer;
	std::string url;
	std::string playload;
};

#pragma pack()

// SPIDER通讯服务端
class SpiderServer : public noncopyable {
	public:
		// @函数名: 构造函数
		explicit SpiderServer();

		// @函数名: 析构函数
		virtual ~SpiderServer();

		// @函数名: 初始化函数
		virtual int32_t init(const char* cfg_file);

		// @函数名: 主线程启动函数
		virtual int32_t start();

	private:
		// @函数名: 配置读取函数
		int32_t load_server_config(const char* cfg_file);

		// @函数名: 通信线程
		static Q_THREAD_T comm_thread(void* ptr_info);

		// @函数名: 工作线程
		static Q_THREAD_T work_thread(void* ptr_info);

		// @函数名: 识别线程
		static Q_THREAD_T proc_thread(void* ptr_info);

		// @函数名: 发送线程
		static Q_THREAD_T send_thread(void* ptr_info);

		// @函数名: XML解析函数
		int32_t parseTasksFromXML(const std::string& xml, std::vector<taskInfo>& tasks);

		// @函数名: 链接获取函数
		int32_t parseAndGetLinks(const std::string& xml, std::string& strTask);

		// @函数名: 数据存储函数
		int32_t write_data_file(const char* ptr_file, FILE*& fp_w, QMutexLock& file_mutex, const char* ptr_buf, int32_t buf_len);

		// @函数名: 数据备份函数
		int32_t backup_file(const char* ptr_file, const char* ptr_path, char* ptr_buf, int32_t buf_size);

		// @函数名: 释放线程信息
		void free_thread_info();

		// @函数名: 获取线程运行状态
		static int32_t get_thread_state(void* ptr_info);

	protected:
		/* general */
		uint32_t        pid_;
		char*           pidfile_;
		char*           configfile_;
		time_t          unixtime_;
		uint32_t        hz_;
		uint32_t        arch_bits_;
		bool            start_flag_;
		bool            exit_flag_;
		/* configuration */
		char*           server_name_;
		uint16_t        server_port_;
		int32_t         server_timeout_;
		/* threads */
		threadInfo*     thread_info_;
		int32_t         thread_max_;
		int32_t	        comm_thread_max_;
		int32_t         comm_buffer_size_;
		int32_t         comm_thread_timeout_;
		int32_t         work_thread_max_;
		int32_t         work_buffer_size_;
		int32_t         work_thread_timeout_;
		int32_t	        proc_thread_max_;
		int32_t         proc_buffer_size_;
		int32_t         proc_thread_timeout_;
		int32_t	        send_thread_max_;
		int32_t         send_buffer_size_;
		int32_t         send_thread_timeout_;
		/* memory */
		QPoolAllocator* pool_allocator_;
		int32_t         chunk_size_;
		/* networking */
		Q_SOCKET_T      listen_sock_;
		QQueue<std::string>* task_queue_;
		QTrigger*       task_trigger_;
		int32_t         queue_size_;
		/* url processor */
		URLProcessor    processor_;
		QDiskCache<uint64_t> uniq_cache_;
		/* storage */
		char*           task_path_;
		char*           task_read_path_;
		char*           task_write_path_;
		FILE*           task_write_fp_;
		QMutexLock      task_mutex_;
		char*           data_path_;
		char*           data_read_path_;
		char*           data_write_path_;
		FILE*           data_write_fp_;
		QMutexLock      data_mutex_;
		/* push server */
		char*           send_ip_;
		int32_t         send_port_;
		/* sentinel */
		QRemoteMonitor* monitor_;
		uint16_t        monitor_port_;
		/* log */
		QLogger*        logger_;
		char*           log_path_;
		char*           log_prefix_;
		int32_t         log_size_;
		int32_t         log_screen_;
		/* fields used only for stats */
		time_t          stat_starttime_;
		time_t          stat_lastinteraction_;
		uint32_t        stat_numconnections_;
		uint32_t        stat_succconnections_;
		uint32_t        stat_failedconnections_;
};

Q_END_NAMESPACE

#endif // __SPIDERSERVER_H_
