/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qspiderserver.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/12/29
**
*********************************************************************************************/

#ifndef __QSPIDERSERVER_H_
#define __QSPIDERSERVER_H_

#include <qglobal.h>
#include <qalgorithm.h>
#include <qallocator.h>
#include <qbitmap.h>
#include <qbuffer.h>
#include <qbytearray.h>
#include <qconfigreader.h>
#include <qdatastream.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qdiskcache.h>
#include <qfile.h>
#include <qfunc.h>
#include <qhashmap.h>
#include <qhashsearch.h>
#include <qheap.h>
#include <qhttpclient.h>
#include <qhttpserver.h>
#include <qlist.h>
#include <qlogger.h>
#include <qmarkovmodel.h>
#include <qmd5.h>
#include <qmd5file.h>
#include <qnetworkaccessmanager.h>
#include <qqueue.h>
#include <qregexp.h>
#include <qremotemonitor.h>
#include <qserialization.h>
#include <qservice.h>
#include <qstack.h>
#include <qstring.h>
#include <qtextcodec.h>
#include <qthread.h>
#include <qvector.h>
#include <qzlibmanager.h>
#include <downloader.h>
#include <md5compress.h>
#include <parser.h>

#include <ServerPicker.h>

#define SPIDER_OK              (0)
#define SPIDER_ERR             (-1)

#define SPIDER_DEFAULT_HZ      (10)
#define SPIDER_MIN_HZ          (1)
#define SPIDER_MAX_HZ          (500)

#define SPIDER_DEFAULT_PIDFILE ("/var/run/spider.pid")
#define SPIDER_DEFAULT_CONFIG_FILE ("../conf/init.conf")

#define SPIDER_DEFAULT_BACKLOG (511)

#define SPIDER_INVALID_SOCKET  (-1)
#define SPIDER_DEFAULT_THREAD_TIMEOUT (10000)

#define SPIDER_CHUNK_SIZE      (1<<20)

#define SPIDER_IP_SIZE         (16)
#define SPIDER_NAME_SIZE       (1<<8)
#define SPIDER_PATH_SIZE       (1<<8)
#define SPIDER_URL_SIZE        (1<<8)
#define SPIDER_SITE_SIZE       (1<<7)
#define SPIDER_ENAME_SIZE      (1<<7)
#define SPIDER_PARAM_SIZE      (30)

#define SPIDER_DEFAULT_START   (0)
#define SPIDER_DEFAULT_FINISH  (-1)
#define SPIDER_DEFAULT_STEP    (-1)

#define SPIDER_DEFAULT_TID     (-1)
#define SPIDER_DEFAULT_PRIORITY (0)

#define SPIDER_METHOD_UNKNOWN  (0x00)
#define SPIDER_METHOD_GET      (0x01)
#define SPIDER_METHOD_POST     (0x02)
#define SPIDER_METHOD_HEAD     (0x03)
#define SPIDER_METHOD_PUT      (0x04)
#define SPIDER_METHOD_DELETE   (0x05)

#define SPIDER_LOG_PATH        ("../log")
#define SPIDER_LOG_PREFIX      (NULL)
#define SPIDER_LOG_SIZE        (1<<10)

#define SPIDER_CACHE_BUCKET_SIZE (1<<20)
#define SPIDER_CACHE_INTERVAL  (60)

#define PROTOCOL_HEAD_VERSION  (*(uint64_t*)"YST1.0.0")
#define PROTOCOL_FILE_END_MARK (*(uint64_t*)"@#@#@#@#")

Q_USING_NAMESPACE

using std::vector;
using std::map;

#pragma pack(1)

struct threadInfo {
	void		*pthis;
	uint32_t	id;
	int8_t 		status;
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
		timeout(SPIDER_DEFAULT_THREAD_TIMEOUT),
		for_worker(NULL)
	{}
};

