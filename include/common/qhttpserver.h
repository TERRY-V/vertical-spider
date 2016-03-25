/********************************************************************************************
**
** Copyright (C) 2010-2015 Terry Niu (Beijing, China)
** Filename:	qhttpserver.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2015/03/02
**
*********************************************************************************************/

#ifndef __QHTTPSERVER_H_
#define __QHTTPSERVER_H_

#include "qglobal.h"
#include "qqueue.h"
#include "qlogger.h"
#include "qconfigreader.h"

Q_BEGIN_NAMESPACE

// HTTP协议相关状态码
#define CONTINUE	(100)				// 客户端应当继续发送请求
#define SWITCH_PROTOCOL (101)				// 切换协议
#define OK_REQUEST	(200)				// 请求已成功
#define CREATED_REQUEST	(201)				// 请求已实现, 已创建新资源
#define ACCEPT_REQUEST	(202)				// 请求已接收, 但尚未处理
#define NO_CONTENT	(204)				// 服务器请求处理成功, 不需要返回任何实体内容
#define RESET_CONTENT	(205)				// 服务器请求处理成功, 没返回任何内容
#define PARTIAL_CONTENT	(206)				// 服务器已成功处理部分GET请求(断点续传)
#define MOVED_REQUEST	(301)				// 资源请求重定向
#define FOUND_REQUEST	(302)				// 客户端资源在其它地方找到
#define SEE_OTHER	(303)				// 查看其他位置
#define NOT_MODIFIED	(304)				// 申请资源没有更新
#define USE_PROXY	(305)				// 需要使用代理
#define SWITCH_PROXY	(306)				// 新版规范, 该状态码已停用
#define TEMP_REDIRECT	(307)				// 临时重定向
#define BAD_REQUEST	(400)				// 请求错误
#define UNAUTH_REQUEST	(401)				// 请求需要认证信息
#define FORBID_REQUEST	(403)				// 禁止访问
#define NO_RESOURCE	(404)				// 资源不存在
#define METHOD_ERROR	(405)				// 请求方法不支持
#define NOT_ACCEPTABLE	(406)				// 不可接受
#define PROXY_ERROR	(407)				// 客户端代理服务器认证错误
#define TIMEOUT_ERROR	(408)				// 请求超时
#define CONFLICT_ERROR	(409)				// 冲突
#define GONE_ERROR	(410)				// 已删除
#define LENGTH_ERROR	(411)				// 需要有效长度
#define PRECOND_ERROR	(412)				// 先决条件失败
#define ENTITY_ERROR	(413)				// 请求实体过大
#define URI_ERROR	(414)				// 请求URI过长
#define MEDIATYPE_ERROR	(415)				// 类型不支持
#define RANGE_ERROR	(416)				// 请求范围错误
#define EXPECT_ERROR	(417)				// 未满足期望值
#define INTERNAL_ERROR	(500)				// 服务器内部错误
#define NOT_IMPLEMENTED	(501)				// 尚未实施
#define BAD_GATEWAY	(502)				// 网关错误
#define SERVICE_ERROR	(503)				// 服务不可用
#define GATEWAY_TIMEOUT	(504)				// 网关超时
#define VERSION_ERROR	(505)				// 版本不支持

#define INVALID_SOCKET	(-1)				// 默认套接字
#define IPADDR_MAX_LEN	(30)				// IP字符串最大长度
#define STATUS_LINE_LEN	(30)				// 响应状态行最大长度
#define EPOLL_EVENT_MAX	(10000)				// EPOLL最大支持事件数

#define HTTP_WEB_SERVER	((char*)"QHS/1.0")		// WEB SERVER名称

// HTTP协议请求方法
enum method_t {
	UNKOWN		= 0x00,				// 位置请求
	GET		= 0x01,				// GET请求
	POST		= 0x02,				// POST请求
	HEAD		= 0x03,				// HEAD请求
	PUT		= 0x04,				// 上传某个资源
	DELETE		= 0x05,				// 删除某个资源
	TRACE		= 0x06,				// 要求服务器返回原始HTTP请求内容
	OPTIONS		= 0x07,				// 查看服务器对URL支持的请求方法
	CONNECT		= 0x08,				// 用于代理服务器, 请求连接转化为一个安全隧道
	PATCH		= 0x09				// 对某个资源做部分修改
};

// HTTP协议版本信息
enum version_t {
	HTTP_1_0	= 0x00,				// HTTP协议1.0
	HTTP_1_1	= 0x01				// HTTP协议1.1
};

// HTTP协议连接信息
enum connection_t {
	HTTP_CLOSE	= 0x00,				// HTTP协议短连接
	HTTP_KEEP_ALIVE	= 0x01				// HTTP协议连接复用
};

// HTTP协议内容类型
enum content_type_t {
	UNKNOWN		= 0x00,				// 位置类型
	TEXT_HTML	= 0x01,				// text/html
	TEXT_XML	= 0x02,				// text/xml
	TEXT_CSS	= 0x03,				// text/css
	TEXT_JAVASCRIPT	= 0x04,				// text/javascript
	TEXT_PLAIN	= 0x05,				// text/plain
	IMAGE_JPEG	= 0x06,				// image/jpeg
	IMAGE_PNG	= 0x07,				// image/png
	IMAGE_GIF	= 0x08				// image/gif
};

// 字符串切片信息
struct slice_t {
	char*		ptr;				// 字符串指针
	int32_t		length;				// 字符串的实际长度

	slice_t() :
		ptr(0),
		length(0)
	{
	}
};

// IO数据信息结构体
struct io_data_t {
	int32_t		sock_fd;			// SOCKET请求套接字描述符
	struct sockaddr_in sock_addr;			// SOCKET请求地址信息
	socklen_t	sock_len;			// SOCKET请求地址长度

	enum method_t	method;				// HTTP协议请求方法
	enum version_t 	version;			// HTTP协议版本信息
	enum connection_t keep_alive;			// HTTP协议是否保持连接
	struct slice_t	url;				// HTTP协议请求URL信息
	struct slice_t	host;				// HTTP协议服务器信息
	struct slice_t	accept;				// HTTP客户端接收数据类型
	struct slice_t	referer;			// HTTP客户端来路页面
	struct slice_t	user_agent;			// HTTP客户端用户代理
	struct slice_t	accept_encoding;		// HTTP客户端接收编码类型
	struct slice_t	accept_language;		// HTTP客户端接收语言类型

	char* 		request_header;			// HTTP协议请求头信息
	int32_t		request_header_size;		// HTTP协议请求头信息的最大长度
	int32_t 	request_header_len;		// HTTP协议请求头信息的实际长度
	char* 		request_content;		// HTTP协议请求正文信息
	int32_t		request_content_size;		// HTTP协议请求正文信息的最大长度
	int32_t 	request_content_len;		// HTTP协议请求正文信息的实际长度

	uint16_t	status;				// HTTP协议响应状态码
	char		status_line[STATUS_LINE_LEN];	// HTTP协议响应状态行
	char*		server;				// HTTP协议响应服务器
	uint32_t	content_length;			// HTTP协议响应内容长度
	enum content_type_t content_type;		// HTTP协议响应内容类型

