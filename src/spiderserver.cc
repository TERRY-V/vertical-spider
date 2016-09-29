#include "spiderserver.h"

Q_BEGIN_NAMESPACE

SpiderServer::SpiderServer()
{
	this->pid_=getpid();
	this->pidfile_=q_strdup(SPIDER_DEFAULT_PIDFILE);
	this->unixtime_=time(NULL);
	this->hz_=SPIDER_DEFAULT_HZ;
	this->arch_bits_=(sizeof(long)==8)?64:32;
	this->start_flag_=false;
	this->exit_flag_=false;
	this->server_name_=NULL;
	this->server_port_=SPIDER_DEFAULT_SERVER_PORT;
	this->server_timeout_=SPIDER_DEFAULT_SERVER_TIMEOUT;
	this->thread_info_=NULL;
	this->thread_max_=0;
	this->comm_thread_max_=SPIDER_DEFAULT_THREAD_NUM;
	this->comm_buffer_size_=SPIDER_DEFAULT_BUFFER_SIZE;
	this->comm_thread_timeout_=SPIDER_DEFAULT_THREAD_TIMEOUT;
	this->work_thread_max_=SPIDER_DEFAULT_THREAD_NUM;
	this->work_buffer_size_=SPIDER_DEFAULT_BUFFER_SIZE;
	this->work_thread_timeout_=SPIDER_DEFAULT_THREAD_TIMEOUT;
	this->proc_thread_max_=SPIDER_DEFAULT_THREAD_NUM;
	this->proc_buffer_size_=SPIDER_DEFAULT_BUFFER_SIZE;
	this->proc_thread_timeout_=SPIDER_DEFAULT_THREAD_TIMEOUT;
	this->pool_allocator_=NULL;
	this->chunk_size_=SPIDER_DEFAULT_CHUNK_SIZE;
	this->listen_sock_=SPIDER_DEFAULT_INVALID_SOCKET;
	this->task_queue_=NULL;
	this->task_trigger_=NULL;
	this->queue_size_=SPIDER_DEFAULT_QUEUE_SIZE;
	this->task_path_=NULL;
	this->task_read_path_=NULL;
	this->task_write_path_=NULL;
	this->task_write_fp_=NULL;
	this->data_path_=NULL;
	this->data_read_path_=NULL;
	this->data_write_path_=NULL;
	this->data_write_fp_=NULL;
	this->send_ip_=NULL;
	this->send_port_=SPIDER_DEFAULT_SEND_PORT;
	this->monitor_=NULL;
	this->monitor_port_=SPIDER_DEFAULT_MONITOR_PORT;
	this->logger_=NULL;
	this->log_path_=q_strdup(SPIDER_DEFAULT_LOG_PATH);
	this->log_prefix_=q_strdup(SPIDER_DEFAULT_LOG_PREFIX);
	this->log_size_=SPIDER_DEFAULT_LOG_SIZE;
	this->log_screen_=SPIDER_DEFAULT_LOG_SCREEN;
	this->stat_starttime_=time(NULL);
	this->stat_lastinteraction_=0;
	this->stat_numconnections_=0;
	this->stat_succconnections_=0;
	this->stat_failedconnections_=0;
}

SpiderServer::~SpiderServer()
{
	exit_flag_=true;

	q_free(pidfile_);
	q_free(log_path_);
	q_free(log_prefix_);

	q_close_socket(listen_sock_);

	free_thread_info();

	q_free(task_path_);
	q_free(task_read_path_);
	q_free(task_write_path_);

	q_free(data_path_);
	q_free(data_read_path_);
	q_free(data_write_path_);

	fclose(task_write_fp_);
	task_write_fp_=NULL;

	fclose(data_write_fp_);
	data_write_fp_=NULL;

	q_delete< QPoolAllocator >(pool_allocator_);

	q_delete< QQueue<std::string> >(task_queue_);
	q_delete< QTrigger >(task_trigger_);

	q_free(log_path_);
	q_free(log_prefix_);
	q_delete<QLogger>(logger_);

	q_delete<QRemoteMonitor>(monitor_);
}