struct configInfo {
	/* general */
	char		server_name[SPIDER_NAME_SIZE];
	char		server_ip[SPIDER_IP_SIZE];
	int32_t		server_port;
	int32_t		monitor_port;
	int32_t		sock_timeout;
	/* threads */
	int32_t		comm_thread_max;
	int32_t		comm_buffer_size;
	int32_t		comm_thread_timeout;
	int32_t		work_thread_max;
	int32_t		work_buffer_size;
	int32_t		work_thread_timeout;
	int32_t		scan_thread_max;
	int32_t		scan_buffer_size;
	int32_t		scan_thread_timeout;
	int32_t		extract_thread_max;
	int32_t		extract_buffer_size;
	int32_t		extract_thread_timeout;
	int32_t		send_thread_max;
	int32_t		send_buffer_size;
	int32_t		send_thread_timeout;
	/* download */
	int32_t         auto_update;
	int32_t		only_update;
	int32_t         update_interval;
	int32_t		download_interval;
	int32_t		download_timeout;
	int32_t		retry_num_max;
	int32_t		retry_interval;
	/* io */
	int32_t		chunk_size;
	/* queue */
	int32_t		client_queue_size;
	int32_t		task_queue_size;
	/* templates */
	char		templ_list_path[SPIDER_PATH_SIZE];
	char		templ_detail_path[SPIDER_PATH_SIZE];
	/* image */
	char		img_server[SPIDER_IP_SIZE];
	int32_t		img_port;
	/* storage */
	char		send_data_path[SPIDER_PATH_SIZE];
	char		send_bak_path[SPIDER_PATH_SIZE];
	char		send_read_path[SPIDER_PATH_SIZE];
	char		send_write_path[SPIDER_PATH_SIZE];
	/* destination */
	char		send_ip[SPIDER_IP_SIZE];
	int32_t		send_port;
};

struct requestHeader {
	uint64_t	version;
	int32_t		length;
	int16_t		protocol_type;
	int16_t		source_type;
	char		save[14];
	int16_t		cmd_type;
};

struct replyHeader {
	uint64_t	version;
	int32_t		length;
	char		save[14];
	int16_t		cmd_type;
	int32_t		status_code;
};

struct landHeader {
	uint64_t	version;
	int32_t		length;
};

struct clientInfo {
	Q_SOCKET_T	sock_client;
	char		client_ip[SPIDER_IP_SIZE];
	int32_t		client_port;

	char*		request_buffer;	
	int32_t		request_buffer_size;
	char*		reply_buffer;
	int32_t		reply_buffer_size;

	clientInfo() :
		sock_client(SPIDER_INVALID_SOCKET),
		request_buffer(NULL),
		request_buffer_size(0),
		reply_buffer(NULL),
		reply_buffer_size(0)
	{}
};

struct entryInfo {
	/* entry url */
	int32_t		entry_id;
	char		url[SPIDER_URL_SIZE];
	int32_t		url_len;
	char		param[SPIDER_PARAM_SIZE];
	int32_t		param_len;
	char		website[SPIDER_SITE_SIZE];
	int32_t		website_len;
	char		ename[SPIDER_ENAME_SIZE];
	int32_t		ename_len;
	int32_t		method;		/* request type: (1) GET (2) POST */
	bool		is_cookie;	/* wether or not to use cookie */
	bool		is_proxy;	/* wether or not to use proxy */
	/* templates */
	int32_t		tid;
	int32_t		ntid;
	/* type */
	int32_t		type;
	int32_t		extratype;
	/* other params */
	bool		status;
	bool		period;
	uint32_t	priority;

	entryInfo() :
		entry_id(0),
		url_len(0),
		param_len(0),
		method(SPIDER_METHOD_GET),
		is_cookie(false),
		is_proxy(false),
		tid(SPIDER_DEFAULT_TID),
		ntid(SPIDER_DEFAULT_TID),
		status(true),
		period(false),
		priority(SPIDER_DEFAULT_PRIORITY)
	{
		memset(url, '\0', sizeof(url));
		memset(param, '\0', sizeof(param));
		memset(website, '\0', sizeof(website));
		memset(ename, '\0', sizeof(ename));
	}
};

