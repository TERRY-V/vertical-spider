#include "qtcpsocket.h"

Q_BEGIN_NAMESPACE

// TCP通讯客户端
QTcpClient::QTcpClient()
{
	this->server_port_=TCP_DEFAULT_SERVER_PORT;
	this->server_timeout_=TCP_DEFAULT_SERVER_TIMEOUT;
	this->sock_=TCP_DEFAULT_INVALID_SOCKET;
	this->version_=TCP_HEADER_VERSION;
	this->protocol_type_=TCP_DEFAULT_PROTOCOL_TYPE;
	this->source_type_=TCP_DEFAULT_SOURCE_TYPE;
	this->command_type_=TCP_DEFAULT_COMMAND_TYPE;
	this->operate_type_=TCP_DEFAULT_OPERATE_TYPE;
	this->request_buffer_=NULL;
	this->request_buffer_size_=TCP_DEFAULT_REQUEST_SIZE;
	this->reply_buffer_=NULL;
	this->reply_buffer_size_=TCP_DEFAULT_REPLY_SIZE;
}

QTcpClient::~QTcpClient()
{
	q_close_socket(sock_);
	q_delete_array<char>(this->request_buffer_);
	q_delete_array<char>(this->reply_buffer_);
}

void QTcpClient::setHost(const char* server_ip, uint16_t server_port)
{
	strcpy(this->server_ip_, server_ip);
	this->server_port_=server_port;
}

void QTcpClient::setTimeout(int32_t timeout)
{
	this->server_timeout_=timeout;
}

void QTcpClient::setVersion(uint64_t version)
{
	this->version_=version;
}

void QTcpClient::setProtocolType(uint16_t protocol_type)
{
	this->protocol_type_=protocol_type;
}

void QTcpClient::setSourceType(uint16_t source_type)
{
	this->source_type_=source_type;
}

void QTcpClient::setCommandType(uint16_t command_type)
{
	this->command_type_=command_type;
}

void QTcpClient::setOperateType(uint16_t operate_type)
{
	this->operate_type_=operate_type;
}

int32_t QTcpClient::sendRequest(const char* ptr_data, int32_t data_len)
{
	if(ptr_data==NULL||data_len<0)
		return TCP_ERR;

	int32_t packet_len=sizeof(requestHeader)+sizeof(uint16_t)+sizeof(uint32_t)+data_len;
	request_buffer_size_=packet_len;

	request_buffer_=q_new_array<char>(request_buffer_size_);
	if(request_buffer_==NULL)
		return TCP_ERR_HEAP_ALLOC;

	requestHeader* request_header=reinterpret_cast<requestHeader*>(request_buffer_);
	request_header->version=TCP_HEADER_VERSION;
	request_header->length=packet_len-sizeof(baseHeader);

	request_header->protocol_type=protocol_type_;
	request_header->source_type=source_type_;
	request_header->command_type=command_type_;

	*(uint16_t *)(request_buffer_+sizeof(requestHeader))=operate_type_;
	*(uint32_t *)(request_buffer_+sizeof(requestHeader)+sizeof(uint16_t))=data_len;
	memcpy(request_buffer_+sizeof(requestHeader)+sizeof(uint16_t)+sizeof(int32_t), ptr_data, data_len);

	if(q_init_socket()<0)
		return TCP_ERR_SOCKET_INIT;

	if(q_connect_socket(sock_, server_ip_, server_port_)<0)
		return TCP_ERR_SOCKET_CONNECTION;

	if(q_set_overtime(sock_, server_timeout_)<0)
		return TCP_ERR_SOCKET_TIMEOUT;

	if(q_sendbuf(sock_, request_buffer_, packet_len)<0)
		return TCP_ERR_SOCKET_SEND;

	return TCP_OK;
}

int32_t QTcpClient::getReply(networkReply* reply)
{
	baseHeader base_header;
	if(q_recvbuf(sock_, (char*)&base_header, sizeof(baseHeader))<0)
		return TCP_ERR_SOCKET_RECV;

	if(base_header.version!=TCP_HEADER_VERSION)
		return TCP_ERR_SOCKET_VERSION;

	if(base_header.length<=0||base_header.length+sizeof(baseHeader)>1<<20)
		return TCP_ERR_PACKET_LENGTH;

	reply_buffer_size_=base_header.length;
	reply_buffer_=q_new_array<char>(reply_buffer_size_);
	if(reply_buffer_==NULL)
		return TCP_ERR_HEAP_ALLOC;

	if(q_recvbuf(sock_, reply_buffer_, base_header.length))
		return TCP_ERR_SOCKET_RECV;

	replyParam* reply_param=reinterpret_cast<replyParam*>(reply_buffer_);

	reply->status=reply_param->status;
	if(!reply->status && base_header.length!=sizeof(replyParam)) {
		reply->length=*(uint32_t*)(reply_buffer_+sizeof(replyParam));
		reply->data=reply_buffer_+sizeof(replyParam)+sizeof(int32_t);
	}

	return TCP_OK;
}