	char*		response_header;		// HTTP协议响应头信息
	int32_t		response_header_size;		// HTTP协议响应头信息的最大长度
	int32_t		response_header_len;		// HTTP协议响应头信息的实际长度
	char* 		response_content;		// HTTP协议响应正文信息
	int32_t		response_content_size;		// HTTP协议响应正文信息的最大长度
	int32_t 	response_content_len;		// HTTP协议响应正文信息的实际长度

	io_data_t() :
		sock_fd(INVALID_SOCKET),
		method(GET),
		version(HTTP_1_1),
		keep_alive(HTTP_CLOSE),
		request_header(0),
		request_header_size(0),
		request_header_len(0),
		request_content(0),
		request_content_size(0),
		request_content_len(0),
		status(BAD_REQUEST),
		server(HTTP_WEB_SERVER),
		content_length(0),
		content_type(TEXT_HTML),
		response_header(0),
		response_header_size(0),
		response_header_len(0),
		response_content(0),
		response_content_size(0),
		response_content_len(0)
	{
		memset(&sock_addr, 0, sizeof(struct sockaddr_in));
		memset(&sock_len, 0, sizeof(socklen_t));
		memset(&url, 0, sizeof(struct slice_t));
		memset(&host, 0, sizeof(struct slice_t));
		memset(&accept, 0, sizeof(struct slice_t));
		memset(&referer, 0, sizeof(struct slice_t));
		memset(&user_agent, 0, sizeof(struct slice_t));
		memset(&accept_encoding, 0, sizeof(struct slice_t));
		memset(&accept_language, 0, sizeof(struct slice_t));
	}

	void reset()
	{
		sock_fd=INVALID_SOCKET;

		request_header_len=0;
		request_content_len=0;
		method=GET;
		version=HTTP_1_1;
		keep_alive=HTTP_CLOSE;

		response_header_len=0;
		response_content_len=0;
		status=BAD_REQUEST;
		content_length=0;
		content_type=TEXT_HTML;

		memset(&sock_addr, 0, sizeof(struct sockaddr_in));
		memset(&sock_len, 0, sizeof(socklen_t));
		memset(&url, 0, sizeof(struct slice_t));
		memset(&host, 0, sizeof(struct slice_t));
		memset(&accept, 0, sizeof(struct slice_t));
		memset(&referer, 0, sizeof(struct slice_t));
		memset(&user_agent, 0, sizeof(struct slice_t));
		memset(&accept_encoding, 0, sizeof(struct slice_t));
		memset(&accept_language, 0, sizeof(struct slice_t));
	}
};

// 线程信息结构体
struct thread_info_t {
	int8_t		id;				// 线程序号id
	int8_t 		status;				// 线程工作状态: 0不工作 1工作
	int8_t		run_flag;			// 线程启动成功标识: 0未启动 1启动成功 -1启动失败
	int32_t		buf_size;			// 线程中使用内存大小
	char*		ptr_buf;			// 线程中使用内存
	void*		ptr_this;			// 线程回调指针
	int32_t		timeout;			// 线程超时时间
	QStopwatch	sw;				// 线程运行时间

	void*		for_worker;			// 工作线程专用

	thread_info_t() :
		id(0),
		status(0),
		run_flag(0),
		buf_size(0),
		ptr_buf(0),
		ptr_this(NULL),
		timeout(3000),				// 默认线程超时时间10秒
		for_worker(NULL)
	{
	}
};

// 外部配置文件信息
struct config_info_t {
	char		server_name[256];		// 服务名称

	char		server_ip[16];			// 通信服务本机IP地址
	int32_t		server_port;			// 通信服务端口号
	int32_t		monitor_port;			// 服务监控端口号

	int32_t		sock_timeout;			// socket超时时间

	int32_t		comm_thread_max;		// 通信线程数
	int32_t		comm_buffer_size;		// 通信线程所使用内存大小
	int32_t		comm_thread_timeout;		// 通信线程监控超时时间

	int32_t		work_thread_max;		// 工作线程数
	int32_t		work_buffer_size;		// 工作线程所使用内存大小
	int32_t		work_thread_timeout;		// 工作线程监控超时时间

	int32_t		task_queue_size;		// 任务队列最大空间

	int32_t		request_header_size;		// IO请求头信息的内存大小
	int32_t		request_content_size;		// IO请求正文信息的内存大小
	int32_t		response_header_size;		// IO响应头信息的内存大小
	int32_t		response_content_size;		// IO响应正文信息的内存大小
};

// 基于Epoll模型的高并发Web服务器(HTTP协议)
class QHttpServer: public noncopyable {
	public:
		inline QHttpServer() :
			m_tcp_listenfd(-1),
			m_udp_listenfd(-1),
			m_start_flag(0),
			m_exit_flag(0),
			m_thread_max(0),
			m_ptr_trd_info(NULL),
			m_epoll_events(NULL),
			m_ptr_cfg_info(NULL),
			m_ptr_trigger(NULL)
		{}

		virtual ~QHttpServer()
		{
			q_close_socket(m_tcp_listenfd);
			q_close_socket(m_udp_listenfd);

			m_exit_flag=true;

			for(int32_t i=0; i<m_thread_max; ++i)
			{
				while(m_ptr_trd_info[i].run_flag!=-1)
					q_sleep(1);

				if(m_ptr_trd_info[i].ptr_buf)
					q_delete_array<char>(m_ptr_trd_info[i].ptr_buf);

				if(m_ptr_trd_info[i].for_worker) {
					struct io_data_t* ptr_io_data=reinterpret_cast<io_data_t*>(m_ptr_trd_info[i].for_worker);

					if(ptr_io_data->request_header)
						q_delete_array<char>(ptr_io_data->request_header);

					if(ptr_io_data->request_content)
						q_delete_array<char>(ptr_io_data->request_content);

					if(ptr_io_data->response_header)
						q_delete_array<char>(ptr_io_data->response_header);

					if(ptr_io_data->response_content)
						q_delete_array<char>(ptr_io_data->response_content);

					q_delete<io_data_t>(ptr_io_data);
				}
			}

			if(m_ptr_trigger)
				q_delete<QTrigger>(m_ptr_trigger);

			if(m_ptr_cfg_info)
				q_delete<config_info_t>(m_ptr_cfg_info);

			if(m_epoll_events)
				q_delete_array<struct epoll_event>(m_epoll_events);

			if(m_ptr_trd_info)
				q_delete_array<thread_info_t>(m_ptr_trd_info);
		}