int32_t SpiderServer::init(const char* cfg_file)
{
	struct timeval tv;
	setlocale(LC_COLLATE, "");
	srand(time(NULL)^getpid());
	gettimeofday(&tv, NULL);

	Q_INFO("Hello, initing the server...");
	int32_t ret=0;

	/* logger */
	logger_=q_new<QLogger>();
	if(logger_==NULL) {
		Q_FATAL("logger_ alloc error, null value!");
		return SPIDER_ERR;
	}

	ret=logger_->init(log_path_, log_prefix_, log_size_);
	if(ret<0) {
		Q_FATAL("logger_ init error, ret = (%d)!", ret);
		return SPIDER_ERR;
	}

	/* config */
	ret=load_server_config(cfg_file);
	if(ret<0) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"server configuration loading error, ret = (%d)!", \
				ret);
		return SPIDER_ERR;
	}

	/* allocator */
	pool_allocator_=q_new< QPoolAllocator >();
	if(pool_allocator_==NULL) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"pool_allocator_ is null!");
		return SPIDER_ERR;
	}

	ret=pool_allocator_->init(chunk_size_);
	if(ret<0) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"pool_allocator_ init error, ret = (%d)!", \
				ret);
		return SPIDER_ERR;
	}

	/* queue */
	task_queue_=q_new< QQueue<std::string> >();
	if(task_queue_==NULL) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"task_queue_ is null!");
		return SPIDER_ERR;
	}

	ret=task_queue_->init(queue_size_+1);
	if(ret<0) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"init task_queue_ error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	task_trigger_=q_new<QTrigger>();
	if(task_trigger_==NULL) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"task_trigger_ is null!");
		return SPIDER_ERR;
	}

	/* directory */
	if(!QDir::mkdir(task_path_)) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"mkdir task_path_ (%s) error!", \
				task_path_);
		return SPIDER_ERR;
	}

	if(access(task_write_path_, 00)==0)
	{
		ret=q_repair_file(task_write_path_, (const char*)&SPIDER_TAILER_FILE_MARK, sizeof(SPIDER_TAILER_FILE_MARK));
		if(ret<0) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"q_repair_file (%s) error, ret = (%d)!", \
					task_write_path_, \
					ret);
			return SPIDER_ERR;
		}

		task_write_fp_=::fopen(task_write_path_, "rb+");
		if(task_write_fp_==NULL) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"fopen (%s) error!", \
					task_write_path_);
			return SPIDER_ERR;
		}

		if(::fseek(task_write_fp_, 0, SEEK_END)!=0) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"fseek (%s) error!", \
					task_write_path_);
			return SPIDER_ERR;
		}
	}

	if(!QDir::mkdir(data_path_)) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"mkdir data_path_ (%s) error!", \
				data_path_);
		return SPIDER_ERR;
	}

	if(access(data_write_path_, 00)==0)
	{
		ret=q_repair_file(data_write_path_, (const char*)&SPIDER_TAILER_FILE_MARK, sizeof(SPIDER_TAILER_FILE_MARK));
		if(ret<0) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"q_repair_file (%s) error, ret = (%d)!", \
					data_write_path_, \
					ret);
			return SPIDER_ERR;
		}

		data_write_fp_=::fopen(data_write_path_, "rb+");
		if(data_write_fp_==NULL) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"fopen (%s) error!", \
					data_write_path_);
			return SPIDER_ERR;
		}

		if(::fseek(data_write_fp_, 0, SEEK_END)!=0) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"fseek (%s) error!", \
					data_write_path_);
			return SPIDER_ERR;
		}
	}

	/* init template */
	std::string text=QFile::readAll("../conf/template.xml");
	ret=processor_.init(text.c_str(), text.length());
	if(ret<0) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"init template error, ret = (%d)!", \
				ret);
		return SPIDER_ERR;
	}

	/* disk cache */
	ret=uniq_cache_.init(1<<20, 0, 60);
	if(ret<0) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"init cache error, ret = (%d)!", \
				ret);
		return SPIDER_ERR;
	}

	/* network */
	ret=q_init_socket();
	if(ret<0) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"init socket error, ret = (%d)!", \
				ret);
		return SPIDER_ERR;
	}

	ret=q_TCP_server(listen_sock_, server_port_);
	if(ret<0) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"SPIDER server error, listen_port = (%d), ret = (%d)!", \
				server_port_, \
				ret);
		return SPIDER_ERR;
	}

	/* threads */
	thread_max_=comm_thread_max_+work_thread_max_+proc_thread_max_+send_thread_max_;

	thread_info_=q_new_array<threadInfo>(thread_max_);
	if(thread_info_==NULL) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"create thread_info_ error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
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
			return SPIDER_ERR;
		}

		ret=q_create_thread(SpiderServer::comm_thread, ptr_trd+i);
		if(ret<0) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"create comm_thread (%d) error", \
					i+1);
			return SPIDER_ERR;
		}

		while(ptr_trd[i].flag!=1)
			q_sleep(1);

		if(ptr_trd[i].flag==-1) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"start comm_thread (%d) error", \
					i+1);
			return SPIDER_ERR;
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
			return SPIDER_ERR;
		}

		ret=q_create_thread(SpiderServer::work_thread, ptr_trd+i);
		if(ret<0) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"create work_thread (%d) error", \
					i+1);
			return SPIDER_ERR;
		}

		while(ptr_trd[i].flag!=1)
			q_sleep(1);

		if(ptr_trd[i].flag==-1) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"start work_thread (%d) error", \
					i+1);
			return SPIDER_ERR;
		}
	}

	ptr_trd+=work_thread_max_;
	for(int32_t i=0; i!=proc_thread_max_; ++i)
	{
		ptr_trd[i].pthis=this;
		ptr_trd[i].id=i;
		ptr_trd[i].status=0;
		ptr_trd[i].flag=0;
		ptr_trd[i].timeout=proc_thread_timeout_;
		ptr_trd[i].buf_size=proc_buffer_size_;
		ptr_trd[i].ptr_buf=q_new_array<char>(ptr_trd[i].buf_size);
		if(ptr_trd[i].ptr_buf==NULL) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"create proc_thread (%d) buffer error", \
					i+1);
			return SPIDER_ERR;
		}

		ret=q_create_thread(SpiderServer::proc_thread, ptr_trd+i);
		if(ret<0) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"create proc_thread (%d) error", \
					i+1);
			return SPIDER_ERR;
		}

		while(ptr_trd[i].flag!=1)
			q_sleep(1);

		if(ptr_trd[i].flag==-1) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"start proc_thread (%d) error", \
					i+1);
			return SPIDER_ERR;
		}
	}

	ptr_trd+=proc_thread_max_;
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
			return SPIDER_ERR;
		}

		ret=q_create_thread(SpiderServer::send_thread, ptr_trd+i);
		if(ret<0) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"create send_thread (%d) error", \
					i+1);
			return SPIDER_ERR;
		}

		while(ptr_trd[i].flag!=1)
			q_sleep(1);

		if(ptr_trd[i].flag==-1) {
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"start send_thread (%d) error", \
					i+1);
			return SPIDER_ERR;
		}
	}

	/* monitor */
	monitor_=q_new<QRemoteMonitor>();
	if(monitor_==NULL) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"monitor_ is null!");
		return SPIDER_ERR;
	}

	ret=monitor_->init(monitor_port_, 10000, get_thread_state, this, 1);
	if(ret<0) {
		logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
				"init monitor error, monitor_port = (%d), ret = (%d)", \
				monitor_port_, \
				ret);
		return SPIDER_ERR;
	}

	start_flag_=true;

	return SPIDER_OK;
}