QTcpServer::QTcpServer()
{
	this->pid_=getpid();
	this->pidfile_=q_strdup(TCP_DEFAULT_PIDFILE);
	this->unixtime_=time(NULL);
	this->hz_=TCP_DEFAULT_HZ;
	this->arch_bits_=(sizeof(long)==8)?64:32;
	this->start_flag_=false;
	this->exit_flag_=false;
	this->server_name_=NULL;
	this->server_port_=TCP_DEFAULT_SERVER_PORT;
	this->server_timeout_=TCP_DEFAULT_SERVER_TIMEOUT;
	this->thread_info_=NULL;
	this->thread_max_=0;
	this->comm_thread_max_=TCP_DEFAULT_THREAD_NUM;
	this->comm_buffer_size_=TCP_DEFAULT_BUFFER_SIZE;
	this->comm_thread_timeout_=TCP_DEFAULT_THREAD_TIMEOUT;
	this->work_thread_max_=TCP_DEFAULT_THREAD_NUM;
	this->work_buffer_size_=TCP_DEFAULT_BUFFER_SIZE;
	this->work_thread_timeout_=TCP_DEFAULT_THREAD_TIMEOUT;
	this->send_thread_max_=TCP_DEFAULT_THREAD_NUM;
	this->send_buffer_size_=TCP_DEFAULT_BUFFER_SIZE;
	this->send_thread_timeout_=TCP_DEFAULT_THREAD_TIMEOUT;
	this->listen_sock_=TCP_DEFAULT_INVALID_SOCKET;
	this->queue_size_=TCP_DEFAULT_QUEUE_SIZE;
	this->client_request_size_=TCP_DEFAULT_REQUEST_SIZE;
	this->client_reply_size_=TCP_DEFAULT_REPLY_SIZE;
	this->header_size_=TCP_DEFAULT_HEADER_SIZE;
	this->chunk_queue_=NULL;
	this->client_queue_=NULL;
	this->client_trigger_=NULL;
	this->send_ip_=NULL;
	this->send_port_=0;
	this->data_path_=NULL;
	this->img_path_=NULL;
	this->read_path_=NULL;
	this->write_path_=NULL;
	this->monitor_=NULL;
	this->monitor_port_=TCP_DEFAULT_MONITOR_PORT;
	this->logger_=NULL;
	this->log_path_=q_strdup(TCP_DEFAULT_LOG_PATH);
	this->log_prefix_=q_strdup(TCP_DEFAULT_LOG_PREFIX);
	this->log_size_=TCP_DEFAULT_LOG_SIZE;
	this->log_screen_=TCP_DEFAULT_LOG_SCREEN;
	this->stat_starttime_=time(NULL);
	this->stat_lastinteraction_=0;
	this->stat_numconnections_=0;
	this->stat_succconnections_=0;
	this->stat_failedconnections_=0;
	this->stat_rejectedconnections_=0;
}

QTcpServer::~QTcpServer()
{
	exit_flag_=true;

	q_free(pidfile_);
	q_free(log_path_);
	q_free(log_prefix_);

	q_close_socket(listen_sock_);

	free_thread_info();

	clientInfo* client_info=NULL;
	while(chunk_queue_->pop_non_blocking(client_info)==0)
	{
		q_delete_array<char>(client_info->request_buffer);
		q_delete_array<char>(client_info->reply_buffer);
	}
	q_delete< QQueue<clientInfo*> >(chunk_queue_);

	q_delete< QQueue<clientInfo*> >(client_queue_);
	q_delete< QTrigger >(client_trigger_);

	q_free(send_ip_);

	q_free(data_path_);
	q_free(img_path_);
	q_free(read_path_);
	q_free(write_path_);

	fclose(write_fp_);
	write_fp_=NULL;

	q_free(log_path_);
	q_free(log_prefix_);
	q_delete<QLogger>(logger_);
	q_delete<QRemoteMonitor>(monitor_);
}