struct taskInfo {
	int32_t		task_id;
	int32_t		tid;
	int32_t		ntid;
	char		url[SPIDER_URL_SIZE];
	int32_t		url_len;
	int32_t		type;
	int32_t		extratype;
	int32_t		method;		/* method: (1) GET (2) POST */
	bool		is_cookie;	/* wether or not to use cookie */
	bool		is_proxy;	/* wether or not to use proxy */

	taskInfo():
		task_id(0),
		tid(SPIDER_DEFAULT_TID),
		ntid(SPIDER_DEFAULT_TID),
		url_len(0),
		method(SPIDER_METHOD_GET),
		is_cookie(false),
		is_proxy(false)
	{
		memset(url, '\0', sizeof(url));
	}
};

struct interfaceInfo {
	Downloader*     ptr_downloader;
	Parser*         ptr_parser;

	interfaceInfo() :
		ptr_downloader(NULL),
		ptr_parser(NULL)
	{}
};

#pragma pack()

class QSpiderServer : public noncopyable {
	public:
		// @函数名: 构造函数
		QSpiderServer();

		// @函数名: 析构函数
		virtual ~QSpiderServer();

		// @函数名: 初始化函数
		int32_t init(const char* cfg_file);

		// @函数名: 主线程执行函数
		int32_t run();

	private:
		// @函数名: 通信线程
		static Q_THREAD_T comm_thread(void* ptr_info);

		// @函数名: 工作线程
		static Q_THREAD_T work_thread(void* ptr_info);

		// @函数名: 入口扫描线程
		static Q_THREAD_T scan_thread(void* ptr_info);

		// @函数名: 抽取线程
		static Q_THREAD_T extract_thread(void* ptr_info);

		// @函数名: 发送线程
		static Q_THREAD_T send_thread(void* ptr_info);

		// @函数名: 模板初始化函数
		// @参数01: 模板tid
		// @参数02: 模板配置文件路径
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t load_templates(int32_t tid, const char* tplfile);

		// @函数名: 任务处理函数
		// @参数01: 通信接收的请求数据
		// @参数02: 请求数据的长度
		// @参数03: 通信回送数据的内存
		// @参数04: 通信回送数据的最大内存空间
		// @参数05: 辅助内存区
		// @参数06: 辅助内存区的最大空间
		// @参数07: 网络下载类句柄
		// @参数08: 抽取类句柄
		// @返回值: 成功返回待回送数据的实际长度，失败返回<0的错误码
		int32_t fun_process(char* request_buffer, int32_t request_len, char* reply_buf, int32_t reply_size, \
				char* ptr_buf, \
				int32_t buf_size, \
				Downloader* downloader, \
				Parser* parser);
		
		// @函数名: 入口页处理函数
		// @参数01: XML数据信息
		// @参数02: XML数据信息的长度
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t process_entry(const char* ptr_xml, int32_t xml_len);

		// @函数名: 抽取测试函数
		// @参数01: XML数据信息
		// @参数02: XML数据信息的长度
		// @参数03: 网络下载类句柄
		// @参数04: 抽取类句柄
		// @参数05: 模板ID
		// @参数06: 辅助内存区
		// @参数07: 辅助内存区的最大空间
		// @参数08: 通信回送数据的内存
		// @参数09: 通信回送数据的最大内存空间
		// @返回值: 成功返回待回送数据的实际长度，失败返回<0的错误码
		int32_t process_url(const char* ptr_xml, int32_t xml_len, Downloader* downloader, Parser* parser, \
				int32_t tid, \
				char* ptr_buf, \
				int32_t buf_size, \
				char* ptr_reply, \
				int32_t reply_size);

		// @函数名: 模板处理函数
		// @参数01: 模板操作类型
		// @参数02: 模板ID
		// @参数03: XML数据信息
		// @参数04: XML数据信息的长度
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t process_template(int32_t type, int32_t tid, const char* ptr_xml, int32_t xml_len);