int32_t SpiderServer::load_server_config(const char* cfg_file)
{
	QConfigReader config;
	int32_t ret=0;

	if(config.init(cfg_file)<0)
		return SPIDER_ERR;

	ret=config.getFieldString("pidfile", pidfile_);
	if(ret<0)
		return SPIDER_ERR;

	ret=config.getFieldString("server-name", server_name_);
	if(ret<0)
		return SPIDER_ERR;

	ret=config.getFieldUint16("server-port", server_port_);
	if(ret<0)
		return SPIDER_ERR;

	ret=config.getFieldUint16("monitor-port", monitor_port_);
	if(ret<0)
		return SPIDER_ERR;

	ret=config.getFieldInt32("server-timeout", server_timeout_);
	if(ret<0)
		return SPIDER_ERR;

	ret=config.getFieldInt32("comm-thread-max", comm_thread_max_);
	if(ret<0)
		return SPIDER_ERR;

	ret=config.getFieldInt32("comm-buffer-size", comm_buffer_size_);
	if(ret<0)
		return SPIDER_ERR;

	ret=config.getFieldInt32("comm-thread-timeout", comm_thread_timeout_);
	if(ret<0)
		return SPIDER_ERR;

	ret=config.getFieldInt32("work-thread-max", work_thread_max_);
	if(ret<0)
		return SPIDER_ERR;

	ret=config.getFieldInt32("work-buffer-size", work_buffer_size_);
	if(ret<0)
		return SPIDER_ERR;

	ret=config.getFieldInt32("work-thread-timeout", work_thread_timeout_);
	if(ret<0)
		return SPIDER_ERR;

	ret=config.getFieldInt32("proc-thread-max", proc_thread_max_);
	if(ret<0)
		return SPIDER_ERR;

	ret=config.getFieldInt32("proc-buffer-size", proc_buffer_size_);
	if(ret<0)
		return SPIDER_ERR;

	ret=config.getFieldInt32("proc-thread-timeout", proc_thread_timeout_);
	if(ret<0)
		return SPIDER_ERR;

	ret=config.getFieldInt32("send-thread-max", send_thread_max_);
	if(ret<0)
		return SPIDER_ERR;

	ret=config.getFieldInt32("send-buffer-size", send_buffer_size_);
	if(ret<0)
		return SPIDER_ERR;

	ret=config.getFieldInt32("send-thread-timeout", send_thread_timeout_);
	if(ret<0)
		return SPIDER_ERR;

	ret=config.getFieldInt32("queue-size", queue_size_);
	if(ret<0)
		return SPIDER_ERR;

	ret=config.getFieldString("task-path", task_path_);
	if(ret<0)
		return SPIDER_ERR;

	ret=config.getFieldString("task-read-path", task_read_path_);
	if(ret<0)
		return SPIDER_ERR;

	ret=config.getFieldString("task-write-path", task_write_path_);
	if(ret<0)
		return SPIDER_ERR;

	ret=config.getFieldString("data-path", data_path_);
	if(ret<0)
		return SPIDER_ERR;

	ret=config.getFieldString("data-read-path", data_read_path_);
	if(ret<0)
		return SPIDER_ERR;

	ret=config.getFieldString("data-write-path", data_write_path_);
	if(ret<0)
		return SPIDER_ERR;

	ret=config.getFieldString("send-ip", send_ip_);
	if(ret<0)
		return SPIDER_ERR;

	ret=config.getFieldInt32("send-port", send_port_);
	if(ret<0)
		return SPIDER_ERR;

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

	Q_INFO("proc-thread-max      = (%d)", proc_thread_max_);
	Q_INFO("proc-buffer-size     = (%d)", proc_buffer_size_);
	Q_INFO("proc-thread-timeout  = (%d)", proc_thread_timeout_);

	Q_INFO("send-thread-max      = (%d)", send_thread_max_);
	Q_INFO("send-buffer-size     = (%d)", send_buffer_size_);
	Q_INFO("send-thread-timeout  = (%d)", send_thread_timeout_);

	Q_INFO("queue-size           = (%d)", queue_size_);

	Q_INFO("task-path            = (%s)", task_path_);
	Q_INFO("task-read-path       = (%s)", task_read_path_);
	Q_INFO("task-write-path      = (%s)", task_write_path_);

	Q_INFO("data-path            = (%s)", data_path_);
	Q_INFO("data-read-path       = (%s)", data_read_path_);
	Q_INFO("data-write-path      = (%s)", data_write_path_);

	Q_INFO("send-ip              = (%s)", send_ip_);
	Q_INFO("send-port            = (%d)", send_port_);
#endif

	return SPIDER_OK;
}