int32_t QTcpServer::init(const char* cfg_file)
{
	struct timeval tv;
	setlocale(LC_COLLATE, "");
	srand(time(NULL)^getpid());
	gettimeofday(&tv, NULL);

	Q_INFO("Hello, initing the server...");
	int32_t ret=0;

	/* config */
	ret=load_server_config(cfg_file);
	if(ret<0) {
		Q_FATAL("server configuration loading error, ret = (%d)!", ret);
		return TCP_ERR;
	}

	/* logger */
	logger_=q_new<QLogger>();
	if(logger_==NULL) {
		Q_FATAL("logger_ alloc error, null value!");
		return TCP_ERR;
	}

	ret=logger_->init(log_path_, log_prefix_, log_size_);
	if(ret<0) {
		Q_FATAL("logger_ init error, ret = (%d)!", ret);
		return TCP_ERR;
	}

	/* queue */
	chunk_queue_=q_new< QQueue<clientInfo*> >();
	if(chunk_queue_==NULL) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"chunk_queue_ alloc error, null value!");
		return TCP_ERR;
	}
	
	ret=chunk_queue_->init(queue_size_+1);
	if(ret<0) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"init chunk_queue_ error, ret = (%d)", \
				ret);
		return TCP_ERR;
	}

	for(int32_t i=0; i<queue_size_; ++i)
	{
		clientInfo* client_info=q_new<clientInfo>();
		if(client_info==NULL) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"client_info is null!");
			return TCP_ERR;
		}

		client_info->request_buffer_size=client_request_size_;
		client_info->request_buffer=q_new_array<char>(client_info->request_buffer_size);
		if(client_info->request_buffer==NULL) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"client_info->request_buffer is null!");
			return TCP_ERR;
		}

		client_info->reply_buffer_size=client_reply_size_;
		client_info->reply_buffer=q_new_array<char>(client_info->reply_buffer_size);
		if(client_info->reply_buffer==NULL) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"client_info->reply_buffer is null!");
			return TCP_ERR;
		}

		chunk_queue_->push(client_info);
	}

	/* client queue */
	client_queue_=q_new< QQueue<clientInfo*> >();
	if(client_queue_==NULL) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"client_queue_ is null!");
		return TCP_ERR;
	}

	ret=client_queue_->init(queue_size_+1);
	if(ret<0) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"init client_queue_ error, ret = (%d)", \
				ret);
		return TCP_ERR;
	}

	client_trigger_=q_new<QTrigger>();
	if(client_trigger_==NULL) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"client_trigger_ is null!");
		return TCP_ERR;
	}

	/* directory */
	if(!QDir::mkdir(data_path_)) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"mkdir data_path_ (%s) error!", \
				data_path_);
		return TCP_ERR;
	}

	if(!QDir::mkdir(img_path_)) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"mkdir img_path_ (%s) error!", \
				img_path_);
		return TCP_ERR;
	}

	if(access(write_path_, 00)==0)
	{
		ret=q_repair_file(write_path_, (const char*)&TCP_TAILER_FILE_MARK, sizeof(TCP_TAILER_FILE_MARK));
		if(ret<0) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"q_repair_file (%s) error, ret = (%d)!", \
					write_path_, \
					ret);
			return TCP_ERR;
		}

		write_fp_=::fopen(write_path_, "rb+");
		if(write_fp_==NULL) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"fopen (%s) error!", \
					write_path_);
			return TCP_ERR;
		}

		if(::fseek(write_fp_, 0, SEEK_END)!=0) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"fseek (%s) error!", \
					write_path_);
			return TCP_ERR;
		}
	}

	/* network */
	ret=q_init_socket();
	if(ret<0) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"init socket error, ret = (%d)!", \
				ret);
		return TCP_ERR;
	}

	ret=q_TCP_server(listen_sock_, server_port_);
	if(ret<0) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"TCP server error, listen_port = (%d), ret = (%d)!", \
				server_port_, \
				ret);
		return TCP_ERR;
	}

	/* threads */
	thread_max_=comm_thread_max_+work_thread_max_+send_thread_max_;

	thread_info_=q_new_array<threadInfo>(thread_max_);
	if(thread_info_==NULL) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"create thread_info_ error, ret = (%d)", \
				ret);
		return TCP_ERR;
	}

	threadInfo* ptr_trd=thread_info_;
	for(int32_t i=0; i!=comm_thread_max_; ++i)
	{
		ptr_trd[i].pthis=this;
		ptr_trd[i].id=i;
		ptr_trd[i].status=0;
		ptr_trd[i].flag=0;
		ptr_trd[i].timeout=comm_thread_timeout_;
		ptr_trd[i].buf_size=comm_buffer_size_;
		ptr_trd[i].ptr_buf=q_new_array<char>(ptr_trd[i].buf_size);
		if(ptr_trd[i].ptr_buf==NULL) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"create comm_thread (%d) buffer error", \
					i+1);
			return TCP_ERR;
		}

		ret=q_create_thread(QTcpServer::comm_thread, ptr_trd+i);
		if(ret<0) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"create comm_thread (%d) error", \
					i+1);
			return TCP_ERR;
		}

		while(ptr_trd[i].flag!=1)
			q_sleep(1);

		if(ptr_trd[i].flag==-1) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"start comm_thread (%d) error", \
					i+1);
			return TCP_ERR;
		}
	}

	ptr_trd+=comm_thread_max_;
	for(int32_t i=0; i!=work_thread_max_; ++i)
	{
		ptr_trd[i].pthis=this;
		ptr_trd[i].id=i;
		ptr_trd[i].status=0;
		ptr_trd[i].flag=0;
		ptr_trd[i].timeout=work_thread_timeout_;
		ptr_trd[i].buf_size=work_buffer_size_;
		ptr_trd[i].ptr_buf=q_new_array<char>(ptr_trd[i].buf_size);
		if(ptr_trd[i].ptr_buf==NULL) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"create work_thread (%d) buffer error", \
					i+1);
			return TCP_ERR;
		}

		/********************************* START ********************************/
		ret=server_fun_init(ptr_trd[i].for_worker);
		if(ret<0) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"server_fun_init (%d) error!", \
					i+1);
			return TCP_ERR;
		}
		/********************************* FINISH *******************************/

		ret=q_create_thread(QTcpServer::work_thread, ptr_trd+i);
		if(ret<0) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"create work_thread (%d) error", \
					i+1);
			return TCP_ERR;
		}

		while(ptr_trd[i].flag!=1)
			q_sleep(1);

		if(ptr_trd[i].flag==-1) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"start work_thread (%d) error", \
					i+1);
			return TCP_ERR;
		}
	}

	ptr_trd+=work_thread_max_;
	for(int32_t i=0; i!=send_thread_max_; ++i)
	{
		ptr_trd[i].pthis=this;
		ptr_trd[i].id=i;
		ptr_trd[i].status=0;
		ptr_trd[i].flag=0;
		ptr_trd[i].timeout=send_thread_timeout_;
		ptr_trd[i].buf_size=send_buffer_size_;
		ptr_trd[i].ptr_buf=q_new_array<char>(ptr_trd[i].buf_size);
		if(ptr_trd[i].ptr_buf==NULL) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"create send_thread (%d) buffer error", \
					i+1);
			return TCP_ERR;
		}

		ret=q_create_thread(QTcpServer::send_thread, ptr_trd+i);
		if(ret<0) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"create send_thread (%d) error", \
					i+1);
			return TCP_ERR;
		}

		while(ptr_trd[i].flag!=1)
			q_sleep(1);

		if(ptr_trd[i].flag==-1) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"start send_thread (%d) error", \
					i+1);
			return TCP_ERR;
		}
	}

	/* monitor */
	monitor_=q_new<QRemoteMonitor>();
	if(monitor_==NULL) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"monitor_ is null!");
		return TCP_ERR;
	}

	ret=monitor_->init(monitor_port_, 10000, get_thread_state, this, 1);
	if(ret<0) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"init monitor error, monitor_port = (%d), ret = (%d)", \
				monitor_port_, \
				ret);
		return TCP_ERR;
	}

	start_flag_=true;

	return TCP_OK;
}