		// @函数名: 初始化函数
		// @参数01: 配置文件名称
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t init(const char* cfg_file)
		{
			int32_t ret=0;

			// 创建配置信息相关句柄
			m_ptr_cfg_info=q_new<config_info_t>();
			if(m_ptr_cfg_info==NULL) {
				Q_LOG_ERROR("m_ptr_cfg_info is null!");
				return -1;
			}

			// 从外部配置文件中读入配置等相关信息
			QConfigReader conf;
			if(conf.init(cfg_file)!=0) {
				Q_LOG_ERROR("conf init error, cfg_file = (%s)", cfg_file);
				return -2;
			}

			ret=conf.getFieldString("SERVER_NAME", m_ptr_cfg_info->server_name, sizeof(m_ptr_cfg_info->server_name));
			if(ret<0) {
				Q_LOG_ERROR("init SERVER_NAME error, ret = (%d)", ret);
				return -3;
			}

			ret=conf.getFieldString("SERVER_IP", m_ptr_cfg_info->server_ip, sizeof(m_ptr_cfg_info->server_ip));
			if(ret<0) {
				Q_LOG_ERROR("init SERVER_IP error, ret = (%d)", ret);
				return -4;
			}

			ret=conf.getFieldInt32("SERVER_PORT", m_ptr_cfg_info->server_port);
			if(ret<0) {
				Q_LOG_ERROR("init SERVER_PORT error, ret = (%d)", ret);
				return -5;
			}

			ret=conf.getFieldInt32("MONITOR_PORT", m_ptr_cfg_info->monitor_port);
			if(ret<0) {
				Q_LOG_ERROR("init MONITOR_PORT error, ret = (%d)", ret);
				return -6;
			}

			ret=conf.getFieldInt32("SOCK_TIMEOUT", m_ptr_cfg_info->sock_timeout);
			if(ret<0) {
				Q_LOG_ERROR("init SOCK_TIMEOUT error, ret = (%d)", ret);
				return -7;
			}

			ret=conf.getFieldInt32("COMM_THREAD_MAX", m_ptr_cfg_info->comm_thread_max);
			if(ret<0) {
				Q_LOG_ERROR("init COMM_THREAD_MAX error, ret = (%d)", ret);
				return -8;
			}

			ret=conf.getFieldInt32("COMM_BUFFER_SIZE", m_ptr_cfg_info->comm_buffer_size);
			if(ret<0) {
				Q_LOG_ERROR("init COMM_BUFFER_SIZE error, ret = (%d)", ret);
				return -9;
			}

			ret=conf.getFieldInt32("COMM_THREAD_TIMEOUT", m_ptr_cfg_info->comm_thread_timeout);
			if(ret<0) {
				Q_LOG_ERROR("init COMM_THREAD_TIMEOUT error, ret = (%d)", ret);
				return -10;
			}

			ret=conf.getFieldInt32("WORK_THREAD_MAX", m_ptr_cfg_info->work_thread_max);
			if(ret<0) {
				Q_LOG_ERROR("init WORK_THREAD_MAX error, ret = (%d)", ret);
				return -11;
			}

			ret=conf.getFieldInt32("WORK_BUFFER_SIZE", m_ptr_cfg_info->work_buffer_size);
			if(ret<0) {
				Q_LOG_ERROR("init WORK_BUFFER_SIZE error, ret = (%d)", ret);
				return -12;
			}

			ret=conf.getFieldInt32("WORK_THREAD_TIMEOUT", m_ptr_cfg_info->work_thread_timeout);
			if(ret<0) {
				Q_LOG_ERROR("init WORK_THREAD_TIMEOUT error, ret = (%d)", ret);
				return -13;
			}

			ret=conf.getFieldInt32("TASK_QUEUE_SIZE", m_ptr_cfg_info->task_queue_size);
			if(ret<0) {
				Q_LOG_ERROR("init TASK_QUEUE_SIZE error, ret = (%d)", ret);
				return -14;
			}

			ret=conf.getFieldInt32("REQUEST_HEADER_SIZE", m_ptr_cfg_info->request_header_size);
			if(ret<0) {
				Q_LOG_ERROR("init REQUEST_HEADER_SIZE error, ret = (%d)", ret);
				return -15;
			}

			ret=conf.getFieldInt32("REQUEST_CONTENT_SIZE", m_ptr_cfg_info->request_content_size);
			if(ret<0) {
				Q_LOG_ERROR("init REQUEST_CONTENT_SIZE error, ret = (%d)", ret);
				return -16;
			}

			ret=conf.getFieldInt32("RESPONSE_HEADER_SIZE", m_ptr_cfg_info->response_header_size);
			if(ret<0) {
				Q_LOG_ERROR("init RESPONSE_HEADER_SIZE error, ret = (%d)", ret);
				return -17;
			}

			ret=conf.getFieldInt32("RESPONSE_CONTENT_SIZE", m_ptr_cfg_info->response_content_size);
			if(ret<0) {
				Q_LOG_ERROR("init RESPONSE_CONTENT_SIZE error, ret = (%d)", ret);
				return -18;
			}

#if defined (DEBUG)
			Q_DEBUG("SERVER_NAME		= %s", m_ptr_cfg_info->server_name);

			Q_DEBUG("SERVER_IP 		= %s", m_ptr_cfg_info->server_ip);
			Q_DEBUG("SERVER_PORT 		= %d", m_ptr_cfg_info->server_port);
			Q_DEBUG("MONITOR_PORT 		= %d", m_ptr_cfg_info->monitor_port);

			Q_DEBUG("SOCK_TIMEOUT 		= %d", m_ptr_cfg_info->sock_timeout);

			Q_DEBUG("COMM_THREAD_MAX 	= %d", m_ptr_cfg_info->comm_thread_max);
			Q_DEBUG("COMM_BUFFER_SIZE 	= %d", m_ptr_cfg_info->comm_buffer_size);
			Q_DEBUG("COMM_THREAD_TIMEOUT 	= %d", m_ptr_cfg_info->comm_thread_timeout);

			Q_DEBUG("WORK_THREAD_MAX 	= %d", m_ptr_cfg_info->work_thread_max);
			Q_DEBUG("WORK_BUFFER_SIZE 	= %d", m_ptr_cfg_info->work_buffer_size);
			Q_DEBUG("WORK_THREAD_TIMEOUT 	= %d", m_ptr_cfg_info->work_thread_timeout);

			Q_DEBUG("TASK_QUEUE_SIZE 	= %d", m_ptr_cfg_info->task_queue_size);

			Q_DEBUG("REQUEST_HEADER_SIZE 	= %d", m_ptr_cfg_info->request_header_size);
			Q_DEBUG("REQUEST_CONTENT_SIZE 	= %d", m_ptr_cfg_info->request_content_size);
			Q_DEBUG("RESPONSE_HEADER_SIZE 	= %d", m_ptr_cfg_info->response_header_size);
			Q_DEBUG("RESPONSE_CONTENT_SIZE 	= %d", m_ptr_cfg_info->response_content_size);
#endif

			// 初始化任务队列
			if(m_client_task_queue.init(m_ptr_cfg_info->task_queue_size)) {
				Q_LOG_ERROR("init queue error, size = (%d)", m_ptr_cfg_info->task_queue_size);
				return -31;
			}

			// 初始化触发器
			m_ptr_trigger=q_new<QTrigger>();
			if(m_ptr_trigger==NULL) {
				Q_LOG_ERROR("m_ptr_trigger is null!");
				return -32;
			}

			// 初始化socket对象
			if(q_init_socket()) {
				Q_LOG_ERROR("init socket error!");
				return -33;
			}

			// 监听TCP套接字信息
			if(q_TCP_server(m_tcp_listenfd, m_ptr_cfg_info->server_port)) {
				Q_LOG_ERROR("listen tcp (%d) error!", m_ptr_cfg_info->server_port);
				return -34;
			}

			// 监听UDP套接字信息
			if(q_UDP_server(m_udp_listenfd, m_ptr_cfg_info->server_port)) {
				Q_LOG_ERROR("listen udp (%d) error!", m_ptr_cfg_info->server_port);
				return -35;
			}

			// 创建EPOLL实例描述符
			// In the initial epoll_create() implementation, the size argument informed the kernel of the number of file descriptors 
			// that the caller expected to add to the epoll instance.
#if 0
			m_epollfd=epoll_create(10);
#else
			m_epollfd=epoll_create1(0);
#endif
			if(m_epollfd<0) {
				Q_LOG_ERROR("epoll create error!");
				return -36;
			}

			// 创建EPOLL事件池
			m_epoll_events=q_new_array<struct epoll_event>(EPOLL_EVENT_MAX);
			if(m_epoll_events==NULL) {
				Q_LOG_ERROR("epoll events alloc error!");
				return -37;
			}

			// 将TCP套接字加入EPOLL池
			if(add_socket_to_epoll(m_tcp_listenfd)) {
				Q_LOG_ERROR("add socket to epoll error!");
				return -38;
			}

			// 将UDP套接字加入EPOLL池
			if(add_socket_to_epoll(m_udp_listenfd)) {
				Q_LOG_ERROR("add socket to epoll error!");
				return -39;
			}

			// 创建线程相关信息
			m_thread_max=m_ptr_cfg_info->comm_thread_max+m_ptr_cfg_info->work_thread_max;
			m_ptr_trd_info=q_new_array<thread_info_t>(m_thread_max);
			if(m_ptr_trd_info==NULL) {
				Q_LOG_ERROR("create m_ptr_trd_info error!");
				return -51;
			}

			// 创建指定数目的通信线程
			thread_info_t* ptr_trd=m_ptr_trd_info;
			for(int32_t i=0; i!=m_ptr_cfg_info->comm_thread_max; ++i)
			{
				ptr_trd[i].ptr_this=this;
				ptr_trd[i].id=i;
				ptr_trd[i].status=0;
				ptr_trd[i].run_flag=0;
				ptr_trd[i].timeout=m_ptr_cfg_info->comm_thread_timeout;
				ptr_trd[i].buf_size=m_ptr_cfg_info->comm_buffer_size;
				ptr_trd[i].ptr_buf=q_new_array<char>(ptr_trd[i].buf_size);
				if(ptr_trd[i].ptr_buf==NULL) {
					Q_LOG_ERROR("create comm_thread (%d) buffer error", i+1);
					return -52;
				}

				ret=q_create_thread(comm_thread, ptr_trd+i);
				if(ret<0) {
					Q_LOG_ERROR("create comm_thread (%d) error", i+1);
					return -53;
				}

				while(ptr_trd[i].run_flag==0)
					q_sleep(1);

				if(ptr_trd[i].run_flag==-1) {
					Q_LOG_ERROR("start comm_thread (%d) error", i+1);
					return -54;
				}
			}

			// 创建指定数目的工作线程
			ptr_trd+=m_ptr_cfg_info->comm_thread_max;
			for(int32_t i=0; i!=m_ptr_cfg_info->work_thread_max; ++i)
			{
				ptr_trd[i].ptr_this=this;
				ptr_trd[i].id=i;
				ptr_trd[i].status=0;
				ptr_trd[i].run_flag=0;
				ptr_trd[i].timeout=m_ptr_cfg_info->work_thread_timeout;
				ptr_trd[i].buf_size=m_ptr_cfg_info->work_buffer_size;
				ptr_trd[i].ptr_buf=q_new_array<char>(ptr_trd[i].buf_size);
				if(ptr_trd[i].ptr_buf==NULL) {
					Q_LOG_ERROR("create work_thread (%d) buffer error", i+1);
					return -55;
				}

				struct io_data_t* ptr_io_data=q_new<io_data_t>();
				if(ptr_io_data==NULL) {
					Q_LOG_ERROR("ptr_io_data alloc error!");
					return -56;
				}

				ptr_io_data->sock_fd=INVALID_SOCKET;

				ptr_io_data->request_header_size=m_ptr_cfg_info->request_header_size;
				ptr_io_data->request_header_len=0;
				ptr_io_data->request_header=q_new_array<char>(ptr_io_data->request_header_size);
				if(ptr_io_data->request_header==NULL) {
					Q_LOG_ERROR("ptr_io_data->request_header is null!");
					return -57;
				}

				ptr_io_data->request_content_size=m_ptr_cfg_info->request_content_size;
				ptr_io_data->request_content_len=0;
				ptr_io_data->request_content=q_new_array<char>(ptr_io_data->request_content_size);
				if(ptr_io_data->request_content==NULL) {
					Q_LOG_ERROR("ptr_io_data->request_content is null!");
					return -58;
				}

				ptr_io_data->response_header_size=m_ptr_cfg_info->response_header_size;
				ptr_io_data->response_header_len=0;
				ptr_io_data->response_header=q_new_array<char>(ptr_io_data->response_header_size);
				if(ptr_io_data->response_header==NULL) {
					Q_LOG_ERROR("ptr_io_data->response_header is null!");
					return -59;
				}

				ptr_io_data->response_content_size=m_ptr_cfg_info->response_content_size;
				ptr_io_data->response_content_len=0;
				ptr_io_data->response_content=q_new_array<char>(ptr_io_data->response_content_size);
				if(ptr_io_data->response_content==NULL) {
					Q_LOG_ERROR("ptr_io_data->response_content is null!");
					return -60;
				}

				ptr_io_data->keep_alive=HTTP_CLOSE;
				ptr_trd[i].for_worker=reinterpret_cast<void*>(ptr_io_data);

				ret=q_create_thread(work_thread, ptr_trd+i);
				if(ret<0) {
					Q_LOG_ERROR("create work_thread (%d) error", i+1);
					return -61;
				}

				while(ptr_trd[i].run_flag==0)
					q_sleep(1);

				if(ptr_trd[i].run_flag==-1) {
					Q_LOG_ERROR("start work_thread (%d) error", i+1);
					return -62;
				}
			}

			ptr_trd+=m_ptr_cfg_info->work_thread_max;

			m_start_flag=true;
			return 0;
		}