int32_t SpiderServer::start()
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
	return SPIDER_OK;
}

Q_THREAD_T SpiderServer::comm_thread(void* ptr_info)
{
	threadInfo* ptr_trd=reinterpret_cast<threadInfo*>(ptr_info);
	Q_CHECK_PTR(ptr_trd);

	SpiderServer* ptr_this=reinterpret_cast<SpiderServer*>(ptr_trd->pthis);
	Q_CHECK_PTR(ptr_this);

	requestHeader* request_header=reinterpret_cast<requestHeader*>(ptr_trd->ptr_buf);
	Q_CHECK_PTR(request_header);

	Q_SOCKET_T client_sock;
	char client_ip[16];
	int32_t client_port;

	replyHeader reply_header;
	reply_header.version=SPIDER_HEADER_VERSION;
	reply_header.length=sizeof(replyHeader)-sizeof(baseHeader);
	reply_header.command_type=SPIDER_DEFAULT_COMMAND_TYPE;

	ptr_trd->flag=1;

	while(!ptr_this->exit_flag_)
	{
		ptr_trd->status=0;

		try {
			if(q_accept_socket(ptr_this->listen_sock_, \
						client_sock, \
						client_ip, \
						client_port)) {
				ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
						"SPIDER socket accept error!");
				throw SPIDER_ERR_SOCKET_ACCEPT;
			}

			ptr_trd->status=1;
			ptr_trd->sw.start();

			ptr_this->logger_->log(LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
					"---------- Request from [%s:%d] ----------", \
					client_ip, \
					client_port);

			if(q_set_overtime(client_sock, \
						ptr_this->server_timeout_)) {
				ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
						"SPIDER socket set timeout (%d) error!", \
						ptr_this->server_timeout_);
				throw SPIDER_ERR_SOCKET_TIMEOUT;
			}

			if(q_recvbuf(client_sock, \
						ptr_trd->ptr_buf, \
						sizeof(baseHeader))) {
				ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
						"SPIDER socket recv header error, size = (%d)!", \
						sizeof(baseHeader));
				throw SPIDER_ERR_SOCKET_RECV;
			}

			if(request_header->version!=SPIDER_HEADER_VERSION) {
				ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
						"SPIDER socket version error, version = (%.*s)!", \
						sizeof(uint64_t), \
						(char*)&request_header->version);
				throw SPIDER_ERR_SOCKET_VERSION;
			}

			if(request_header->length>0 \
					&& request_header->length>ptr_trd->buf_size-static_cast<int32_t>(sizeof(baseHeader))) {
				ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
						"SPIDER socket recv length (%d) > request_buffer_size (%d)!", \
						request_header->length, \
						ptr_trd->buf_size-sizeof(baseHeader));
				throw SPIDER_ERR_PACKET_LENGTH;
			}

			if(q_recvbuf(client_sock, \
						ptr_trd->ptr_buf+sizeof(baseHeader), \
						request_header->length)) {
				ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
						"SPIDER socket recv content error, size = (%d)!", \
						request_header->length);
				throw SPIDER_ERR_SOCKET_RECV;
			}

			if(request_header->protocol_type==SPIDER_DEFAULT_PROTOCOL_TYPE \
					&& request_header->source_type==SPIDER_DEFAULT_SOURCE_TYPE \
					&& request_header->command_type==SPIDER_DEFAULT_COMMAND_TYPE) {
				if(ptr_this->write_data_file(ptr_this->task_write_path_, \
						ptr_this->task_write_fp_, \
						ptr_this->task_mutex_, \
						ptr_trd->ptr_buf+sizeof(requestHeader)+sizeof(uint16_t)+sizeof(int32_t), \
						*(int32_t*)(ptr_trd->ptr_buf+sizeof(requestHeader)+sizeof(uint16_t)))) {
					ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
							"SPIDER write data file error!");
					throw SPIDER_ERR_WRITE_FILE;
				}
			} else {
				ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
						"SPIDER socket unknown operation, protocol_type = (%d), " \
						"source_type = (%d), " \
						"command_type = (%d)!", \
						request_header->protocol_type, \
						request_header->source_type, \
						request_header->command_type);
				throw SPIDER_ERR_UNKNOWN_PROTOCOL;
			}

			reply_header.status=0;

			if(q_sendbuf(client_sock, \
						(char*)&reply_header, \
						sizeof(replyHeader))) {
				ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
						"SPIDER socket send error, size = (%d)!", \
						sizeof(replyHeader));
				throw SPIDER_ERR_SOCKET_SEND;
			}

			q_close_socket(client_sock);
		} catch(const int32_t errcode) {
			reply_header.status=errcode;
			q_sendbuf(client_sock, (char*)(&reply_header), sizeof(replyHeader));
			q_close_socket(client_sock);

			q_add_and_fetch(&ptr_this->stat_failedconnections_);
		}

		q_add_and_fetch(&ptr_this->stat_numconnections_);

		ptr_trd->sw.stop();
		ptr_this->logger_->log(LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
				"Task (%u) process finshed, status = (%d), which consumed: %dms!", \
				ptr_this->stat_numconnections_, \
				reply_header.status, \
				ptr_trd->sw.elapsed_ms());
	}

	ptr_trd->flag=-1;
	return NULL;
}