int32_t QTcpServer::load_server_config(const char* cfg_file)
{
	QConfigReader config;
	int32_t ret=0;

	if(config.init(cfg_file)<0)
		return TCP_ERR;

	ret=config.getFieldString("pidfile", pidfile_);
	if(ret<0)
		return TCP_ERR;

	ret=config.getFieldString("server-name", server_name_);
	if(ret<0)
		return TCP_ERR;

	ret=config.getFieldUint16("server-port", server_port_);
	if(ret<0)
		return TCP_ERR;

	ret=config.getFieldUint16("monitor-port", monitor_port_);
	if(ret<0)
		return TCP_ERR;

	ret=config.getFieldInt32("server-timeout", server_timeout_);
	if(ret<0)
		return TCP_ERR;

	ret=config.getFieldInt32("comm-thread-max", comm_thread_max_);
	if(ret<0)
		return TCP_ERR;

	ret=config.getFieldInt32("comm-buffer-size", comm_buffer_size_);
	if(ret<0)
		return TCP_ERR;

	ret=config.getFieldInt32("comm-thread-timeout", comm_thread_timeout_);
	if(ret<0)
		return TCP_ERR;

	ret=config.getFieldInt32("work-thread-max", work_thread_max_);
	if(ret<0)
		return TCP_ERR;

	ret=config.getFieldInt32("work-buffer-size", work_buffer_size_);
	if(ret<0)
		return TCP_ERR;

	ret=config.getFieldInt32("work-thread-timeout", work_thread_timeout_);
	if(ret<0)
		return TCP_ERR;

	ret=config.getFieldInt32("send-thread-max", send_thread_max_);
	if(ret<0)
		return TCP_ERR;

	ret=config.getFieldInt32("send-buffer-size", send_buffer_size_);
	if(ret<0)
		return TCP_ERR;

	ret=config.getFieldInt32("send-thread-timeout", send_thread_timeout_);
	if(ret<0)
		return TCP_ERR;

	ret=config.getFieldInt32("queue-size", queue_size_);
	if(ret<0)
		return TCP_ERR;

	ret=config.getFieldString("send-ip", send_ip_);
	if(ret<0)
		return TCP_ERR;

	ret=config.getFieldInt32("send-port", send_port_);
	if(ret<0)
		return TCP_ERR;

	ret=config.getFieldString("data-path", data_path_);
	if(ret<0)
		return TCP_ERR;

	ret=config.getFieldString("img-path", img_path_);
	if(ret<0)
		return TCP_ERR;

	ret=config.getFieldString("read-path", read_path_);
	if(ret<0)
		return TCP_ERR;

	ret=config.getFieldString("write-path", write_path_);
	if(ret<0)
		return TCP_ERR;

	ret=config.getFieldString("log-path", log_path_);
	if(ret<0)
		return TCP_ERR;

	ret=config.getFieldString("log-prefix", log_prefix_);
	if(ret<0)
		return TCP_ERR;

	ret=config.getFieldYesNo("log-screen", log_screen_);
	if(ret<0)
		return TCP_ERR;

	ret=config.getFieldInt32("log-size", log_size_);
	if(ret<0)
		return TCP_ERR;

#if 1
	Q_INFO("pidfile              = (%s)", pidfile_);
	Q_INFO("server-name          = (%s)", server_name_);
	Q_INFO("server-port          = (%d)", server_port_);
	Q_INFO("monitor-port         = (%d)", monitor_port_);
	Q_INFO("server-timeout       = (%d)", server_timeout_);

	Q_INFO("comm-thread-max      = (%d)", comm_thread_max_);
	Q_INFO("comm-buffer-size     = (%d)", comm_buffer_size_);
	Q_INFO("comm-thread-timeout  = (%d)", comm_thread_timeout_);

	Q_INFO("work-thread-max      = (%d)", work_thread_max_);
	Q_INFO("work-buffer-size     = (%d)", work_buffer_size_);
	Q_INFO("work-thread-timeout  = (%d)", work_thread_timeout_);

	Q_INFO("send-thread-max      = (%d)", send_thread_max_);
	Q_INFO("send-buffer-size     = (%d)", send_buffer_size_);
	Q_INFO("send-thread-timeout  = (%d)", send_thread_timeout_);

	Q_INFO("queue-size           = (%d)", queue_size_);

	Q_INFO("send-ip              = (%s)", send_ip_);
	Q_INFO("send-port            = (%d)", send_port_);

	Q_INFO("img-path             = (%s)", img_path_);
	Q_INFO("read-path            = (%s)", read_path_);
	Q_INFO("write-path           = (%s)", write_path_);

	Q_INFO("log-path             = (%s)", log_path_);
	Q_INFO("log-prefix           = (%s)", log_prefix_);
	Q_INFO("log-screen           = (%d)", log_screen_);
	Q_INFO("log-size             = (%d)", log_size_);
#endif

	return TCP_OK;
}