		// @函数名: 启动服务
		int32_t start()
		{
			Q_DEBUG("Input s or S to stop the web server!");

			char ch_;
			std::cin>>ch_;
			while(ch_!='s'&&ch_!='S') {
				Q_DEBUG("Input s or S to stop the web server!");
				std::cin>>ch_;
			}

			Q_DEBUG("Server stop!");
			return 0;
		}

	private:
		// @函数名: 通信线程
		static Q_THREAD_T comm_thread(void* ptr_info)
		{
			thread_info_t* ptr_trd=static_cast<thread_info_t*>(ptr_info);
			QHttpServer* ptr_this=static_cast<QHttpServer*>(ptr_trd->ptr_this);

			int32_t nfds=0;
			int32_t ret=0;

			int32_t sockfd=INVALID_SOCKET;
			int32_t client_fd=INVALID_SOCKET;

			ptr_trd->run_flag=1;

			while(!ptr_this->m_exit_flag) {
				ptr_trd->status=0;

				nfds=::epoll_wait(ptr_this->m_epollfd, ptr_this->m_epoll_events, EPOLL_EVENT_MAX, -1);
				if(nfds<0 && errno!=EINTR) {
					Q_DEBUG("QHttpServer: epoll_wait error!");
					q_sleep(1);
					continue;
				}

				ptr_trd->status=1;
				ptr_trd->sw.start();

				for(int32_t i=0; i<nfds; ++i) {
					sockfd=ptr_this->m_epoll_events[i].data.fd;

					if((ptr_this->m_epoll_events[i].events&EPOLLERR)||(ptr_this->m_epoll_events[i].events&EPOLLHUP)||(!(ptr_this->m_epoll_events[i].events&EPOLLIN))) {
						// An error has occured on this fd, or the socket is not ready for reading (why were we notified then?)
						Q_DEBUG("QHttpServer: epoll error, sockfd = (%d)!", sockfd);
						q_close_socket(sockfd);
						continue;
					} else if(sockfd==ptr_this->m_tcp_listenfd) {
						// We have a notification on the listening socket, which means one or more incoming connections.
						struct sockaddr_in client_addr;
						socklen_t client_addr_len=sizeof(client_addr);

						// 接受客户端连接
						client_fd=::accept(ptr_this->m_tcp_listenfd, (struct sockaddr*)&client_addr, &client_addr_len);
						if(client_fd<0) {
							if(errno==EAGAIN||errno==EWOULDBLOCK) {
								// The socket is marked nonblocking and no connections are present to be accepted.
								continue;
							} else if(errno==EINTR) {
								// The system call was interrupted by a signal that was caught before a valid connection arrived.
								continue;
							} else {
								Q_DEBUG("QHttpServer: accept error!");
								continue;
							}
						}

						Q_DEBUG("----- TCP request from [%s:%d] -----", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
						Q_DEBUG("client_fd = (%d)", client_fd);

#if 0
						// 设置套接字超时时间
						if(q_set_overtime(client_fd, ptr_this->m_ptr_cfg_info->sock_timeout)) {
							Q_DEBUG("QHttpServer: set overtime error!");
							continue;
						}
#endif

						// 将描述符加入epoll池并设置为非阻塞模式
						if(ptr_this->add_socket_to_epoll(client_fd)) {
							Q_DEBUG("QHttpServer: client_fd (%d) add to epoll error!", client_fd);
							q_close_socket(client_fd);
							continue;
						}
					} else if(sockfd==ptr_this->m_udp_listenfd) {
						struct sockaddr_in client_addr;
						socklen_t client_addr_len=sizeof(client_addr);

						Q_DEBUG("----- UDP request from [%s:%d] -----", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
						Q_DEBUG("client_fd = (%d)", sockfd);

						// 接收客户端UDP数据
						ret=recvfrom(ptr_this->m_udp_listenfd, ptr_trd->ptr_buf, ptr_trd->buf_size, 0, (struct sockaddr*)&client_addr, &client_addr_len);
						if(ret<0) {
							Q_DEBUG("QHttpServer: udp recv error, ret = (%d)", ret);
							throw -1;
						}

						// 发送客户端UDP数据
						ret=sendto(ptr_this->m_udp_listenfd, ptr_trd->ptr_buf, ptr_trd->buf_size, 0, (struct sockaddr*)&client_addr, client_addr_len);
						if(ret<0) {
							Q_DEBUG("QHttpServer: udp send error, ret = (%d), ret");
							throw -2;
						}
					} else if(ptr_this->m_epoll_events[i].events&EPOLLIN) {
						// 将Socket描述符写入待处理队列
						ptr_this->m_client_task_queue.push(sockfd);
						ptr_this->m_ptr_trigger->signal();
					} else {
						Q_DEBUG("QHttpServer: epoll error, sockfd = (%d)", sockfd);

						// 如果发生异常, 直接关闭客户端连接
						ptr_this->remove_socket_in_epoll(sockfd);
						q_close_socket(sockfd);
					}
				}
			}

			ptr_trd->run_flag=-1;
			return NULL;
		}

		// @函数名: 工作线程
		static Q_THREAD_T work_thread(void* ptr_info)
		{
			thread_info_t* ptr_trd=static_cast<thread_info_t*>(ptr_info);
			QHttpServer* ptr_this=static_cast<QHttpServer*>(ptr_trd->ptr_this);

			struct io_data_t* ptr_io_data=reinterpret_cast<io_data_t*>(ptr_trd->for_worker);

			Q_SOCKET_T client_fd=INVALID_SOCKET;
			int32_t ret=0;

			ptr_trd->run_flag=1;

			while(!ptr_this->m_exit_flag) {
				ptr_trd->status=0;
				ptr_this->m_ptr_trigger->wait();

				ptr_trd->status=1;
				while(ptr_this->m_client_task_queue.pop_non_blocking(client_fd)==0) {
					try {
						Q_DEBUG("client_fd = (%d)", client_fd);

						ptr_io_data->reset();
						ptr_io_data->sock_fd=client_fd;

						// 获取socket地址
						ret=::getpeername(client_fd, (struct sockaddr*)&(ptr_io_data->sock_addr), (socklen_t*)&(ptr_io_data->sock_len));
						if(ret<0) {
							if(errno==EBADF) {
								Q_DEBUG("QHttpServer: client_fd is not a valid descriptor, client_fd = (%d)!", client_fd);
							} else if(errno==ENOTCONN) {
								Q_DEBUG("QHttpServer: client_fd is not connected, client_fd = (%d)!", client_fd);
							} else if(errno==ENOTSOCK) {
								Q_DEBUG("QHttpServer: client_fd is a file, not a sock, client_fd = (%d)!", client_fd);
							} else if(errno==EINVAL) {
								Q_DEBUG("QHttpServer: sock_len is invalid, client_fd = (%d)!", client_fd);
							} else if(errno==EFAULT) {
								Q_DEBUG("QHttpServer: sock_addr is invalid, client_fd = (%d)!", client_fd);
							} else {
								Q_DEBUG("QHttpServer: getpeername error, client_fd = (%d)!", client_fd);
							}
							throw (int32_t)BAD_REQUEST;
						}

						// 非阻塞数据接收
						ret=ptr_this->recv_buf_nonblocking(client_fd, ptr_io_data->request_header, ptr_io_data->request_header_size);
						if(ret<0) {
							Q_DEBUG("QHttpServer: tcp recv head error, ret = (%d)", ret);
							throw (int32_t)BAD_REQUEST;
						}

						ptr_io_data->request_header_len=ret;
						Q_DEBUG("request_header =\r\n%.*s", ptr_io_data->request_header_len, ptr_io_data->request_header);

						// 处理客户端请求
						ret=ptr_this->process_request(ptr_io_data);
						if(ret) {
							Q_DEBUG("QHttpServer: process request error, ret = (%d)!", ret);
							throw ret;
						}

						// 处理服务端响应内容
						ret=ptr_this->process_response_content(ptr_io_data);
						if(ret) {
							Q_DEBUG("QHttpServer: process func error, ret = (%d)!", ret);
							throw ret;
						}

						// 处理服务端响应头信息
						ret=ptr_this->process_response_header(ptr_io_data);
						if(ret) {
							Q_DEBUG("QHttpServer: process response error, ret = (%d)!", ret);
							throw ret;
						}

						Q_DEBUG("response_header =\r\n%.*s", ptr_io_data->response_header_len, ptr_io_data->response_header);
						Q_DEBUG("response_content =\r\n%.*s", ptr_io_data->response_content_len, ptr_io_data->response_content);

						// 非阻塞数据发送
						ret=ptr_this->send_buf_nonblocking(client_fd, ptr_io_data->response_header, ptr_io_data->response_header_len);
						if(ret<0) {
							Q_DEBUG("QHttpServer: tcp send header error, ret = (%d)!", ret);
							throw (int32_t)INTERNAL_ERROR;
						}

						// 非阻塞数据发送
						ret=ptr_this->send_buf_nonblocking(client_fd, ptr_io_data->response_content, ptr_io_data->response_content_len);
						if(ret<0) {
							Q_DEBUG("QHttpServer: tcp send content error, ret = (%d)!", ret);
							throw (int32_t)INTERNAL_ERROR;
						}

						// Closing the descriptor will make epoll remove it from the set of descriptors which are monitored.
						q_close_socket(client_fd);
					} catch(const int32_t err) {
						switch (err) {
							case MOVED_REQUEST:
								break;
							case FOUND_REQUEST:
								break;
							case NOT_MODIFIED:
								break;
							case BAD_REQUEST:
								break;
							case UNAUTH_REQUEST:
								break;
							case FORBID_REQUEST:
								break;
							case NO_RESOURCE:
								break;
							case METHOD_ERROR:
								break;
							case TIMEOUT_ERROR:
								break;
							case GONE_ERROR:
								break;
							case LENGTH_ERROR:
								break;
							case INTERNAL_ERROR:
								break;
							case BAD_GATEWAY:
								break;
							case SERVICE_ERROR:
								break;
							case GATEWAY_TIMEOUT:
								break;
							case VERSION_ERROR:
								break;
							default:
								break;
						}

						ptr_this->process_response_content_error(ptr_io_data, err);
						ptr_this->process_response_header(ptr_io_data);

						Q_DEBUG("response_header =\r\n%.*s", ptr_io_data->response_header_len, ptr_io_data->response_header);
						Q_DEBUG("response_content =\r\n%.*s", ptr_io_data->response_content_len, ptr_io_data->response_content);

						ptr_this->send_buf_nonblocking(client_fd, ptr_io_data->response_header, ptr_io_data->response_header_len);
						ptr_this->send_buf_nonblocking(client_fd, ptr_io_data->response_content, ptr_io_data->response_content_len);

						// Closing the descriptor will make epoll remove it from the set of descriptors which are monitored.
						q_close_socket(client_fd);
					}
				}
			}

			ptr_trd->run_flag=-1;
			return NULL;
		}

		// @函数名: 将监听描述符加入EPOLL池
		// @参数01: 监听描述符
		// @参数02: 是否设置为非阻塞模式
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t add_socket_to_epoll(int32_t fd, bool nonblocking=true)
		{
			struct epoll_event event;
			memset(&event, 0, sizeof(event));
			event.data.fd=fd;
			// EPOLL对文件描述符的两种操作方式: 水平触发(LT)和边沿触发(ET)
			// 水平触发(LT)检测到文件描述符有事件发生便会通知应用程序, 应用程序可以不立即处理该事件, 因为后来还会通知该事件。
			// 边沿触发(ET)检测到有事件通知后, 必须立即处理该事件, 因为后面将不再触发。
			event.events=EPOLLIN|EPOLLET|EPOLLRDHUP;

			if(nonblocking&&q_set_nonblocking(fd)) {
				Q_DEBUG("QHttpServer: set nonblocking error!");
				return -1;
			}

			if(epoll_ctl(m_epollfd, EPOLL_CTL_ADD, fd, &event)<0) {
				Q_DEBUG("QHttpServer: epoll ctl add error!");
				return -2;
			}

			return 0;
		}

		// @函数名: 修改描述符状态信息
		// @参数01: 监听描述符
		// @参数02: EPOLL状态信息
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t modify_socket_in_epoll(int32_t fd, int32_t en)
		{
			struct epoll_event event;
			memset(&event, 0, sizeof(event));
			event.data.fd=fd;
			event.events=en|EPOLLET|EPOLLONESHOT|EPOLLRDHUP;

			if(epoll_ctl(m_epollfd, EPOLL_CTL_MOD, fd, &event)<0) {
				Q_DEBUG("QHttpServer: epoll ctl mod error!");
				return -1;
			}
			return 0;
		}

		// @函数名: 从Epoll池中删除socket描述符
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t remove_socket_in_epoll(int32_t fd)
		{
			if(epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, 0)<0) {
				Q_DEBUG("QHttpServer: epoll ctl remove error!");
				return -1;
			}
			return 0;
		}

		// @函数名: 非阻塞socket数据接收函数
		// @参数01: 待接收套接字描述符
		// @参数02: 待接收数据buffer
		// @参数03: 待接收数据buffer的长度
		// @返回值: 成功返回读取数据的实际长度, 失败返回<0的错误码
		int32_t recv_buf_nonblocking(Q_SOCKET_T in_socket, char* in_buf, int32_t request_header_size)
		{
			int32_t retval=0, finlen=0;
			while(true) {
				if(finlen>=request_header_size)
					return -1;

				// The recv call return the number of bytes received, or -1 if an error occurred. In the event of an error, errno is set to indicate the error.
				retval=(int32_t)::recv(in_socket, in_buf+finlen, request_header_size-finlen, 0);
				if(retval<0) {
					if(errno==EAGAIN||errno==EWOULDBLOCK)
						break;
					else
						return -2;
				} else if(retval>0) {
					finlen+=retval;
				} else {
					// When a stream socket peer has performed an orderly shutdown, the return value will be 0 (the traditional "end-of-file" return).
					// The value 0 may also be returned if the requested number of bytes to receive from a stream socket was 0.
					break;
				}
			}

			return finlen;
		}

		// @函数名: 非阻塞socket数据发送函数
		// @参数01: 待发送套接字描述符
		// @参数02: 待发送数据buffer
		// @参数03: 待发送数据buffer的长度
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t send_buf_nonblocking(Q_SOCKET_T in_socket, char* in_buf, int32_t buf_len)
		{
			int32_t retval=0, finlen=0;
			do {
				retval=(int32_t)::send(in_socket, in_buf+finlen, buf_len-finlen, 0);
				if(retval<0) {
					if(errno==EAGAIN||errno==EWOULDBLOCK)
						break;
					if(errno==EINTR)
						continue;
					return -1;
				} else if(retval>0) {
					finlen+=retval;
				} else {
					break;
				}
			} while(retval>0&&finlen<buf_len);
			if(finlen<buf_len)
				return -3;
			return 0;
		}

#if 0
		// @函数名: 解析HTTP协议请求行信息
		int32_t parse_request_line(struct io_data_t* ptr_io_data)
		{
			char* url=strpbrk(ptr_io_data->request_header, " \t");
			if(url==NULL) {
				Q_DEBUG("QHttpServer: bad request!");
				return -1;
			}

			char* method=ptr_io_data->request_header;
			if(::q_strncasecmp(method, "GET", 3)==0) {
				ptr_io_data->method=GET;
			} else {
				Q_DEBUG("QHttpServer: method not supported!");
				return -2;
			}

			url+=strspn(url, " \t");

			char* version=strpbrk(url, " \t");
			if(version==NULL) {
				Q_DEBUG("QHttpServer: version error!");
				return -3;
			}
			*version++='\0';
			version+=strspn(version, " \t");

			if(q_strncasecmp(version, "HTTP:/1.1", 9)==0) {
				ptr_io_data->version=HTTP_1_1;
			} else if(q_strncasecmp(version, "HTTP:/1.0", 9)==0) {
				ptr_io_data->version=HTTP_1_0;
			} else {
				Q_DEBUG("QHttpServer: version error!");
				return -4;
			}

			if(q_strncasecmp(url, "http://", 7)==0) {
				url+=7;
				url=strchr(url, '/');
			}

			if(!url||url[0]!='/') {
				Q_DEBUG("QHttpServer: request url error!");
				return -5;
			} else {
				ptr_io_data->url=url;
			}

			return 0;
		}
#endif

		// @函数名: 处理请求HTTP协议的相关字段
		int32_t process_request(struct io_data_t* ptr_io_data)
		{
			char* ptr_text=ptr_io_data->request_header;
			char* ptr_temp=NULL;

			// 解析请求行
			if(q_strncasecmp(ptr_text, "GET", 3)==0) {
				ptr_io_data->method=GET;
				ptr_text+=3;
			} else {
				Q_DEBUG("QHttpServer: bad request, method not supported!");
				return METHOD_ERROR;
			}

			while(*ptr_text&&(*ptr_text==' '||*ptr_text=='\t')&&*ptr_text!='\r'&&*ptr_text!='\n')
				ptr_text++;
			ptr_temp=ptr_text;
			while(*ptr_temp&&*ptr_temp!=' '&&*ptr_temp!='\t'&&*ptr_temp!='\r'&&*ptr_temp!='\n')
				ptr_temp++;
			ptr_io_data->url.ptr=ptr_text;
			ptr_io_data->url.length=ptr_temp-ptr_text;

			ptr_text=ptr_temp;
			while(*ptr_text&&(*ptr_text==' '||*ptr_text=='\t')&&*ptr_text!='\r'&&*ptr_text!='\n')
				ptr_text++;

			if(q_strncasecmp(ptr_text, "HTTP/1.1", 8)==0) {
				ptr_io_data->version=HTTP_1_1;
				ptr_text+=8;
			} else if(q_strncasecmp(ptr_text, "HTTP/1.0", 8)==0) {
				ptr_io_data->version=HTTP_1_0;
				ptr_text+=8;
			} else {
				Q_DEBUG("QHttpServer: bad request, version error!");
				return VERSION_ERROR;
			}

			// 解析其它请求参数
			while(*ptr_text) {
				if(q_strncasecmp(ptr_text, "Connection:", 11)==0) {
					ptr_text+=11;
					while(*ptr_text&&(*ptr_text==' '||*ptr_text=='\t')&&*ptr_text!='\r'&&*ptr_text!='\n')
						ptr_text++;
					if(q_strncasecmp(ptr_text, "keep-alive", 10)==0) {
						ptr_io_data->keep_alive=HTTP_KEEP_ALIVE;
						ptr_text+=10;
					}
					continue;
				} else if(q_strncasecmp(ptr_text, "Host:", 5)==0) {
					ptr_text+=5;
					while(*ptr_text&&(*ptr_text==' '||*ptr_text=='\t')&&*ptr_text!='\r'&&*ptr_text!='\n')
						ptr_text++;
					ptr_temp=ptr_text;
					while(*ptr_temp!='\r'&&*ptr_temp!='\n')
						ptr_temp++;
					ptr_io_data->host.ptr=ptr_text;
					ptr_io_data->host.length=ptr_temp-ptr_text;
					ptr_text=ptr_temp;
					continue;
				} else if(q_strncasecmp(ptr_text, "Referer:", 8)==0) {
					ptr_text+=8;
					while(*ptr_text&&(*ptr_text==' '||*ptr_text=='\t')&&*ptr_text!='\r'&&*ptr_text!='\n')
						ptr_text++;
					ptr_temp=ptr_text;
					while(*ptr_temp!='\r'&&*ptr_temp!='\n')
						ptr_temp++;
					ptr_io_data->referer.ptr=ptr_text;
					ptr_io_data->referer.length=ptr_temp-ptr_text;
					ptr_text=ptr_temp;
					continue;
				} else if(q_strncasecmp(ptr_text, "User-Agent:", 11)==0) {
					ptr_text+=11;
					while(*ptr_text&&(*ptr_text==' '||*ptr_text=='\t')&&*ptr_text!='\r'&&*ptr_text!='\n')
						ptr_text++;
					ptr_temp=ptr_text;
					while(*ptr_temp!='\r'&&*ptr_temp!='\n')
						ptr_temp++;
					ptr_io_data->user_agent.ptr=ptr_text;
					ptr_io_data->user_agent.length=ptr_temp-ptr_text;
					ptr_text=ptr_temp;
					continue;
				} else if(q_strncasecmp(ptr_text, "Accept:", 7)==0) {
					ptr_text+=7;
					while(*ptr_text&&(*ptr_text==' '||*ptr_text=='\t')&&*ptr_text!='\r'&&*ptr_text!='\n')
						ptr_text++;
					ptr_temp=ptr_text;
					while(*ptr_temp!='\r'&&*ptr_temp!='\n')
						ptr_temp++;
					ptr_io_data->accept.ptr=ptr_text;
					ptr_io_data->accept.length=ptr_temp-ptr_text;
					ptr_text=ptr_temp;
					continue;
				} else if(q_strncasecmp(ptr_text, "Accept-Encoding:", 16)==0) {
					ptr_text+=16;
					while(*ptr_text&&(*ptr_text==' '||*ptr_text=='\t')&&*ptr_text!='\r'&&*ptr_text!='\n')
						ptr_text++;
					ptr_temp=ptr_text;
					while(*ptr_temp!='\r'&&*ptr_temp!='\n')
						ptr_temp++;
					ptr_io_data->accept_encoding.ptr=ptr_text;
					ptr_io_data->accept_encoding.length=ptr_temp-ptr_text;
					ptr_text=ptr_temp;
					continue;
				} else if(q_strncasecmp(ptr_text, "Accept-Language:", 16)==0) {
					ptr_text+=16;
					while(*ptr_text&&(*ptr_text==' '||*ptr_text=='\t')&&*ptr_text!='\r'&&*ptr_text!='\n')
						ptr_text++;
					ptr_temp=ptr_text;
					while(*ptr_temp!='\r'&&*ptr_temp!='\n')
						ptr_temp++;
					ptr_io_data->accept_language.ptr=ptr_text;
					ptr_io_data->accept_language.length=ptr_temp-ptr_text;
					ptr_text=ptr_temp;
					continue;
				}
				++ptr_text;
			}

			return 0;
		}

		// @函数名: 服务器响应正文信息处理函数
		int32_t process_response_content(struct io_data_t* ptr_io_data)
		{
#if 0
			char real_file[128];
			struct stat buf;
			FILE* fp=NULL;

			strncpy(real_file, ptr_io_data->url.ptr+1, ptr_io_data->url.length-1);

			if(::stat(real_file, &buf)<0) {
				Q_DEBUG("QHttpServer: stat error, %s!", strerror(errno));
				return NO_RESOURCE;
			}

			if(!(buf.st_mode&S_IROTH)) {
				Q_DEBUG("QHttpServer: forbidden request!");
				return FORBID_REQUEST;
			}

			if(buf.st_size<=0||buf.st_size>ptr_io_data->request_content_size) {
				Q_DEBUG("QHttpServer: buf.st_size = %d", buf.st_size);
				return INTERNAL_ERROR;
			}

			fp=::fopen(real_file, "rb");
			if(fp==NULL) {
				Q_DEBUG("QHttpServer: fopen %s error!", real_file);
				return NO_RESOURCE;
			}

			if(::fread(ptr_io_data->response_content, buf.st_size, 1, fp)<=0) {
				Q_DEBUG("QHttpServer: fread error!");
				::fclose(fp);
				return INTERNAL_ERROR;
			}

			::fclose(fp);

			ptr_io_data->response_content_len=buf.st_size;
#else
			char* ptr_beg=ptr_io_data->response_content;
			char* ptr_end=ptr_io_data->response_content+ptr_io_data->response_content_size;
			int32_t ret=0;

			ret=q_snprintf(ptr_beg, ptr_io_data->response_content_size, "<html><title>GOOD</title><content>HELLO, I LOVE YOU!</content></html>");
			if(ret<0||ptr_beg+ret>=ptr_end) {
				Q_DEBUG("QHttpServer: process response content error!");
				return INTERNAL_ERROR;
			} else {
				ptr_io_data->response_content_len=ret;
			}
#endif

			ptr_io_data->status=OK_REQUEST;
			strcpy(ptr_io_data->status_line, get_http_method_string(OK_REQUEST));
			ptr_io_data->server=HTTP_WEB_SERVER;
			ptr_io_data->content_length=ptr_io_data->response_content_len;
			ptr_io_data->content_type=TEXT_HTML;

			return 0;
		}

		// @函数名: 服务器响应头信息处理函数
		int32_t process_response_header(struct io_data_t* ptr_io_data)
		{
			ptr_io_data->response_header_len=q_snprintf(ptr_io_data->response_header, ptr_io_data->response_header_size-1, \
					"%s %d %s\r\n" \
					"Server: %s\r\n" \
					"Content-Length: %d\r\n" \
					"Content-Type: %s\r\n" \
					"Connection: %s\r\n" \
					"\r\n", \
					get_http_version_string(ptr_io_data->version),
					ptr_io_data->status, \
					ptr_io_data->status_line, \
					ptr_io_data->server, \
					ptr_io_data->content_length, \
					get_http_content_type_string(ptr_io_data->content_type),
#if 1
					"Close");
#else
					get_http_connection_string(ptr_io_data->keep_alive));
#endif
			if(ptr_io_data->response_header_len<0||ptr_io_data->response_header_len>=ptr_io_data->response_header_size)
				return INTERNAL_ERROR;
			return 0;
		}