Q_THREAD_T SpiderServer::work_thread(void* ptr_info)
{
	threadInfo* ptr_trd=reinterpret_cast<threadInfo*>(ptr_info);
	Q_CHECK_PTR(ptr_trd);

	SpiderServer* ptr_this=reinterpret_cast<SpiderServer*>(ptr_trd->pthis);
	Q_CHECK_PTR(ptr_this);

	baseHeader base_header;
	uint64_t end_mark=0;

	uint64_t file_size=0;
	uint64_t current_size=0;

	ptr_trd->flag=1;

	while(!ptr_this->exit_flag_)
	{
		ptr_trd->status=0;

		if(!QFile::exists(ptr_this->task_read_path_))
		{
			if(!QFile::exists(ptr_this->task_write_path_)) {
				q_sleep(1000);
				continue;
			}

			ptr_this->task_mutex_.lock();
			if(ptr_this->task_write_fp_) {
				::fclose(ptr_this->task_write_fp_);
				ptr_this->task_write_fp_=NULL;
			}

			QFile::rename(ptr_this->task_write_path_, ptr_this->task_read_path_);
			ptr_this->task_mutex_.unlock();
		}

		FILE *fp_r=NULL;
		do {
			fp_r=::fopen(ptr_this->task_read_path_, "rb");
			if(fp_r)
				break;
			q_sleep(1000);
		} while(fp_r==NULL);

		ptr_trd->status=1;

		file_size=QFile::size(ptr_this->task_read_path_);
		current_size=0;

		try {
			while(current_size<file_size) {
				ptr_trd->sw.start();

				if(fread(&base_header, sizeof(baseHeader), 1, fp_r)!=1)
					throw -1;

				if(base_header.version!=SPIDER_HEADER_VERSION)
					throw -2;

				if(base_header.length>ptr_trd->buf_size)
					throw -3;

				if(fread(ptr_trd->ptr_buf, base_header.length, 1, fp_r)!=1)
					throw -4;

				if(fread(&end_mark, sizeof(end_mark), 1, fp_r)!=1)
					throw -5;

				if(end_mark!=SPIDER_TAILER_FILE_MARK)
					throw -6;

				current_size+=sizeof(baseHeader)+base_header.length+sizeof(end_mark);

				ptr_this->task_queue_->push(std::string(ptr_trd->ptr_buf, base_header.length));
				ptr_this->task_trigger_->signal();

				ptr_trd->sw.stop();
			}

			fclose(fp_r);
			fp_r=NULL;

			ptr_this->backup_file(ptr_this->task_read_path_, ptr_this->task_path_, ptr_trd->ptr_buf, ptr_trd->buf_size);
			QFile::remove(ptr_this->task_read_path_);
		} catch(const int32_t err) {
			fclose(fp_r);
			fp_r=NULL;

			char str_file[1<<5]={0};
			q_snprintf(str_file, sizeof(str_file), "%s.%ld", ptr_this->task_read_path_, time(NULL));
			QFile::rename(ptr_this->task_read_path_, str_file);

			ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
					"read task file (%s) error, err = (%d)", \
					ptr_this->task_read_path_, \
					err);
		}
	}

	ptr_trd->flag=-1;
	return NULL;
}