int32_t QTcpServer::start()
{
	logger_->log(LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
			"Input q or quit to stop the server!");

	char line[1024]={0};
	while(fgets(line, sizeof(line), stdin))
	{
		if(line[0]=='q' || strcmp(line, "quit")==0)
			break;
		logger_->log(LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"Input q or quit to stop the server!");
	}

	logger_->log(LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
			"Byebye!");
	return TCP_OK;
}

Q_THREAD_T QTcpServer::comm_thread(void* ptr_info)
{
	threadInfo* ptr_trd=reinterpret_cast<threadInfo*>(ptr_info);
	Q_CHECK_PTR(ptr_trd);

	QTcpServer* ptr_this=reinterpret_cast<QTcpServer*>(ptr_trd->pthis);
	Q_CHECK_PTR(ptr_this);

	clientInfo* client_info=NULL;

	ptr_trd->flag=1;

	while(!ptr_this->exit_flag_)
	{
		ptr_trd->status=0;

		client_info=ptr_this->chunk_queue_->pop();

		try {
			if(q_accept_socket(ptr_this->listen_sock_, \
						client_info->client_sock, \
						client_info->client_ip, \
						client_info->client_port)) {
				ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
						"TCP socket accept error!");
				throw TCP_ERR_SOCKET_ACCEPT;
			}

			ptr_trd->status=1;
			ptr_trd->sw.start();

			ptr_this->logger_->log(LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
					"---------- Request from [%s:%d] ----------", \
					client_info->client_ip, \
					client_info->client_port);

			if(q_set_overtime(client_info->client_sock, \
						ptr_this->server_timeout_)) {
				ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
						"TCP socket set timeout (%d) error!", \
						ptr_this->server_timeout_);
				throw TCP_ERR_SOCKET_TIMEOUT;
			}

			ptr_this->client_queue_->push(client_info);
			ptr_this->client_trigger_->signal();
		} catch(const int32_t err) {
			ptr_this->chunk_queue_->push(client_info);
			ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
					"Task process socket error, err = (%d)!",
					err);
		}

		ptr_trd->sw.stop();
	}

	ptr_trd->flag=-1;
	return NULL;
}