		// @函数名: 任务拼合函数
		// @参数01: 任务信息结构体
		// @参数02: 任务输出内存区
		// @参数03: 任务输出内存的最大空间
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t combine_task(struct taskInfo* task_info, char* ptr_buf, int32_t buf_size);

		// @函数名: 详情页解析函数
		// @参数01: 详情页聚合XML数据
		// @参数02: XML数据的实际长度
		// @参数03: 列表页任务信息
		// @参数04: 列表页包含的详情页数
		// @返回值: 成功返回0, 重复详情页返回1, 失败返回<0的错误码
		int32_t parse_detail_url(const char* ptr_xml, int32_t xml_len, struct taskInfo* list_task_info, int32_t& url_num);

		// @函数名: 数据打包函数
		// @参数01: XML数据内容
		// @参数02: XML数据内容的实际长度
		// @参数03: 落地数据内容
		// @参数04: 落地数据内容的实际长度
		// @返回值: 成功返回数据包的实际长度，失败返回<0的错误码
		int32_t pack_data(const char* ptr_xml, int32_t xml_len, char* ptr_buf, int32_t buf_size);

		// @函数名: 数据落地函数
		// @参数01: 落地文件名
		// @参数02: 落地文件句柄
		// @参数03: 落地数据内容
		// @参数04: 落地数据内容的实际长度
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t write_data_file(const char* ptr_file, FILE*& fp_w, const char* ptr_buf, int32_t buf_len);

		// @函数名: 数据备份函数
		// @参数01: 落地文件名
		// @参数02: 辅助内存区
		// @参数03: 辅助内存区的最大空间
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t backup_file(const char* ptr_file, char* ptr_buf, int32_t buf_size);

		// @函数名: 获取各线程运行状态
		// @参数01: 通常为this指针
		// @返回值: 返回超时的线程数, 0表示没有超时的线程
		static int32_t get_thread_state(void* ptr_info);

	protected:
		/* General */
		uint32_t             m_pid;
		char*                m_pidfile;
		time_t               m_unixtime;
		uint32_t             m_hz;
		uint32_t             m_arch_bits;
		bool                 m_start_flag;
		bool                 m_exit_flag;
		/* configuration */
		configInfo*          m_ptr_cfg_info;
		/* threads */
		threadInfo*          m_ptr_trd_info;	
		int32_t	             m_thread_max;
		/* memory */
		QPoolAllocator*      m_ptr_pool_allocator;
		/* networking */
		Q_SOCKET_T           m_sock_svr;
		QQueue<clientInfo>*  m_ptr_client_queue;
		QTrigger*            m_ptr_client_trigger;
		/* entries */
		vector<entryInfo>    m_entry_array;
		QTrigger*            m_ptr_entry_trigger;
		/* detail page */
		QQueue<taskInfo>*    m_ptr_dp_queue;
		QTrigger*            m_ptr_dp_trigger;
		/* extractor */
		ServerPicker*        m_ptr_picker;
		/* template */
		TemplateManager*     m_ptr_tpl_manager;
		/* persistence */
		uint64_t             I_SEND_FILE_MARK;
		FILE*                m_send_write_fp;
		QMutexLock           m_send_file_mutex;
		QDiskCache<uint64_t>* m_ptr_uniq_cache;
		int32_t              m_cache_bucket_size;
		int32_t	             m_cache_interval;
		/* log */
		QLogger*             m_ptr_logger;
		char*                m_ptr_log_path;
		char*                m_ptr_log_prefix;
		int32_t              m_log_size;
		/* fields used only for stats */
		time_t               m_stat_starttime;
		time_t               m_stat_lastinteraction;
		uint32_t             m_stat_numconnections;
		uint32_t             m_stat_succ_conn;
		uint32_t             m_stat_failed_conn;
		uint32_t             m_stat_rejected_conn;
		/* sentinel */
		QRemoteMonitor*      m_ptr_remote_monitor;
};

#endif // __QSPIDERSERVER_H_