Q_THREAD_T SpiderServer::proc_thread(void* ptr_info)
{
	threadInfo* ptr_trd = reinterpret_cast<threadInfo*>(ptr_info);
	Q_CHECK_PTR(ptr_trd);

	SpiderServer* ptr_this = reinterpret_cast<SpiderServer*>(ptr_trd->pthis);
	Q_CHECK_PTR(ptr_this);

	QStopwatch stopwatch;
	std::string strTask;
	int32_t ret=0;

	ptr_trd->flag = 1;

	while (!ptr_this->exit_flag_)
	{
		ptr_trd->status = 0;
		ptr_this->task_trigger_->wait();
		while (ptr_this->task_queue_->pop_non_blocking(strTask) == 0)
		{
			ptr_trd->status=1;
			ptr_trd->sw.start();

			std::vector<taskInfo> tasks;
			int32_t wflag=0;

			ret=ptr_this->parseTasksFromXML(strTask, tasks);
			if(ret<0) {
				ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
						"Parse task (%s) error, ret = (%d)", \
						strTask.c_str(), \
						ret);
				continue;
			}

			for(uint32_t i=0; i!=tasks.size(); ++i)
			{
				ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
						"---------- Url (%s) process begins now.....", \
						tasks[i].url.c_str());

				ret=ptr_this->processor_.process(tasks[i].method, \
						tasks[i].referer.c_str(), \
						tasks[i].url.c_str(), \
						tasks[i].playload.c_str(), \
						ptr_trd->ptr_buf, \
						ptr_trd->buf_size, \
						&wflag);
				if(ret<0) {
					ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
							"url (%s) process error, ret = (%d)", \
							tasks[i].url.c_str(), \
							ret);
					continue;
				}

				if(wflag) {
					if(ptr_this->write_data_file(ptr_this->data_write_path_, \
							ptr_this->data_write_fp_, \
							ptr_this->data_mutex_, \
							ptr_trd->ptr_buf, \
							ret)) {
						ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
								"url (%s) write error, ret = (%d)", \
								tasks[i].url.c_str(), \
								ret);
						continue;
					}
				}

				std::string xmlData(ptr_trd->ptr_buf, ret);
				std::string linksTask;

				ret=ptr_this->parseAndGetLinks(xmlData, linksTask);
				if(ret<0) {
					ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
							"url (%s) parseAndGetLinks error, xml = (%s), ret = (%d)", \
							tasks[i].url.c_str(), \
							xmlData.c_str(), \
							ret);
					continue;
				}

				if(linksTask.length() && ptr_this->write_data_file(ptr_this->task_write_path_, \
							ptr_this->task_write_fp_, \
							ptr_this->task_mutex_, \
							linksTask.c_str(), \
							linksTask.length())) {
					ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
							"SPIDER write data file error!");
					continue;
				}

				ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
						"Url (%s) process successful!", \
						tasks[i].url.c_str());
			}

			ptr_trd->sw.stop();
		}
	}

	ptr_trd->flag = -1;
	return NULL;
}

int32_t SpiderServer::parseTasksFromXML(const std::string& xml, std::vector<taskInfo>& tasks)
{
	XMLDocument doc;
	doc.Parse(xml.c_str(), xml.length());
	if(doc.Error()) {
		Q_INFO("Parse XML error (%s)!", doc.GetErrorStr1());
		return -1;
	}

	XMLElement *rootElement=doc.RootElement();
	if(rootElement==NULL)
		return -2;

	XMLElement *taskElement=rootElement->FirstChildElement("task");
	while(taskElement!=NULL)
	{
		try {
			taskInfo task_info;

			char* method=const_cast<char*>(taskElement->Attribute("method"));
			if(method==NULL) {
				task_info.method=1;
			} else if(::strlen(method)==3 && ::strncmp(method, "GET", 3)==0) {
				task_info.method=1;
			} else if(::strlen(method)==4 && ::strncmp(method, "POST", 4)==0) {
				task_info.method=2;
			} else {
				throw -1;
			}

			XMLElement* refererElement=taskElement->FirstChildElement("referer");
			if(refererElement&&refererElement->GetText())
				task_info.referer.assign(refererElement->GetText());

			XMLElement* urlElement=taskElement->FirstChildElement("url");
			if(urlElement&&urlElement->GetText())
				task_info.url.assign(urlElement->GetText());
			else
				throw -2;

			if(task_info.method==2)
			{
				XMLElement* playloadElement=taskElement->FirstChildElement("playload");
				if(playloadElement&&playloadElement->GetText())
					task_info.playload.assign(playloadElement->GetText());
				else
					throw -3;
			}

			XMLElement* paramElement=taskElement->FirstChildElement("param");
			if(paramElement&&paramElement->GetText())
			{
				int32_t start=0, finish=0, step=0;
				sscanf(paramElement->GetText(), "%d:%d:%d", &start, &finish, &step);

				for(int32_t now=start; now<=finish; now+=step) {
					task_info.url=q_format(urlElement->GetText(), now);
					tasks.push_back(task_info);
				}
			} else {
				tasks.push_back(task_info);
			}

			taskElement=taskElement->NextSiblingElement();
		} catch(const int32_t code) {
			taskElement=taskElement->NextSiblingElement();
			return -3;
		}
	}

	return 0;
}