Q_THREAD_T QTcpServer::work_thread(void* ptr_info)
{
	threadInfo* ptr_trd=reinterpret_cast<threadInfo*>(ptr_info);
	Q_CHECK_PTR(ptr_trd);

	QTcpServer* ptr_this=reinterpret_cast<QTcpServer*>(ptr_trd->pthis);
	Q_CHECK_PTR(ptr_this);

	clientInfo* client_info=NULL;
	QStopwatch sw_work;
	int32_t recv_len=0;
	int32_t send_len=0;

	ptr_trd->flag=1;

	while(!ptr_this->exit_flag_)
	{
		ptr_trd->status=0;

		ptr_this->client_trigger_->wait();

		while(ptr_this->client_queue_->pop_non_blocking(client_info)==0)
		{
			ptr_trd->status=1;
			ptr_trd->sw.start();

			sw_work.start();

			try {
				if(q_recvbuf(client_info->client_sock, client_info->request_buffer, ptr_this->header_size_)) {
					ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
							"TCP socket recv header error, size = (%d)!", \
							ptr_this->header_size_);
					throw TCP_ERR_SOCKET_RECV;
				}

				// fun header
				recv_len=ptr_this->server_fun_header(client_info->request_buffer, ptr_this->header_size_);
				if(recv_len<0) {
					ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
							"TCP socket server_fun_header error, code = (%d)!", \
							recv_len);
					throw TCP_ERR_PACKET_HEADER;
				}

				if(recv_len>0 && recv_len>client_info->request_buffer_size) {
					ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
							"TCP socket recv_len (%d) > request_buffer_size (%d)!", \
							recv_len, \
							client_info->request_buffer_size);
					throw TCP_ERR_PACKET_LENGTH;
				}

				if(q_recvbuf(client_info->client_sock, client_info->request_buffer, recv_len)) {
					ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
							"TCP socket recv content error, size = (%d)!", \
							recv_len);
					throw TCP_ERR_SOCKET_RECV;
				}

				// fun process
				send_len=ptr_this->server_fun_process(client_info->request_buffer, \
						recv_len, \
						client_info->reply_buffer+sizeof(baseHeader), \
						client_info->reply_buffer_size-sizeof(baseHeader), \
						ptr_trd->for_worker);
				if(send_len<0) {
					ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
							"TCP Socket server_fun_process error, code = (%d)!", \
							send_len);
					throw send_len;
				} else {
					baseHeader* base_header=reinterpret_cast<baseHeader*>(client_info->reply_buffer);
					base_header->version=TCP_HEADER_VERSION;
					base_header->length=send_len;
				}

				if(send_len>0 && q_sendbuf(client_info->client_sock, client_info->reply_buffer, send_len+sizeof(baseHeader))) {
					ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
							"TCP socket send error, size = (%d)", \
							send_len);
					throw TCP_ERR_SOCKET_SEND;
				}

				q_close_socket(client_info->client_sock);
			} catch(const int32_t errcode) {
				replyHeader reply_header;
				reply_header.version=TCP_HEADER_VERSION;
				reply_header.length=sizeof(replyHeader)-sizeof(uint64_t)-sizeof(int32_t);
				reply_header.command_type=TCP_DEFAULT_COMMAND_TYPE;
				reply_header.status=errcode;

				q_sendbuf(client_info->client_sock, (char*)(&reply_header), sizeof(replyHeader));
				q_close_socket(client_info->client_sock);

				q_add_and_fetch(&ptr_this->stat_failedconnections_);
				ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
						"Working thread [%s:%d] process error, code = (%d)!", \
						client_info->client_ip, \
						client_info->client_port, \
						errcode); 
			}

			q_add_and_fetch(&ptr_this->stat_numconnections_);
			ptr_this->chunk_queue_->push(client_info);

			sw_work.stop();
			ptr_this->logger_->log(LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
					"Task (%u) process finshed, which consumed: %dms!", \
					ptr_this->stat_numconnections_, \
					sw_work.elapsed_ms());
		}
		ptr_trd->sw.stop();
	}

	ptr_trd->flag=-1;
	return NULL;
}

