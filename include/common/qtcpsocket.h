/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qtcpsocket.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2012/11/03
**
*********************************************************************************************/

#ifndef __QTCPSOCKET_H_
#define __QTCPSOCKET_H_

#include "qglobal.h"
#include "qalgorithm.h"
#include "qconfigreader.h"
#include "qdatetime.h"
#include "qdir.h"
#include "qfile.h"
#include "qfunc.h"
#include "qlogger.h"
#include "qmd5.h"
#include "qqueue.h"
#include "qremotemonitor.h"
#include "qservice.h"
#include "qvector.h"
#include "cJSON.h"

Q_BEGIN_NAMESPACE

#define TCP_OK                    (0)
#define TCP_ERR                   (-1)

#define TCP_ERR_HEAP_ALLOC        (-2)

#define TCP_ERR_SOCKET_INIT	  (-11)
#define TCP_ERR_SOCKET_CONNECTION (-12)

#define TCP_ERR_SOCKET_ACCEPT     (-13)
#define TCP_ERR_SOCKET_TIMEOUT    (-14)
#define TCP_ERR_SOCKET_RECV       (-15)
#define TCP_ERR_PACKET_HEADER     (-16)
#define TCP_ERR_PACKET_LENGTH     (-17)
#define TCP_ERR_SOCKET_SEND       (-18)

#define TCP_ERR_SOCKET_VERSION    (-19)
#define TCP_ERR_PROTOCOL_TYPE     (-20)
#define TCP_ERR_SOURCE_TYPE       (-21)
#define TCP_ERR_COMMAND_TYPE      (-22)
#define TCP_ERR_BUFFER_SIZE       (-23)
#define TCP_ERR_OPERATE_TYPE      (-24)
#define TCP_ERR_DATA_LENGTH       (-25)

#define TCP_DEFAULT_HZ            (10)
#define TCP_DEFAULT_MIN_HZ        (1)
#define TCP_DEFAULT_MAX_HZ        (500)

#define TCP_DEFAULT_PIDFILE       ("/var/run/niu.pid")
#define TCP_DEFAULT_CONFIG_FILE   ("../conf/init.conf")

#define TCP_DEFAULT_SERVER_IP     ("127.0.0.1")
#define TCP_DEFAULT_SERVER_PORT	  (8088)
#define TCP_DEFAULT_MONITOR_PORT  (8078)
#define TCP_DEFAULT_SERVER_TIMEOUT (10000)

#define TCP_DEFAULT_PROTOCOL_TYPE (1)
#define TCP_DEFAULT_SOURCE_TYPE   (1)
#define TCP_DEFAULT_COMMAND_TYPE  (1)
#define TCP_DEFAULT_OPERATE_TYPE  (1)

#define TCP_DEFAULT_INVALID_SOCKET (-1)
#define TCP_DEFAULT_THREAD_NUM    (1)
#define TCP_DEFAULT_BUFFER_SIZE   (1<<20)
#define TCP_DEFAULT_THREAD_TIMEOUT (12000)

#define TCP_DEFAULT_QUEUE_SIZE    (200)
#define TCP_DEFAULT_REQUEST_SIZE  (1<<20)
#define TCP_DEFAULT_REPLY_SIZE    (1<<20)
#define TCP_DEFAULT_HEADER_SIZE   (12)

#define TCP_DEFAULT_IP_SIZE       (16)
#define TCP_DEFAULT_NAME_SIZE     (1<<8)
#define TCP_DEFAULT_PATH_SIZE     (1<<8)

#define TCP_DEFAULT_LOG_PATH      ("../log/")
#define TCP_DEFAULT_LOG_PREFIX	  (NULL)
#define TCP_DEFAULT_LOG_SIZE      (1<<10)
#define TCP_DEFAULT_LOG_SCREEN    (1)

#define TCP_HEADER_VERSION	  (*(uint64_t*)"YST1.0.0")
#define TCP_TAILER_FILE_MARK	  (*(uint64_t*)"@#@#@#@#")

#pragma pack(1)

/* thread info */
struct threadInfo {
	void		*pthis;
	uint32_t	id;
	int8_t		status;
	int8_t		flag;
	int32_t		buf_size;
	char*		ptr_buf;
	int32_t		timeout;
	QStopwatch	sw;

	void*		for_worker;

	threadInfo() :
		pthis(NULL),
		id(0),
		status(0),
		flag(0),
		buf_size(0),
		ptr_buf(NULL),
		timeout(TCP_DEFAULT_THREAD_TIMEOUT),
		for_worker(NULL)
	{}
};

/* protocol */
struct baseHeader {
	uint64_t	version;
	int32_t		length;
};

/* request param */
struct requestParam {
	uint16_t	protocol_type;
	uint16_t	source_type;
	char		reserved[14];
	uint16_t	command_type;
};

/* reply param */
struct replyParam {
	char		reserved[14];
	uint16_t	command_type;
	int32_t		status;
};

/* request header */
struct requestHeader {
	uint64_t	version;
	int32_t		length;
	uint16_t	protocol_type;
	uint16_t	source_type;
	char		reserved[14];
	uint16_t	command_type;
};