int32_t SpiderServer::parseAndGetLinks(const std::string& xml, std::string& strTask)
{
	/* 获取链接 */
	std::vector<taskInfo> links;
	XMLDocument doc;

	doc.Parse(xml.c_str(), xml.length());
	if(doc.Error()) {
		Q_INFO("Parse XML error (%s)!", doc.GetErrorStr1());
		return -1;
	}

	XMLElement *rootElement=doc.RootElement();
	if(rootElement==NULL)
		return -2;

	XMLElement *basicElement=rootElement->FirstChildElement("basic");
	if(basicElement==NULL)
		return -3;

	XMLElement *srclinkElement=basicElement->FirstChildElement("srclink");
	if(srclinkElement==NULL||srclinkElement->GetText()==NULL)
		return -4;

	XMLElement *linksElement=rootElement->FirstChildElement("links");
	if(linksElement==NULL)
		return -5;

	XMLElement *linkElement=linksElement->FirstChildElement("link");
	while(linkElement!=NULL)
	{
		XMLElement* urlElement=linkElement->FirstChildElement("url");
		if(urlElement==NULL||urlElement->GetText()==NULL) {
			linkElement=linkElement->NextSiblingElement();
			continue;
		}

		taskInfo task_info;
		task_info.method=1;
		task_info.referer.assign(srclinkElement->GetText());
		task_info.url.assign(q_repair_url(std::string(urlElement->GetText()), q_get_host(task_info.referer)));

		uint64_t urlKey=make_md5(task_info.url.c_str(), task_info.url.length());
		if(uniq_cache_.searchKey_FL(urlKey)) {
			uniq_cache_.addKey_FL(urlKey);
			links.push_back(task_info);
			logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_screen_, \
					"Found new url(%s), adds to queue!", \
					task_info.url.c_str());
		}

		linkElement=linkElement->NextSiblingElement();
	}

	/* 生成任务 */
	if(links.size())
	{
		strTask.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n");
		strTask.append("<tasks>\r\n");

		for(uint32_t i=0; i<links.size(); ++i)
		{
			strTask.append(q_format("<task method=\"%s\">\r\n", links[i].method==1?"GET":"POST"));
			strTask.append(q_format("<referer><![CDATA[%s]]></referer>\r\n", links[i].referer.c_str()));
			strTask.append(q_format("<url><![CDATA[%s]]></url>\r\n", links[i].url.c_str()));
			strTask.append("</task>\r\n");
		}

		strTask.append("</tasks>\r\n");
	}

	return 0;
}