Q_THREAD_T QTcpServer::send_thread(void* ptr_info)
{
	threadInfo* ptr_trd=reinterpret_cast<threadInfo*>(ptr_info);
	Q_CHECK_PTR(ptr_trd);

	QTcpServer* ptr_this=reinterpret_cast<QTcpServer*>(ptr_trd->pthis);
	Q_CHECK_PTR(ptr_this);

	replyHeader reply_header;
	baseHeader base_header;

	Q_SOCKET_T sock_client=TCP_DEFAULT_INVALID_SOCKET;
	uint64_t end_mark=0;

	uint64_t file_size=0;
	uint64_t current_size=0;

	int32_t ret=0;

	ptr_trd->flag=1;

	while(!ptr_this->exit_flag_)
	{
		ptr_trd->status=0;

		if(!QFile::exists(ptr_this->read_path_))
		{
			if(!QFile::exists(ptr_this->write_path_)) {
				q_sleep(1000);
				continue;
			}

			ptr_this->file_mutex_.lock();
			if(ptr_this->write_fp_) {
				::fclose(ptr_this->write_fp_);
				ptr_this->write_fp_=NULL;
			}

			QFile::rename(ptr_this->write_path_, ptr_this->read_path_);
			ptr_this->file_mutex_.unlock();
		}

		FILE *fp_r=NULL;
		do {
			fp_r=::fopen(ptr_this->read_path_, "rb");
			if(fp_r)
				break;
			q_sleep(1000);
		} while(fp_r==NULL);

		ptr_trd->status=1;

		file_size=QFile::size(ptr_this->read_path_);
		current_size=0;

		try {
			while(current_size<file_size) {
				ptr_trd->sw.start();

				if(fread(&base_header, sizeof(baseHeader), 1, fp_r)!=1)
					throw -1;

				if(base_header.version!=TCP_HEADER_VERSION)
					throw -2;

				if(base_header.length>ptr_trd->buf_size)
					throw -3;

				if(fread(ptr_trd->ptr_buf, base_header.length, 1, fp_r)!=1)
					throw -4;

				if(fread(&end_mark, sizeof(end_mark), 1, fp_r)!=1)
					throw -5;

				if(end_mark!=TCP_TAILER_FILE_MARK)
					throw -6;

				current_size+=sizeof(baseHeader)+base_header.length+sizeof(end_mark);

				ptr_trd->sw.stop();

				for(;;) {
					ptr_trd->sw.start();

					ret=q_connect_socket(sock_client, ptr_this->send_ip_, ptr_this->send_port_);
					if(ret!=0) {
						Q_INFO("TCP socket [%s:%d] connection......", \
								ptr_this->send_ip_, \
								ptr_this->send_port_);
						q_sleep(1000);
						continue;
					}

					ret=q_set_overtime(sock_client, ptr_this->send_thread_timeout_);
					if(ret!=0) {
						Q_INFO("TCP socket [%s:%d] set overtime......", \
								ptr_this->send_ip_, \
								ptr_this->send_port_);
						q_close_socket(sock_client);
						q_sleep(1000);
						continue;
					}

					ret=q_sendbuf(sock_client, ptr_trd->ptr_buf, base_header.length);
					if(ret!=0) {
						Q_INFO("TCP socket [%s:%d] send......", \
								ptr_this->send_ip_, \
								ptr_this->send_port_);
						q_close_socket(sock_client);
						q_sleep(1000);
						continue;
					}

					ret=q_recvbuf(sock_client, (char*)(&reply_header), sizeof(replyHeader));
					if(ret!=0) {
						Q_INFO("TCP socket [%s:%d] recv......", \
								ptr_this->send_ip_, \
								ptr_this->send_port_);
						q_close_socket(sock_client);
						q_sleep(1000);
						continue;
					}

					if(reply_header.version!=TCP_HEADER_VERSION) {
						Q_INFO("TCP socket [%s:%d] version = (%lu)", \
								ptr_this->send_ip_, \
								ptr_this->send_port_, \
								reply_header.version);
						q_close_socket(sock_client);
						q_sleep(1000);
						continue;
					}

					if(reply_header.length<=0) {
						Q_INFO("TCP socket [%s:%d] length = (%d)", \
								ptr_this->send_ip_, \
								ptr_this->send_port_, \
								reply_header.length);
						q_close_socket(sock_client);
						q_sleep(1000);
						continue;
					}

					if(reply_header.status!=0) {
						Q_INFO("TCP socket [%s:%d] status code = (%d)", \
								ptr_this->send_ip_, \
								ptr_this->send_port_, \
								reply_header.status);
						q_close_socket(sock_client);
						q_sleep(1000);
						continue;
					}

					q_close_socket(sock_client);

					ptr_trd->sw.stop();
					break;
				}
			}

			fclose(fp_r);
			fp_r=NULL;

			ptr_this->backup_file(ptr_this->read_path_, ptr_trd->ptr_buf, ptr_trd->buf_size);
			QFile::remove(ptr_this->read_path_);
		} catch(const int32_t err) {
			fclose(fp_r);
			fp_r=NULL;

			char str_file[1<<5]={0};
			q_snprintf(str_file, sizeof(str_file), "%s.%ld", ptr_this->read_path_, time(NULL));
			QFile::rename(ptr_this->read_path_, str_file);

			ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
					"send file (%s) error, err = (%d)", \
					ptr_this->read_path_, err);
		}
	}

	ptr_trd->flag=-1;
	return NULL;
}