		// @函数名: 服务器错误响应处理函数
		int32_t process_response_content_error(struct io_data_t* ptr_io_data, int32_t status)
		{
			ptr_io_data->response_content_len=q_snprintf(ptr_io_data->response_content, ptr_io_data->response_content_size-1, \
					"<html><title>error_%d.html</title><body>%s: %d error!</body></html>", \
					status, \
					HTTP_WEB_SERVER, \
					status);
			if(ptr_io_data->response_content_len<0||ptr_io_data->response_content_len>=ptr_io_data->response_content_size)
				return INTERNAL_ERROR;

			ptr_io_data->status=status;
			strcpy(ptr_io_data->status_line, get_http_method_string(status));
			ptr_io_data->server=HTTP_WEB_SERVER;
			ptr_io_data->content_length=ptr_io_data->response_content_len;
			ptr_io_data->content_type=TEXT_HTML;

			return 0;
		}

		// @函数名: 获取HTTP协议状态码
		const char* get_http_method_string(int32_t status)
		{
			switch(status) {
				case OK_REQUEST:
					return "OK Request";
				case NO_CONTENT:
					return "No Content";
				case MOVED_REQUEST:
					return "Moved Request";
				case TEMP_REDIRECT:
					return "Temporary Redirect";
				case BAD_REQUEST:
					return "Bad Request";
				case FORBID_REQUEST:
					return "Forbidden Request";
				case METHOD_ERROR:
					return "Method Error";
				case TIMEOUT_ERROR:
					return "Timeout Error";
				case INTERNAL_ERROR:
					return "Internal Error";
				case BAD_GATEWAY:
					return "Bad Gateway";
				case SERVICE_ERROR:
					return "Service Unavailable";
				case GATEWAY_TIMEOUT:
					return "Gateway Timeout";
				case VERSION_ERROR:
					return "Version Error";
				default:
					return "Unknown Error";
			};
		}