/* reply header */
struct replyHeader {
	uint64_t	version;
	int32_t		length;
	char		reserved[14];
	uint16_t	command_type;
	int32_t		status;
};

/* network reply */
struct networkReply {
	int32_t         status;
	char*           data;
	int32_t         length;

	networkReply() :
		status(0),
		data(NULL),
		length(0)
	{}
};

/* client info */
struct clientInfo {
	Q_SOCKET_T      client_sock;
	char            client_ip[TCP_DEFAULT_IP_SIZE];
	int32_t         client_port;

	char*           request_buffer;
	int32_t         request_buffer_size;
	char*           reply_buffer;
	int32_t         reply_buffer_size;

	clientInfo() :
		client_sock(TCP_DEFAULT_INVALID_SOCKET),
		request_buffer(NULL),
		request_buffer_size(0),
		reply_buffer(NULL),
		reply_buffer_size(0)
	{}
};

#pragma pack()

// TCP通讯客户端
class QTcpClient : public noncopyable {
	public:
		// @函数名: 构造函数
		QTcpClient();

		// @函数名: 析构函数
		virtual ~QTcpClient();

		// @函数名: 设置主机名
		void setHost(const char* server_ip, uint16_t server_port);

		// @函数名: 设置超时时间
		void setTimeout(int32_t timeout=TCP_DEFAULT_SERVER_TIMEOUT);

		// @函数名: 设置协议版本信息
		void setVersion(uint64_t version=TCP_HEADER_VERSION);

		// @函数名: 设置协议类型
		void setProtocolType(uint16_t protocol_type);

		// @函数名: 设置来源类型
		void setSourceType(uint16_t source_type);

		// @函数名: 设置命令类型
		void setCommandType(uint16_t command_type);

		// @函数名: 设置操作类型
		void setOperateType(uint16_t operate_type);

		// @函数名: 发送请求信息
		int32_t sendRequest(const char* ptr_data, int32_t data_len);

		// @函数名: 获取响应信息
		int32_t getReply(networkReply* reply);

	protected:
		/* General */
		char            server_ip_[16];
		uint16_t        server_port_;
		int32_t         server_timeout_;
		/* networking */
		Q_SOCKET_T      sock_;
		uint64_t	version_;
		uint16_t	protocol_type_;
		uint16_t	source_type_;
		uint16_t	command_type_;
		uint16_t	operate_type_;
		char*           request_buffer_;
		int32_t         request_buffer_size_;
		char*           reply_buffer_;
		int32_t         reply_buffer_size_;
};

// TCP通讯服务端
class QTcpServer : public noncopyable {
	public:
		// @函数名: 构造函数
		explicit QTcpServer();

		// @函数名: 析构函数
		virtual ~QTcpServer();

		// @函数名: 初始化函数
		virtual int32_t init(const char* cfg_file);

		// @函数名: 主线程启动函数
		virtual int32_t start();

		// @函数名: 继承类必须实现的业务逻辑初始化函数
		virtual int32_t server_fun_init(void*& handle)=0;

		// @函数名: 继承类必须实现的包头解析函数
		virtual int32_t server_fun_header(const char* header_buffer, int32_t header_len, const void* handle=NULL)=0;

		// @函数名: 继承类必须实现的协议处理函数
		virtual int32_t server_fun_process(const char* request_buffer, int32_t request_len, char* reply_buffer, int32_t reply_size, \
				const void* handle=NULL)=0;

		// @函数名: 继承类必须实现的业务逻辑类释放函数
		virtual int32_t server_fun_free(const void* handle=NULL)=0;

	private:
		// @函数名: 配置读取函数
		int32_t load_server_config(const char* cfg_file);

		// @函数名: 通信线程
		static Q_THREAD_T comm_thread(void* ptr_info);

		// @函数名: 工作线程
		static Q_THREAD_T work_thread(void* ptr_info);

		// @函数名: 发送线程
		static Q_THREAD_T send_thread(void* ptr_info);

		// @函数名: 数据存储函数
		int32_t write_data_file(const char* ptr_file, FILE*& fp_w, const char* ptr_buf, int32_t buf_len);

		// @函数名: 数据备份函数
		int32_t backup_file(const char* ptr_file, char* ptr_buf, int32_t buf_size);

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
		int32_t	        send_thread_max_;
		int32_t         send_buffer_size_;
		int32_t         send_thread_timeout_;
		/* networking */
		Q_SOCKET_T      listen_sock_;
		int32_t         queue_size_;
		int32_t         client_request_size_;
		int32_t         client_reply_size_;
		int32_t         header_size_;
		QQueue<clientInfo*>* chunk_queue_;
		QQueue<clientInfo*>* client_queue_;
		QTrigger*       client_trigger_;
		/* storage */
		char*           send_ip_;
		int32_t         send_port_;
		char*           data_path_;
		char*           img_path_;
		char*           read_path_;
		char*           write_path_;
		FILE*           write_fp_;
		QMutexLock      file_mutex_;
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
		uint32_t        stat_rejectedconnections_;
};

Q_END_NAMESPACE

#endif // __QTCPSOCKET_H_