int32_t QTcpServer::write_data_file(const char* ptr_file, FILE*& fp_w, const char* ptr_buf, int32_t buf_len)
{
	int32_t ret=0;
	int64_t offset=0;

	baseHeader base_header;
	base_header.version=TCP_HEADER_VERSION;
	base_header.length=buf_len;

	file_mutex_.lock();

	while(fp_w==NULL) {
		fp_w=::fopen(ptr_file, "wb");
		if(NULL!=fp_w)
			break;
		q_sleep(1000);
	}

	offset=::ftell(fp_w);

	try {
		if(fwrite(&base_header, sizeof(baseHeader), 1, fp_w)!=1)
			throw -2;

		if(fwrite(ptr_buf, buf_len, 1, fp_w)!=1)
			throw -3;

		if(fwrite(&TCP_TAILER_FILE_MARK, sizeof(TCP_TAILER_FILE_MARK), 1, fp_w)!=1)
			throw -4;

		while(::fflush(fp_w)!=0) {
			Q_INFO("fflush error!");
			q_sleep(1000);
		}
	} catch(const int32_t err) {
		ret=err;

		while(::fseek(fp_w, offset, SEEK_SET)!=0) {
			Q_INFO("write_data_file fseek (%ld) error!", offset);
			q_sleep(1000);
		}

		while(q_change_file_size(fp_w, offset)!=0) {
			Q_INFO("write_data_file q_change_file_size (%ld) error!", offset);
			q_sleep(1000);
		}
	}

	file_mutex_.unlock();

	return ret;
}

int32_t QTcpServer::backup_file(const char* ptr_file, char* ptr_buf, int32_t buf_size)
{
	if(ptr_file==NULL||ptr_buf==NULL||buf_size<=0)
		return -1;

	time_t now=time(NULL);
	struct tm* ptm=localtime(&now);

	char bak_name[1<<7]={0};
	q_snprintf(bak_name, sizeof(bak_name)-1, "%s/%04d-%02d-%02d.snd", data_path_, ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday);

	FILE* fp_r=::fopen(ptr_file, "rb");
	if(fp_r==NULL)
		return -2;

	FILE* fp_w=::fopen(bak_name, "ab");
	if(fp_w==NULL) {
		fclose(fp_r);
		fp_r=NULL;
		return -3;
	}

	baseHeader base_header;
	uint64_t end_mark=0;

	uint64_t file_size=QFile::size(ptr_file);
	uint64_t current_size=0;

	int32_t ret=0;
	try {
		while(current_size<file_size) {
			if(::fread(&base_header, sizeof(baseHeader), 1, fp_r)!=1)
				throw -4;

			if(base_header.length>buf_size)
				throw -5;

			if(::fread(ptr_buf, base_header.length, 1, fp_r)!=1)
				throw -6;

			if(::fread(&end_mark, sizeof(end_mark), 1, fp_r)!=1)
				throw -7;

			if(end_mark!=TCP_TAILER_FILE_MARK)
				throw -8;

			if(::fwrite(&base_header, sizeof(baseHeader), 1, fp_w)!=1)
				throw -9;

			if(::fwrite(ptr_buf, base_header.length, 1, fp_w)!=1)
				throw -10;

			if(::fwrite(&end_mark, sizeof(end_mark), 1, fp_w)!=1)
				throw -11;

			current_size+=sizeof(baseHeader)+base_header.length+sizeof(end_mark);
		}
	} catch(int32_t err) {
		ret=err;
	}

	::fclose(fp_w);
	fp_w=NULL;

	::fclose(fp_r);
	fp_r=NULL;

	return ret;
}

void QTcpServer::free_thread_info()
{
	for(int32_t i=0; i<thread_max_; ++i)
	{
		client_trigger_->signal();

		q_sleep(1);
		while(thread_info_[i].flag!=-1)
			q_sleep(1);

		q_delete_array<char>(thread_info_[i].ptr_buf);

		if(thread_info_[i].for_worker!=NULL)
			server_fun_free(thread_info_[i].for_worker);
	}

	q_delete<threadInfo>(thread_info_);
	thread_info_=NULL;
}

int32_t QTcpServer::get_thread_state(void* ptr_info)
{
	QTcpServer* ptr_this=reinterpret_cast<QTcpServer*>(ptr_info);
	Q_CHECK_PTR(ptr_this);

	int32_t timeout_num=0;
	for(int32_t i=0; i<ptr_this->thread_max_; ++i)
	{
		if(ptr_this->thread_info_[i].status!=1)
			continue;

		ptr_this->thread_info_[i].sw.stop();

		if(ptr_this->thread_info_[i].sw.elapsed_ms()>ptr_this->thread_info_[i].timeout)
			++timeout_num;
	}

	return timeout_num;
}

Q_END_NAMESPACE