		// @函数名: 获取HTTP协议版本号
		const char* get_http_version_string(version_t version)
		{
			if(version==HTTP_1_0)
				return "HTTP/1.0";
			else
				return "HTTP/1.1";
		}

		// @函数名: 获取HTTP协议响应内容数据类型
		const char* get_http_content_type_string(content_type_t content_type)
		{
			switch(content_type) {
				case TEXT_HTML:
					return "text/html";
				case TEXT_XML:
					return "text/xml";
				case TEXT_CSS:
					return "text/css";
				case TEXT_JAVASCRIPT:
					return "text/javascript";
				case TEXT_PLAIN:
					return "text/plain";
				case IMAGE_JPEG:
					return "image/jpeg";
				case IMAGE_PNG:
					return "image/png";
				case IMAGE_GIF:
					return "image/gif";
				default:
					return "unknown";
			};
		}

		// @函数名: 获取HTTP协议连接信息
		const char* get_http_connection_string(connection_t keep_alive)
		{
			if(keep_alive==HTTP_KEEP_ALIVE)
				return "Keep-Alive";
			else
				return "Close";
		}

	protected:
		char			m_listen_ip[IPADDR_MAX_LEN];	// 服务器监听绑定IP地址
		int32_t			m_listen_port;			// 服务器监听端口号

		int32_t			m_tcp_listenfd;			// 监听TCP连接套接字
		int32_t			m_udp_listenfd;			// 监听UDP连接套接字

		bool8_t			m_start_flag;			// 服务启动标识
		bool8_t			m_exit_flag;			// 服务退出标识

		int32_t			m_thread_max;			// 服务最大线程数目
		thread_info_t*		m_ptr_trd_info;			// 线程信息句柄

		struct epoll_event*	m_epoll_events;			// EPOLL事件池
		int32_t			m_epollfd;			// EPOLL描述符

		config_info_t*		m_ptr_cfg_info;			// 外部配置文件信息结构体

		QQueue<Q_SOCKET_T>	m_client_task_queue;		// 客户端请求任务队列
		QTrigger*		m_ptr_trigger;			// 任务队列触发器
};

Q_END_NAMESPACE

#endif // __QHTTPSERVER_H_