Q_THREAD_T SpiderServer::send_thread(void* ptr_info)
{
	threadInfo* ptr_trd=reinterpret_cast<threadInfo*>(ptr_info);
	Q_CHECK_PTR(ptr_trd);

	SpiderServer* ptr_this=reinterpret_cast<SpiderServer*>(ptr_trd->pthis);
	Q_CHECK_PTR(ptr_this);

	baseHeader base_header;
	uint64_t end_mark=0;

	uint64_t file_size=0;
	uint64_t current_size=0;

	int32_t ret=0;

	ptr_trd->flag=1;

	while(!ptr_this->exit_flag_)
	{
		ptr_trd->status=0;

		if(!QFile::exists(ptr_this->data_read_path_))
		{
			if(!QFile::exists(ptr_this->data_write_path_)) {
				q_sleep(1000);
				continue;
			}

			ptr_this->data_mutex_.lock();
			if(ptr_this->data_write_fp_) {
				::fclose(ptr_this->data_write_fp_);
				ptr_this->data_write_fp_=NULL;
			}

			QFile::rename(ptr_this->data_write_path_, ptr_this->data_read_path_);
			ptr_this->data_mutex_.unlock();
		}

		FILE *fp_r=NULL;
		do {
			fp_r=::fopen(ptr_this->data_read_path_, "rb");
			if(fp_r)
				break;
			q_sleep(1000);
		} while(fp_r==NULL);

		ptr_trd->status=1;

		file_size=QFile::size(ptr_this->data_read_path_);
		current_size=0;

		try {
			while(current_size<file_size) {
				ptr_trd->sw.start();

				if(fread(&base_header, sizeof(baseHeader), 1, fp_r)!=1)
					throw -1;

				if(base_header.version!=SPIDER_HEADER_VERSION)
					throw -2;

				if(base_header.length>ptr_trd->buf_size)
					throw -3;

				if(fread(ptr_trd->ptr_buf, base_header.length, 1, fp_r)!=1)
					throw -4;

				if(fread(&end_mark, sizeof(end_mark), 1, fp_r)!=1)
					throw -5;

				if(end_mark!=SPIDER_TAILER_FILE_MARK)
					throw -6;

				current_size+=sizeof(baseHeader)+base_header.length+sizeof(end_mark);
				ptr_trd->sw.stop();

				for(;;)
				{
					ptr_trd->sw.start();

					QTcpClient client;
					networkReply reply;

					client.setHost(ptr_this->send_ip_, ptr_this->send_port_);
					client.setTimeout(10000);
					client.setProtocolType(1);
					client.setSourceType(1);
					client.setCommandType(1);
					client.setOperateType(1);

					ret=client.sendRequest(ptr_trd->ptr_buf, base_header.length);
					if(ret<0) {
						Q_INFO("Client [%s:%d] sends request error, errno = (%d)...", \
								ptr_this->send_ip_, \
								ptr_this->send_port_, \
								ret);
						q_sleep(3000);
						continue;
					}

					ret=client.getReply(&reply);
					if(ret<0) {
						Q_INFO("Client [%s:%d] get reply error, errno = (%d)...", \
								ptr_this->send_ip_, \
								ptr_this->send_port_, \
								ret);
						q_sleep(3000);
						continue;
					}

					if(reply.status!=0) {
						Q_INFO("Client [%s:%d] get reply status = (%d)...", \
								ptr_this->send_ip_, \
								ptr_this->send_port_, \
								reply.status);
						q_sleep(3000);
						continue;
					}

					ptr_trd->sw.stop();
					break;
				}
			}

			fclose(fp_r);
			fp_r=NULL;

			ptr_this->backup_file(ptr_this->data_read_path_, ptr_this->data_path_, ptr_trd->ptr_buf, ptr_trd->buf_size);
			QFile::remove(ptr_this->data_read_path_);
		} catch(const int32_t err) {
			fclose(fp_r);
			fp_r=NULL;

			char str_file[1<<5]={0};
			q_snprintf(str_file, sizeof(str_file), "%s.%ld", ptr_this->data_read_path_, time(NULL));
			QFile::rename(ptr_this->data_read_path_, str_file);

			ptr_this->logger_->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, ptr_this->log_screen_, \
					"read data file (%s) error, err = (%d)", \
					ptr_this->data_read_path_, \
					err);
		}
	}

	ptr_trd->flag=-1;
	return NULL;
}

int32_t SpiderServer::write_data_file(const char* ptr_file, FILE*& fp_w, QMutexLock& file_mutex, const char* ptr_buf, int32_t buf_len)
{
	int32_t ret=0;
	int64_t offset=0;

	baseHeader base_header;
	base_header.version=SPIDER_HEADER_VERSION;
	base_header.length=buf_len;

	file_mutex.lock();

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

		if(fwrite(&SPIDER_TAILER_FILE_MARK, sizeof(SPIDER_TAILER_FILE_MARK), 1, fp_w)!=1)
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

	file_mutex.unlock();

	return ret;
}

int32_t SpiderServer::backup_file(const char* ptr_file, const char* ptr_path, char* ptr_buf, int32_t buf_size)
{
	if(ptr_file==NULL||ptr_buf==NULL||buf_size<=0)
		return -1;

	time_t now=time(NULL);
	struct tm* ptm=::localtime(&now);

	char bak_name[1<<7]={0};
	q_snprintf(bak_name, sizeof(bak_name)-1, "%s/%04d-%02d-%02d.snd", ptr_path, ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday);

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

			if(end_mark!=SPIDER_TAILER_FILE_MARK)
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

void SpiderServer::free_thread_info()
{
	for(int32_t i=0; i<thread_max_; ++i)
	{
		q_sleep(1);
		while(thread_info_[i].flag!=-1)
			q_sleep(1);

		q_delete_array<char>(thread_info_[i].ptr_buf);
	}

	q_delete<threadInfo>(thread_info_);
	thread_info_=NULL;
}

int32_t SpiderServer::get_thread_state(void* ptr_info)
{
	SpiderServer* ptr_this=reinterpret_cast<SpiderServer*>(ptr_info);
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
