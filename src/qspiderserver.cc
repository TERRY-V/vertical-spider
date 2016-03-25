#include "qspiderserver.h"

QSpiderServer::QSpiderServer()
{
	this->m_pid=getpid();
	this->m_pidfile=q_strdup(SPIDER_DEFAULT_PIDFILE);
	this->m_unixtime=time(NULL);
	this->m_hz=SPIDER_DEFAULT_HZ;
	this->m_arch_bits=(sizeof(long)==8)?64:32;
	this->m_start_flag=false;
	this->m_exit_flag=false;
	this->m_ptr_cfg_info=NULL;
	this->m_ptr_trd_info=NULL;
	this->m_thread_max=0;
	this->m_ptr_pool_allocator=NULL;
	this->m_sock_svr=SPIDER_INVALID_SOCKET;
	this->m_ptr_client_queue=NULL;
	this->m_ptr_client_trigger=NULL;
	this->m_ptr_entry_trigger=NULL;
	this->m_ptr_dp_queue=NULL;
	this->m_ptr_dp_trigger=NULL;
	this->m_ptr_picker=NULL;
	this->m_ptr_tpl_manager=NULL;
	this->I_SEND_FILE_MARK=PROTOCOL_FILE_END_MARK;
	this->m_send_write_fp=NULL;
	this->m_ptr_uniq_cache=NULL;
	this->m_cache_bucket_size=SPIDER_CACHE_BUCKET_SIZE;
	this->m_cache_interval=SPIDER_CACHE_INTERVAL;
	this->m_ptr_logger=NULL;
	this->m_ptr_log_path=q_strdup(SPIDER_LOG_PATH);
	this->m_ptr_log_prefix=q_strdup(SPIDER_LOG_PREFIX);
	this->m_log_size=SPIDER_LOG_SIZE;
	this->m_stat_starttime=time(NULL);
	this->m_stat_lastinteraction=time(NULL);
	this->m_stat_numconnections=0;
	this->m_stat_succ_conn=0;
	this->m_stat_failed_conn=0;
	this->m_stat_rejected_conn=0;
	this->m_ptr_remote_monitor=NULL;
}

QSpiderServer::~QSpiderServer()
{
	m_exit_flag=true;

	q_free(m_pidfile);
	q_free(m_ptr_log_path);
	q_free(m_ptr_log_prefix);

	q_close_socket(m_sock_svr);

	for(int32_t i=0; i<m_thread_max; ++i)
	{
		m_ptr_client_trigger->signal();
		m_ptr_entry_trigger->signal();
		m_ptr_dp_trigger->signal();

		q_sleep(1);
		while(m_ptr_trd_info[i].flag!=-1)
			q_sleep(1);

		q_delete_array<char>(m_ptr_trd_info[i].ptr_buf);

		if(m_ptr_trd_info[i].for_worker!=NULL)
		{
			interfaceInfo* pst_interface=reinterpret_cast<interfaceInfo*>(m_ptr_trd_info[i].for_worker);
			Q_CHECK_PTR(pst_interface);

			Downloader* downloader=reinterpret_cast<Downloader*>(pst_interface->ptr_downloader);
			q_delete<Downloader>(downloader);

			Parser* parser=reinterpret_cast<Parser*>(pst_interface->ptr_parser);
			q_delete<Parser>(parser);

			q_delete<interfaceInfo>(pst_interface);
		}
	}

	q_delete< QQueue<clientInfo> >(m_ptr_client_queue);
	q_delete< QTrigger >(m_ptr_client_trigger);

	q_delete< QPoolAllocator >(m_ptr_pool_allocator);

	q_delete< QTrigger >(m_ptr_entry_trigger);

	q_delete< QQueue<taskInfo> >(m_ptr_dp_queue);
	q_delete< QTrigger >(m_ptr_dp_trigger);

	q_delete<ServerPicker>(m_ptr_picker);
	q_delete<TemplateManager>(m_ptr_tpl_manager);

	fclose(m_send_write_fp);
	q_delete< QDiskCache<uint64_t> >(m_ptr_uniq_cache);

	q_delete<QLogger>(m_ptr_logger);
	q_free(m_ptr_log_path);
	q_free(m_ptr_log_prefix);

	q_delete<configInfo>(m_ptr_cfg_info);
	q_delete_array<threadInfo>(m_ptr_trd_info);

	q_delete<QRemoteMonitor>(m_ptr_remote_monitor);
}

int32_t QSpiderServer::init(const char* cfg_file)
{
	Q_INFO("Hello, spider-server...");

	int32_t ret=0;

	/* logger */
	m_ptr_logger=q_new<QLogger>();
	if(m_ptr_logger==NULL) {
		Q_FATAL("m_ptr_logger alloc error, null value!");
		return SPIDER_ERR;
	}

	ret=m_ptr_logger->init(m_ptr_log_path, m_ptr_log_prefix, m_log_size);
	if(ret<0) {
		Q_FATAL("m_ptr_logger init error, ret = (%d)", ret);
		return SPIDER_ERR;
	}

	/* config */
	m_ptr_cfg_info=q_new<configInfo>();
	if(m_ptr_cfg_info==NULL) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"m_ptr_cfg_info is null!");
		return SPIDER_ERR;
	}

	QConfigReader conf;
	if(conf.init(cfg_file)!=0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"config init error, cfg_file = (%s)", \
				cfg_file);
		return SPIDER_ERR;
	}

	ret=conf.getFieldString("server-name", m_ptr_cfg_info->server_name, sizeof(m_ptr_cfg_info->server_name));
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init SERVER_NAME error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldString("server-ip", m_ptr_cfg_info->server_ip, sizeof(m_ptr_cfg_info->server_ip));
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init server-ip error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldInt32("server-port", m_ptr_cfg_info->server_port);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init server-port error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldInt32("monitor-port", m_ptr_cfg_info->monitor_port);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init monitor-port error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldInt32("sock-timeout", m_ptr_cfg_info->sock_timeout);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init sock-timeout error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldInt32("comm-thread-max", m_ptr_cfg_info->comm_thread_max);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init comm-thread-max error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldInt32("comm-buffer-size", m_ptr_cfg_info->comm_buffer_size);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init comm-buffer-size error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldInt32("comm-thread-timeout", m_ptr_cfg_info->comm_thread_timeout);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init comm-thread-timeout error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldInt32("work-thread-max", m_ptr_cfg_info->work_thread_max);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init work-thread-max error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldInt32("work-buffer-size", m_ptr_cfg_info->work_buffer_size);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init work-buffer-size error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldInt32("work-thread-timeout", m_ptr_cfg_info->work_thread_timeout);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init work-thread-timeout error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}
	
	ret=conf.getFieldInt32("scan-thread-max", m_ptr_cfg_info->scan_thread_max);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init scan-thread-max error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldInt32("scan-buffer-size", m_ptr_cfg_info->scan_buffer_size);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init scan-buffer-size error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldInt32("scan-thread-timeout", m_ptr_cfg_info->scan_thread_timeout);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init scan-thread-timeout error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}
	
	ret=conf.getFieldInt32("extract-thread-max", m_ptr_cfg_info->extract_thread_max);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init extract-thread-max error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldInt32("extract-buffer-size", m_ptr_cfg_info->extract_buffer_size);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init extract-buffer-size error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldInt32("extract-thread-timeout", m_ptr_cfg_info->extract_thread_timeout);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init extract-thread-timeout error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}
	
	ret=conf.getFieldInt32("send-thread-max", m_ptr_cfg_info->send_thread_max);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init send-thread-max error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldInt32("send-buffer-size", m_ptr_cfg_info->send_buffer_size);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init send-buffer-size error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldInt32("send-thread-timeout", m_ptr_cfg_info->send_thread_timeout);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init send-thread-timeout error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldYesNo("auto-update", m_ptr_cfg_info->auto_update);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init auto-update error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}
	
	ret=conf.getFieldYesNo("only-update", m_ptr_cfg_info->only_update);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init only-update error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}
	
	ret=conf.getFieldInt32("update-interval", m_ptr_cfg_info->update_interval);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init update-interval error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}
	
	ret=conf.getFieldInt32("download-interval", m_ptr_cfg_info->download_interval);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init download-interval error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}
	
	ret=conf.getFieldInt32("download-timeout", m_ptr_cfg_info->download_timeout);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init download-timeout error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}
	
	ret=conf.getFieldInt32("retry-num-max", m_ptr_cfg_info->retry_num_max);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init retry-num-max error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}
	
	ret=conf.getFieldInt32("retry-interval", m_ptr_cfg_info->retry_interval);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init retry-interval error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}
	
	ret=conf.getFieldInt32("chunk-size", m_ptr_cfg_info->chunk_size);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init chunk_size error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldInt32("client-queue-size", m_ptr_cfg_info->client_queue_size);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init client-queue-size error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldInt32("task-queue-size", m_ptr_cfg_info->task_queue_size);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init task-queue-size error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldString("templ-list-path", m_ptr_cfg_info->templ_list_path, sizeof(m_ptr_cfg_info->templ_list_path));
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init templ-list-path error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldString("templ-detail-path", m_ptr_cfg_info->templ_detail_path, sizeof(m_ptr_cfg_info->templ_detail_path));
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init templ-detail-path error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldString("img-server", m_ptr_cfg_info->img_server, sizeof(m_ptr_cfg_info->img_server));
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init img-server error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldInt32("img-port", m_ptr_cfg_info->img_port);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init img-port error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldString("send-data-path", m_ptr_cfg_info->send_data_path, sizeof(m_ptr_cfg_info->send_data_path));
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init send-data-path error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldString("send-bak-path", m_ptr_cfg_info->send_bak_path, sizeof(m_ptr_cfg_info->send_bak_path));
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init send-bak-path error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldString("send-read-path", m_ptr_cfg_info->send_read_path, sizeof(m_ptr_cfg_info->send_read_path));
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init send-read-path error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldString("send-write-path", m_ptr_cfg_info->send_write_path, sizeof(m_ptr_cfg_info->send_write_path));
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init send-write-path error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldString("send-ip", m_ptr_cfg_info->send_ip, sizeof(m_ptr_cfg_info->send_ip));
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init send-ip error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=conf.getFieldInt32("send-port", m_ptr_cfg_info->send_port);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init send-port error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

#if defined (DEBUG)
	Q_INFO("server-name              = %s", m_ptr_cfg_info->server_name);

	Q_INFO("server-ip                = %s", m_ptr_cfg_info->server_ip);
	Q_INFO("server-port              = %d", m_ptr_cfg_info->server_port);
	Q_INFO("monitor-port             = %d", m_ptr_cfg_info->monitor_port);

	Q_INFO("sock-timeout             = %d", m_ptr_cfg_info->sock_timeout);

	Q_INFO("comm-thread-max          = %d", m_ptr_cfg_info->comm_thread_max);
	Q_INFO("comm-buffer-size         = %d", m_ptr_cfg_info->comm_buffer_size);
	Q_INFO("comm-thread-timeout      = %d", m_ptr_cfg_info->comm_thread_timeout);

	Q_INFO("work-thread-max          = %d", m_ptr_cfg_info->work_thread_max);
	Q_INFO("work-buffer-size         = %d", m_ptr_cfg_info->work_buffer_size);
	Q_INFO("work-thread-timeout      = %d", m_ptr_cfg_info->work_thread_timeout);

	Q_INFO("scan-thread-max          = %d", m_ptr_cfg_info->scan_thread_max);
	Q_INFO("scan-buffer-size         = %d", m_ptr_cfg_info->scan_buffer_size);
	Q_INFO("scan-thread-timeout      = %d", m_ptr_cfg_info->scan_thread_timeout);

	Q_INFO("extract-thread-max       = %d", m_ptr_cfg_info->extract_thread_max);
	Q_INFO("extract-buffer-size      = %d", m_ptr_cfg_info->extract_buffer_size);
	Q_INFO("extract-thread-timeout   = %d", m_ptr_cfg_info->extract_thread_timeout);

	Q_INFO("send-thread-max          = %d", m_ptr_cfg_info->send_thread_max);
	Q_INFO("send-buffer-size         = %d", m_ptr_cfg_info->send_buffer_size);
	Q_INFO("send-thread-timeout      = %d", m_ptr_cfg_info->send_thread_timeout);

	Q_INFO("auto-update              = %d", m_ptr_cfg_info->auto_update);
	Q_INFO("only-update              = %d", m_ptr_cfg_info->only_update);

	Q_INFO("update-interval          = %d", m_ptr_cfg_info->update_interval);
	Q_INFO("download-interval        = %d", m_ptr_cfg_info->download_interval);
	Q_INFO("download-timeout         = %d", m_ptr_cfg_info->download_timeout);

	Q_INFO("retry-num-max            = %d", m_ptr_cfg_info->retry_num_max);
	Q_INFO("retry-interval           = %d", m_ptr_cfg_info->retry_interval);

	Q_INFO("chunk-size               = %d", m_ptr_cfg_info->chunk_size);
	Q_INFO("client-queue-size        = %d", m_ptr_cfg_info->client_queue_size);
	Q_INFO("task-queue-size          = %d", m_ptr_cfg_info->task_queue_size);

	Q_INFO("templ-list-path          = %s", m_ptr_cfg_info->templ_list_path);
	Q_INFO("templ-detail-path        = %s", m_ptr_cfg_info->templ_detail_path);

	Q_INFO("img-server               = %s", m_ptr_cfg_info->img_server);
	Q_INFO("img-port                 = %d", m_ptr_cfg_info->img_port);

	Q_INFO("send-data-path           = %s", m_ptr_cfg_info->send_data_path);
	Q_INFO("send-bak-path            = %s", m_ptr_cfg_info->send_bak_path);
	Q_INFO("send-read-path           = %s", m_ptr_cfg_info->send_read_path);
	Q_INFO("send-write-path          = %s", m_ptr_cfg_info->send_write_path);

	Q_INFO("send-ip                  = %s", m_ptr_cfg_info->send_ip);
	Q_INFO("send-port                = %d", m_ptr_cfg_info->send_port);
#endif

	/* allocator */
	m_ptr_pool_allocator=q_new< QPoolAllocator >();
	if(m_ptr_pool_allocator==NULL) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"m_ptr_pool_allocator is null!");
		return SPIDER_ERR;
	}

	ret=m_ptr_pool_allocator->init(m_ptr_cfg_info->chunk_size);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"m_ptr_pool_allocator init error, ret = (%d)!", \
				ret);
		return SPIDER_ERR;
	}

	/* client queue */
	m_ptr_client_queue=q_new< QQueue<clientInfo> >();
	if(m_ptr_client_queue==NULL) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"m_ptr_client_queue is null!");
		return SPIDER_ERR;
	}

	ret=m_ptr_client_queue->init(m_ptr_cfg_info->client_queue_size+1);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init m_ptr_client_queue error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	m_ptr_client_trigger=q_new<QTrigger>();
	if(m_ptr_client_trigger==NULL) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"m_ptr_client_trigger is null!");
		return SPIDER_ERR;
	}

	/* entries */
	m_ptr_entry_trigger=q_new<QTrigger>();
	if(m_ptr_entry_trigger==NULL) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"m_ptr_entry_trigger is null!");
		return SPIDER_ERR;
	}

	/* detail page queue */
	m_ptr_dp_queue=q_new< QQueue<taskInfo> >();
	if(m_ptr_dp_queue==NULL) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"m_ptr_dp_queue is null!");
		return SPIDER_ERR;
	}

	ret=m_ptr_dp_queue->init(m_ptr_cfg_info->task_queue_size+1);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init m_dp_task_queue error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	m_ptr_dp_trigger=q_new<QTrigger>();
	if(m_ptr_dp_trigger==NULL) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"m_ptr_dp_trigger is null!");
		return SPIDER_ERR;
	}

	/* server picker */
	m_ptr_picker=q_new<ServerPicker>();
	if(m_ptr_picker==NULL) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"m_ptr_picker is null!");
		return SPIDER_ERR;
	}

	ret=m_ptr_picker->init();
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"m_ptr_picker init error, ret=(%d)!",\
				ret);
		return SPIDER_ERR;
	}

	/* template */
	m_ptr_tpl_manager=q_new<TemplateManager>();
	if(m_ptr_tpl_manager==NULL) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"m_ptr_tpl_manager is null!");
		return SPIDER_ERR;
	}

	ret=m_ptr_tpl_manager->init();
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"m_ptr_tpl_manager init error, ret=(%d)!",\
				ret);
		return SPIDER_ERR;
	}

#if 1
	ret=load_templates(0, m_ptr_cfg_info->templ_list_path);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"load templates error, ret = (%d)!", \
				ret);
		return SPIDER_ERR;
	}

	ret=load_templates(1, m_ptr_cfg_info->templ_detail_path);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"load templates error, ret = (%d)!", \
				ret);
		return SPIDER_ERR;
	}
#endif

	/* persistence */
	m_ptr_uniq_cache=q_new< QDiskCache<uint64_t> >();
	if(m_ptr_uniq_cache==NULL) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"m_ptr_uniq_cache is null!");
		return SPIDER_ERR;
	}

	ret=m_ptr_uniq_cache->init(m_cache_bucket_size, 0, m_cache_interval);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"m_uniq_cache init error, ret = (%d)!", \
				ret);
		return SPIDER_ERR;
	}

	/* dir */
	if(!QDir::mkdir(m_ptr_cfg_info->send_data_path)) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"mkdir send-data-path (%s) error!", \
				m_ptr_cfg_info->send_data_path);
		return SPIDER_ERR;
	}

	if(!QDir::mkdir(m_ptr_cfg_info->send_bak_path)) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"mkdir send-bak-path (%s) error!", \
				m_ptr_cfg_info->send_bak_path);
		return SPIDER_ERR;
	}

	if(access(m_ptr_cfg_info->send_write_path, 00)==0)
	{
		ret=q_repair_file(m_ptr_cfg_info->send_write_path, (const char*)&I_SEND_FILE_MARK, sizeof(I_SEND_FILE_MARK));
		if(ret<0) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"q_repair_file (%s) error, ret = (%d)!", \
					m_ptr_cfg_info->send_write_path, \
					ret);
			return SPIDER_ERR;
		}

		m_send_write_fp=::fopen(m_ptr_cfg_info->send_write_path, "rb+");
		if(m_send_write_fp==NULL) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"fopen (%s) error!", \
					m_ptr_cfg_info->send_write_path);
			return SPIDER_ERR;
		}

		if(::fseek(m_send_write_fp, 0, SEEK_END)!=0) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"fseek (%s) error!", \
					m_ptr_cfg_info->send_write_path);
			return SPIDER_ERR;
		}
	}

	/* network */
	ret=q_init_socket();
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init socket error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	ret=q_TCP_server(m_sock_svr, m_ptr_cfg_info->server_port);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"TCP server error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	/* threads */
	m_thread_max=m_ptr_cfg_info->comm_thread_max \
		     +m_ptr_cfg_info->work_thread_max \
		     +m_ptr_cfg_info->scan_thread_max \
		     +m_ptr_cfg_info->extract_thread_max \
		     +m_ptr_cfg_info->send_thread_max;

	m_ptr_trd_info=q_new_array<threadInfo>(m_thread_max);
	if(m_ptr_trd_info==NULL) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"create m_ptr_trd_info error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	threadInfo* ptr_trd=m_ptr_trd_info;
	for(int32_t i=0; i!=m_ptr_cfg_info->comm_thread_max; ++i)
	{
		ptr_trd[i].pthis=this;
		ptr_trd[i].id=i;
		ptr_trd[i].status=0;
		ptr_trd[i].flag=0;
		ptr_trd[i].timeout=m_ptr_cfg_info->comm_thread_timeout;
		ptr_trd[i].buf_size=m_ptr_cfg_info->comm_buffer_size;
		ptr_trd[i].ptr_buf=q_new_array<char>(ptr_trd[i].buf_size);
		if(ptr_trd[i].ptr_buf==NULL) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"create comm_thread (%d) buffer error", \
					i+1);
			return SPIDER_ERR;
		}

		ret=q_create_thread(QSpiderServer::comm_thread, ptr_trd+i);
		if(ret<0) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"create comm_thread (%d) error", \
					i+1);
			return SPIDER_ERR;
		}

		while(ptr_trd[i].flag!=1)
			q_sleep(1);

		if(ptr_trd[i].flag==-1) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"start comm_thread (%d) error", \
					i+1);
			return SPIDER_ERR;
		}
	}

	ptr_trd+=m_ptr_cfg_info->comm_thread_max;
	for(int32_t i=0; i!=m_ptr_cfg_info->work_thread_max; ++i)
	{
		ptr_trd[i].pthis=this;
		ptr_trd[i].id=i;
		ptr_trd[i].status=0;
		ptr_trd[i].flag=0;
		ptr_trd[i].timeout=m_ptr_cfg_info->work_thread_timeout;
		ptr_trd[i].buf_size=m_ptr_cfg_info->work_buffer_size;
		ptr_trd[i].ptr_buf=q_new_array<char>(ptr_trd[i].buf_size);
		if(ptr_trd[i].ptr_buf==NULL) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"create work_thread (%d) buffer error", \
					i+1);
			return SPIDER_ERR;
		}

		interfaceInfo* pst_interface=q_new<interfaceInfo>();
		if(pst_interface==NULL) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"pst_interface (%d) is null!", \
					i+1);
			return SPIDER_ERR;
		}

		pst_interface->ptr_downloader=q_new<Downloader>();
		if(pst_interface->ptr_downloader==NULL) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"downloader (%d) is null!", \
					i+1);
			return SPIDER_ERR;
		}

		if(pst_interface->ptr_downloader->init()) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"downloader (%d) init error!", \
					i+1);
			return SPIDER_ERR;
		}

		pst_interface->ptr_parser=q_new<Parser>();
		if(pst_interface->ptr_parser==NULL) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"parser (%d) is null!", \
					i+1);
			return SPIDER_ERR;
		}

		if(pst_interface->ptr_parser->init(m_ptr_cfg_info->img_server, m_ptr_cfg_info->img_port)) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"parser (%d) init error!", \
					i+1);
			return SPIDER_ERR;
		}

		ptr_trd[i].for_worker=reinterpret_cast<void*>(pst_interface);

		ret=q_create_thread(QSpiderServer::work_thread, ptr_trd+i);
		if(ret<0) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"create work_thread (%d) error", \
					i+1);
			return SPIDER_ERR;
		}

		while(ptr_trd[i].flag!=1)
			q_sleep(1);

		if(ptr_trd[i].flag==-1) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"start work_thread (%d) error", \
					i+1);
			return SPIDER_ERR;
		}
	}

	ptr_trd+=m_ptr_cfg_info->work_thread_max;
	for(int32_t i=0; i!=m_ptr_cfg_info->scan_thread_max; ++i)
	{
		ptr_trd[i].pthis=this;
		ptr_trd[i].id=i;
		ptr_trd[i].status=0;
		ptr_trd[i].flag=0;
		ptr_trd[i].timeout=m_ptr_cfg_info->scan_thread_timeout;
		ptr_trd[i].buf_size=m_ptr_cfg_info->scan_buffer_size;
		ptr_trd[i].ptr_buf=q_new_array<char>(ptr_trd[i].buf_size);
		if(ptr_trd[i].ptr_buf==NULL) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"create scan_thread (%d) buffer error", \
					i+1);
			return SPIDER_ERR;
		}

		interfaceInfo* pst_interface=q_new<interfaceInfo>();
		if(pst_interface==NULL) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"pst_interface (%d) is null!", \
					i+1);
			return SPIDER_ERR;
		}

		pst_interface->ptr_downloader=q_new<Downloader>();
		if(pst_interface->ptr_downloader==NULL) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"downloader (%d) is null!", \
					i+1);
			return SPIDER_ERR;
		}

		if(pst_interface->ptr_downloader->init()) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"downloader (%d) init error!", \
					i+1);
			return SPIDER_ERR;
		}

		pst_interface->ptr_parser=q_new<Parser>();
		if(pst_interface->ptr_parser==NULL) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"parser (%d) is null!", \
					i+1);
			return SPIDER_ERR;
		}

		if(pst_interface->ptr_parser->init(m_ptr_cfg_info->img_server, m_ptr_cfg_info->img_port)) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"parser (%d) init error!", \
					i+1);
			return SPIDER_ERR;
		}

		ptr_trd[i].for_worker=reinterpret_cast<void*>(pst_interface);

		ret=q_create_thread(QSpiderServer::scan_thread, ptr_trd+i);
		if(ret<0) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"create scan_thread (%d) error", \
					i+1);
			return SPIDER_ERR;
		}

		while(ptr_trd[i].flag!=1)
			q_sleep(1);

		if(ptr_trd[i].flag==-1) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"start scan_thread (%d) error", \
					i+1);
			return SPIDER_ERR;
		}
	}

	ptr_trd+=m_ptr_cfg_info->scan_thread_max;
	for(int32_t i=0; i!=m_ptr_cfg_info->extract_thread_max; ++i)
	{
		ptr_trd[i].pthis=this;
		ptr_trd[i].id=i;
		ptr_trd[i].status=0;
		ptr_trd[i].flag=0;
		ptr_trd[i].timeout=m_ptr_cfg_info->extract_thread_timeout;
		ptr_trd[i].buf_size=m_ptr_cfg_info->extract_buffer_size;
		ptr_trd[i].ptr_buf=q_new_array<char>(ptr_trd[i].buf_size);
		if(ptr_trd[i].ptr_buf==NULL) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"create extract_thread (%d) buffer error", \
					i+1);
			return SPIDER_ERR;
		}

		interfaceInfo* pst_interface=q_new<interfaceInfo>();
		if(pst_interface==NULL) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"pst_interface (%d) is null!", \
					i+1);
			return SPIDER_ERR;
		}

		pst_interface->ptr_downloader=q_new<Downloader>();
		if(pst_interface->ptr_downloader==NULL) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"downloader (%d) is null!", \
					i+1);
			return SPIDER_ERR;
		}

		if(pst_interface->ptr_downloader->init()) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"downloader (%d) init error!", \
					i+1);
			return SPIDER_ERR;
		}

		pst_interface->ptr_parser=q_new<Parser>();
		if(pst_interface->ptr_parser==NULL) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"parser (%d) is null!", \
					i+1);
			return SPIDER_ERR;
		}

		if(pst_interface->ptr_parser->init(m_ptr_cfg_info->img_server, m_ptr_cfg_info->img_port)) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"parser (%d) init error!", \
					i+1);
			return SPIDER_ERR;
		}

		ptr_trd[i].for_worker=reinterpret_cast<void*>(pst_interface);

		ret=q_create_thread(QSpiderServer::extract_thread, ptr_trd+i);
		if(ret<0) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"create extract_thread (%d) error", \
					i+1);
			return SPIDER_ERR;
		}

		while(ptr_trd[i].flag!=1)
			q_sleep(1);

		if(ptr_trd[i].flag==-1) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"start extract_thread (%d) error", \
					i+1);
			return SPIDER_ERR;
		}
	}

	ptr_trd+=m_ptr_cfg_info->extract_thread_max;
	for(int32_t i=0; i!=m_ptr_cfg_info->send_thread_max; ++i)
	{
		ptr_trd[i].pthis=this;
		ptr_trd[i].id=i;
		ptr_trd[i].status=0;
		ptr_trd[i].flag=0;
		ptr_trd[i].timeout=m_ptr_cfg_info->send_thread_timeout;
		ptr_trd[i].buf_size=m_ptr_cfg_info->send_buffer_size;
		ptr_trd[i].ptr_buf=q_new_array<char>(ptr_trd[i].buf_size);
		if(ptr_trd[i].ptr_buf==NULL) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"create send_thread (%d) buffer error", \
					i+1);
			return SPIDER_ERR;
		}

		ret=q_create_thread(QSpiderServer::send_thread, ptr_trd+i);
		if(ret<0) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"create send_thread (%d) error", \
					i+1);
			return SPIDER_ERR;
		}

		while(ptr_trd[i].flag!=1)
			q_sleep(1);

		if(ptr_trd[i].flag==-1) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"start send_thread (%d) error", \
					i+1);
			return SPIDER_ERR;
		}
	}

	/* monitor */
	m_ptr_remote_monitor=q_new<QRemoteMonitor>();
	if(m_ptr_remote_monitor==NULL) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"m_ptr_remote_monitor is null!");
		return SPIDER_ERR;
	}

	ret=m_ptr_remote_monitor->init(m_ptr_cfg_info->monitor_port, 10000, get_thread_state, this, 1);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"init monitor error, ret = (%d)", \
				ret);
		return SPIDER_ERR;
	}

	m_start_flag=true;

	return 0;
}

int32_t QSpiderServer::load_templates(int32_t tid, const char* tplfile)
{
	if(tplfile==NULL)
		return SPIDER_ERR;

	char xml[1<<20]={0};
	int32_t xml_len=0;

	FILE* fp=NULL;
	int32_t ret=0;

	xml_len=q_get_file_size((char*)tplfile);

	fp=fopen(tplfile, "rb");
	if(fp==NULL) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"open template file (%s) error!", \
				tplfile);
		return SPIDER_ERR;
	}

	if(fread(xml, xml_len, 1, fp)!=1) {
		fclose(fp);
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"fread template file (%s) error!", \
				tplfile);
		return SPIDER_ERR;
	}

	fclose(fp);
	fp=NULL;

	ret=m_ptr_tpl_manager->add_template(tid, xml, xml_len);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"add_template error, ret = (%d)!", \
				ret);
		return SPIDER_ERR;
	}

	return SPIDER_OK;
}

int32_t QSpiderServer::run()
{
	m_ptr_logger->log(LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, 1, \
			"Input s or S to stop the QSpiderServer!");

	char ch_;
	std::cin>>ch_;
	while(ch_!='s'&&ch_!='S') {
		m_ptr_logger->log(LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, 1, \
				"input s or S to stop the QSpiderServer!");
		std::cin>>ch_;
	}

	m_ptr_logger->log(LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, 1, \
			"QSpiderServer stop!");

	return SPIDER_OK;
}

// 通信线程(半同步/半反应堆)
Q_THREAD_T QSpiderServer::comm_thread(void* ptr_info)
{
	threadInfo* ptr_trd=reinterpret_cast<threadInfo*>(ptr_info);
	Q_CHECK_PTR(ptr_trd);

	QSpiderServer* ptr_this=reinterpret_cast<QSpiderServer*>(ptr_trd->pthis);
	Q_CHECK_PTR(ptr_this);

	clientInfo client_info;

	ptr_trd->flag=1;

	while(!ptr_this->m_exit_flag)
	{
		ptr_trd->status=0;

		try {
			if(q_accept_socket(ptr_this->m_sock_svr, client_info.sock_client, \
						client_info.client_ip, \
						client_info.client_port)) {
				ptr_this->m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
						"TCP socket accept error!");
				throw -1;
			}

			ptr_trd->status=1;
			ptr_trd->sw.start();

			ptr_this->m_ptr_logger->log(LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, 1, \
					"---------- Request from [%s:%d] ----------", \
					client_info.client_ip, \
					client_info.client_port);

			if(q_set_overtime(client_info.sock_client, ptr_this->m_ptr_cfg_info->sock_timeout)) {
				ptr_this->m_ptr_logger->log(LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, 1, \
						"TCP socket set timeout (%d) error!", \
						ptr_this->m_ptr_cfg_info->sock_timeout);
				throw -2;
			}

			client_info.request_buffer=ptr_this->m_ptr_pool_allocator->alloc();
			Q_CHECK_PTR(client_info.request_buffer);

			client_info.reply_buffer=ptr_this->m_ptr_pool_allocator->alloc();
			Q_CHECK_PTR(client_info.reply_buffer);

			ptr_this->m_ptr_client_queue->push(client_info);
			ptr_this->m_ptr_client_trigger->signal();
		} catch(const int32_t err) {
			ptr_this->m_ptr_logger->log(LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, 1, \
					"Task process socket error, err = (%d)!",
					err);
		}

		ptr_trd->sw.stop();
	}

	ptr_trd->flag=-1;
	return NULL;
}

// 工作线程
Q_THREAD_T QSpiderServer::work_thread(void* ptr_info)
{
	threadInfo* ptr_trd=reinterpret_cast<threadInfo*>(ptr_info);
	Q_CHECK_PTR(ptr_trd);

	QSpiderServer* ptr_this=reinterpret_cast<QSpiderServer*>(ptr_trd->pthis);
	Q_CHECK_PTR(ptr_this);

	interfaceInfo* pst_interface=reinterpret_cast<interfaceInfo*>(ptr_trd->for_worker);
	Q_CHECK_PTR(pst_interface);

	Downloader* downloader=reinterpret_cast<Downloader*>(pst_interface->ptr_downloader);
	Q_CHECK_PTR(downloader);

	Parser* parser=reinterpret_cast<Parser*>(pst_interface->ptr_parser);
	Q_CHECK_PTR(parser);

	replyHeader reply_header;
	reply_header.version=PROTOCOL_HEAD_VERSION;
	reply_header.length=sizeof(replyHeader)-sizeof(uint64_t)-sizeof(int32_t);
	reply_header.cmd_type=0;
	reply_header.status_code=0;

	int32_t header_len=sizeof(uint64_t)+sizeof(int32_t);
	int32_t content_len=0;

	clientInfo client_info;
	int32_t ret=0;

	QStopwatch sw_download;

	ptr_trd->flag=1;

	while(!ptr_this->m_exit_flag)
	{
		ptr_trd->status=0;

		ptr_this->m_ptr_client_trigger->wait();

		while(ptr_this->m_ptr_client_queue->pop_non_blocking(client_info)==0)
		{
			ptr_trd->status=1;
			ptr_trd->sw.start();

			sw_download.start();

			try {
				if(q_recvbuf(client_info.sock_client, client_info.request_buffer, header_len)) {
					ptr_this->m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
							"TCP socket recv header error!");
					throw -1;
				}

				if(*(uint64_t*)client_info.request_buffer!=PROTOCOL_HEAD_VERSION) {
					ptr_this->m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
							"TCP socket version error, version = (%.*s)", \
							sizeof(uint64_t), \
							client_info.request_buffer);
					throw -2;
				}

				content_len=*(int32_t*)(client_info.request_buffer+8);
				if(content_len<=0) {
					ptr_this->m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
							"Recv len error: content_len = (%d)", \
							content_len);
					throw -3;
				}

				if(content_len>0)
				{
					if(content_len>client_info.request_buffer_size-header_len) {
						ptr_this->m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
								"Content length error: content_len(%d) > content_size(%d)", \
								content_len, \
								client_info.request_buffer_size-header_len);
						throw -4;
					}

					if(q_recvbuf(client_info.sock_client, client_info.request_buffer+header_len, content_len)) {
						ptr_this->m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
								"TCP socket recv content error, size = (%d)", \
								content_len);
						throw -5;
					}
				}

				ret=ptr_this->fun_process(client_info.request_buffer, \
						header_len+content_len, \
						client_info.reply_buffer, \
						client_info.reply_buffer_size, \
						ptr_trd->ptr_buf, \
						ptr_trd->buf_size, \
						downloader, \
						parser);
				if(ret<0) {
					ptr_this->m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
							"fun process error: ret = (%d)", \
							ret);
					throw ret;
				}

				if(q_sendbuf(client_info.sock_client, client_info.reply_buffer, ret)) {
					ptr_this->m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
							"TCP socket send error: send length = (%d)", \
							ret);
					throw -6;
				}

				q_close_socket(client_info.sock_client);
			} catch(const int32_t err) {
				reply_header.status_code=err;
				q_sendbuf(client_info.sock_client, (char*)(&reply_header), sizeof(replyHeader));
				q_close_socket(client_info.sock_client);

				ptr_this->m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
						"Working thread [%s:%d] process error, ret = (%d)!", \
						client_info.client_ip, \
						client_info.client_port, \
						err); 
			}

			ptr_this->m_ptr_pool_allocator->free(client_info.request_buffer);
			ptr_this->m_ptr_pool_allocator->free(client_info.reply_buffer);

			q_add_and_fetch(&ptr_this->m_stat_numconnections);

			sw_download.stop();
			ptr_this->m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"Task (%u) process finshed, which consumed: %dms!", \
					ptr_this->m_stat_numconnections, \
					sw_download.elapsed_ms());
		}
		ptr_trd->sw.stop();
	}

	ptr_trd->flag=-1;
	return NULL;
}

// 任务处理函数
int32_t QSpiderServer::fun_process(char* request_buffer, int32_t request_len, char* reply_buf, int32_t reply_size, \
		char* ptr_buf, \
		int32_t buf_size, \
		Downloader* downloader, \
		Parser* parser)
{
	Q_CHECK_PTR(request_buffer);
	Q_CHECK_PTR(reply_buf);
	Q_CHECK_PTR(ptr_buf);
	Q_CHECK_PTR(downloader);

	Q_ASSERT(request_len>0, "request_len must be larger than 0!");
	Q_ASSERT(reply_size>0, "reply_size must be larger than 0!");
	Q_ASSERT(buf_size>0, "buf_size must be larger than 0!");

	char *ptr_temp=request_buffer;
	char *ptr_end=ptr_temp+request_len;

	int16_t operate_type=0;
	char* ptr_xml=NULL;
	int32_t xml_len=0;
	int32_t tid=0;

	int32_t reply_len=0;
	int32_t ret=0;

	requestHeader* request_header=reinterpret_cast<requestHeader*>(ptr_temp);
	ptr_temp+=sizeof(requestHeader);

	if(request_header->protocol_type!=1) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"Protocol type error, protocol_type = (%d)", \
				request_header->protocol_type);
		return -11;
	}

	if(request_header->source_type!=0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"Source type error, source_type = (%d)", \
				request_header->source_type);
		return -12;
	}

	if(request_header->cmd_type==0) {
		// 入口页强刷
		if((operate_type=*(int16_t*)ptr_temp)==0) {
			ptr_temp+=sizeof(int16_t);
		} else {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"Operate type error, operate_type = (%d)", \
					operate_type);
			return -13;
		}

		if((xml_len=*(int32_t*)ptr_temp)<=0) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"XML length error, xml_len = (%d)", \
					xml_len);
			return -14;
		}
		ptr_temp+=sizeof(int32_t);

		ptr_xml=ptr_temp;
		ptr_temp+=xml_len;

		if(ptr_temp+sizeof(int32_t)!=ptr_end) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"Fun process protocol format error!"); 
			return -15;
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		replyHeader *reply_header=reinterpret_cast<replyHeader*>(reply_buf);
		reply_header->version=PROTOCOL_HEAD_VERSION;
		reply_header->length=0;
		reply_header->status_code=0;
		reply_header->cmd_type=request_header->cmd_type;

		ret=process_entry(ptr_xml, xml_len);
		if(ret<0) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"process entries error, ret = (%d)", \
					ret);
			return -16;
		}

		reply_len=sizeof(replyHeader);;
		reply_header->length=reply_len-sizeof(PROTOCOL_HEAD_VERSION)-sizeof(int32_t);
	} else if(request_header->cmd_type==1) {
		// 模板测试
		if((operate_type=*(int16_t*)ptr_temp)<0) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"Operate type error, operate_type = (%d)", \
					operate_type);
			return -17;
		}
		ptr_temp+=sizeof(int16_t);

		if((xml_len=*(int32_t*)ptr_temp)<=0) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"XML length error, xml_len = (%d)", \
					xml_len);
			return -18;
		}
		ptr_temp+=sizeof(int32_t);

		ptr_xml=ptr_temp;
		ptr_temp+=xml_len;

		if(ptr_temp+sizeof(int32_t)!=ptr_end) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"Fun process protocol format error!"); 
			return -19;
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		replyHeader *reply_header=reinterpret_cast<replyHeader*>(reply_buf);
		reply_header->version=PROTOCOL_HEAD_VERSION;
		reply_header->length=0;
		reply_header->status_code=0;
		reply_header->cmd_type=request_header->cmd_type;

		char *ptr_reply_temp=reply_buf+sizeof(replyHeader)+sizeof(int32_t);
		char *ptr_reply_end=reply_buf+reply_size;

		ret=process_url(ptr_xml, xml_len, downloader, parser, operate_type, ptr_buf, buf_size, ptr_reply_temp, ptr_reply_end-ptr_reply_temp);
		if(ret<0) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"process extractor error, ret = (%d)", \
					ret);
			return -20;
		}
		ptr_reply_temp+=ret;

		reply_len=ptr_reply_temp-reply_buf;
		reply_header->length=reply_len-sizeof(PROTOCOL_HEAD_VERSION)-4;
		*(int32_t*)(reply_buf+sizeof(replyHeader))=reply_len-sizeof(replyHeader)-4;
	} else if(request_header->cmd_type==2) {
		// 模板增删改
		if((operate_type=*(int16_t*)ptr_temp)<0) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"Operate type error, operate_type = (%d)", \
					operate_type);
			return -21;
		}
		ptr_temp+=sizeof(int16_t);

		if((xml_len=*(int32_t*)ptr_temp)<=0) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"XML length error, xml_len = (%d)", \
					xml_len);
			return -22;
		}
		ptr_temp+=sizeof(int32_t);

		ptr_xml=ptr_temp;
		ptr_temp+=xml_len;

		if((tid=*(int32_t*)ptr_temp)<0) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"tid error, tid = (%d)", \
					tid);
			return -23;
		}

		if(ptr_temp+sizeof(int32_t)!=ptr_end) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"Fun process protocol format error!"); 
			return -24;
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		replyHeader *reply_header=reinterpret_cast<replyHeader*>(reply_buf);
		reply_header->version=PROTOCOL_HEAD_VERSION;
		reply_header->length=0;
		reply_header->status_code=0;
		reply_header->cmd_type=request_header->cmd_type;

		char *ptr_reply_temp=reply_buf+sizeof(replyHeader);

		ret=process_template(operate_type, tid, ptr_xml, xml_len);
		if(ret<0) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"process_template error, ret = (%d)!", \
					ret); 
			return -25;
		}

		reply_len=ptr_reply_temp-reply_buf;
		reply_header->length=reply_len-sizeof(PROTOCOL_HEAD_VERSION)-4;
	} else {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"Command type error, cmd_type = (%d)", \
				request_header->cmd_type);
		return -26;
	}

	return reply_len;
}

int32_t QSpiderServer::process_entry(const char* ptr_xml, int32_t xml_len)
{
	Q_CHECK_PTR(ptr_xml);
	Q_ASSERT(xml_len>0, "xml_len must be larger than 0!");

	XMLDocument doc;
	doc.Parse(ptr_xml, xml_len);
	if(doc.Error())
		return -31;

	/* task */
	XMLElement* taskElement=doc.FirstChildElement("task");
	if(taskElement==NULL)
		return -32;

	/* entries */
	XMLElement* entriesElement=taskElement->FirstChildElement("entries");
	if(entriesElement==NULL)
		return -33;

	/* entry */
	XMLElement* entryElement=entriesElement->FirstChildElement("entry");
	int32_t id=0;
	while(entryElement!=NULL)
	{
		struct entryInfo entry_info;
		entry_info.entry_id=id++;

		XMLElement* webElement=entryElement->FirstChildElement("website");
		if(!webElement||!webElement->GetText()) {
			return -34;
		} else {
			strcpy(entry_info.url, webElement->GetText());
			entry_info.website_len=strlen(webElement->GetText());
		}

		XMLElement* typeElement=entryElement->FirstChildElement("type");
		if(!typeElement||!typeElement->GetText())
			return -35;
		else
			entry_info.type=atoi(typeElement->GetText());

		XMLElement* extratypeElement=entryElement->FirstChildElement("extratype");
		if(!extratypeElement||!extratypeElement->GetText())
			return -36;
		else
			entry_info.extratype=atoi(extratypeElement->GetText());


		XMLElement* enameElement=entryElement->FirstChildElement("ename");
		if(!enameElement||!enameElement->GetText()) {
			return -37;
		} else {
			strcpy(entry_info.url, enameElement->GetText());
			entry_info.ename_len=strlen(enameElement->GetText());
		}

		/* url */
		XMLElement* urlElement=entryElement->FirstChildElement("url");
		if(!urlElement||!urlElement->GetText()) {
			return -38;
		} else {
			strcpy(entry_info.url, urlElement->GetText());
			entry_info.url_len=strlen(urlElement->GetText());
		}

		/* param */
		XMLElement* paramElement=entryElement->FirstChildElement("param");
		if(paramElement&&paramElement->GetText()) {
			strcpy(entry_info.param, paramElement->GetText());
			entry_info.param_len=strlen(paramElement->GetText());
		}

		/* tid */
		XMLElement* tidElement=entryElement->FirstChildElement("tid");
		if(!tidElement||!tidElement->GetText())
			return -39;
		else
			entry_info.tid=atoi(tidElement->GetText());

		/* ntid */
		XMLElement* ntidElement=entryElement->FirstChildElement("ntid");
		if(ntidElement&&ntidElement->GetText())
			entry_info.ntid=atoi(ntidElement->GetText());

		/* status */
		XMLElement* statusElement=entryElement->FirstChildElement("status");
		if(statusElement&&statusElement->GetText())
			entry_info.status=atoi(statusElement->GetText());
		else
			entry_info.status=1;

		/* period */
		XMLElement* periodElement=entryElement->FirstChildElement("period");
		if(periodElement&&periodElement->GetText())
			entry_info.period=atoi(periodElement->GetText());
		else
			entry_info.period=0;

		/* proirity */
		XMLElement* priorityElement=entryElement->FirstChildElement("priority");
		if(priorityElement&&priorityElement->GetText())
			entry_info.priority=atoi(priorityElement->GetText());
		else
			entry_info.priority=0;

		if(entry_info.status)
			m_entry_array.push_back(entry_info);

		entryElement=entryElement->NextSiblingElement();
	}

	m_ptr_entry_trigger->signal();
	return 0;
}

int32_t QSpiderServer::process_url(const char* ptr_xml, int32_t xml_len, Downloader* downloader, Parser* parser, \
		int32_t tid, \
		char* ptr_buf, \
		int32_t buf_size, \
		char* ptr_reply, \
		int32_t reply_size)
{
	Q_CHECK_PTR(ptr_xml);
	Q_CHECK_PTR(ptr_buf);
	Q_CHECK_PTR(ptr_reply);
	Q_CHECK_PTR(downloader);
	Q_CHECK_PTR(parser);

	Q_ASSERT(xml_len>0, "xml_len must be larger than 0!");
	Q_ASSERT(buf_size>0, "buf_size must be larger than 0!");
	Q_ASSERT(reply_size>0, "reply_size must be larger than 0!");

	int32_t retry_num=0;
	int32_t ret=0;
	templInfo* tpl_info=NULL;

	ret=m_ptr_tpl_manager->get_template(tid, tpl_info);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"get_template error, ret = (%d)", \
				ret);
		return -41;
	}

	/* download */
	Q_FOREVER
	{
		try {
			struct ServerInfo server_info(m_ptr_cfg_info->download_timeout);

			ret=m_ptr_picker->getServerInfo(server_info);
			if(ret<0) {
				m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
						"ServerPicker::getServerInfo error, ret = (%d)", \
						ret);
				throw -42;
			}

			ret=downloader->setServerInfo(server_info);
			if(ret<0) {
				m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
						"Downloader::setServerInfo error, ret = (%d)", \
						ret);
				throw -43;
			}

			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"Now trying [%d] to download from [%s:%d] ...", \
					++retry_num, \
					server_info.ip, \
					server_info.port);

			ret=downloader->process(ptr_xml, xml_len, ptr_buf, buf_size);
			if(ret<0) {
				m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
						"Downloader::process error, xml = (%.*s), ret = (%d)", \
						xml_len, \
						ptr_xml, \
						ret);
				throw -44;
			}

			break;
		} catch(const int32_t err) {
			if(retry_num>=m_ptr_cfg_info->retry_num_max) {
				m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
						"Downloader: download error, ret = (%d)", \
						err);
				return err;
			}

			q_sleep(m_ptr_cfg_info->retry_interval);
		}
	}

	/* html parse */
	ret=parser->process(ptr_buf, ret, tpl_info, ptr_reply, reply_size);
	if(ret<0) {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"Parser::process error, ret = (%d)", \
				ret);
		return -45;
	}

	return ret;
}

int32_t QSpiderServer::process_template(int32_t type, int32_t tid, const char* ptr_xml, int32_t xml_len)
{
	Q_CHECK_PTR(ptr_xml);
	Q_ASSERT(tid>=0, "tid must be larger than 0!");
	Q_ASSERT(xml_len>0, "xml_len must be larger than 0!");

	int32_t ret=0;
	if(type==1)
	{
		ret=m_ptr_tpl_manager->add_template(tid, ptr_xml, xml_len);
		if(ret<0) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"add_template error, tid = (%d), ret = (%d)", \
					tid, \
					ret);
			return -51;
		}

		m_ptr_logger->log(LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, 1, \
				"add_template success, tid = (%d), ret = (%d)!", \
				tid, \
				ret);
	} else if(type==2) {
		ret=m_ptr_tpl_manager->update_template(tid, ptr_xml, xml_len);
		if(ret<0) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"update_template error, tid = (%d), ret = (%d)", \
					tid, \
					ret);
			return -52;
		}

		m_ptr_logger->log(LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, 1, \
				"update_template success, tid = (%d), ret = (%d)!", \
				tid, \
				ret);
	} else if(type==3) {
		ret=m_ptr_tpl_manager->delete_template(tid);
		if(ret<0) {
			m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"delete_template error, tid = (%d), ret = (%d)", \
					tid, \
					ret);
			return -53;
		}

		m_ptr_logger->log(LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, 1, \
				"delete_template success, tid = (%d), ret = (%d)!", \
				tid, \
				ret);
	} else {
		m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
				"QSpiderServer::process_template: type (%d) is not supported!", \
				type);
		return -54;
	}

	return 0;
}

// 入口扫描线程
Q_THREAD_T QSpiderServer::scan_thread(void* ptr_info)
{
	threadInfo* ptr_trd=reinterpret_cast<threadInfo*>(ptr_info);
	Q_CHECK_PTR(ptr_trd);

	QSpiderServer* ptr_this=reinterpret_cast<QSpiderServer*>(ptr_trd->pthis);
	Q_CHECK_PTR(ptr_this);

	interfaceInfo* pst_interface=reinterpret_cast<interfaceInfo*>(ptr_trd->for_worker);
	Q_CHECK_PTR(pst_interface);

	Downloader* downloader=reinterpret_cast<Downloader*>(pst_interface->ptr_downloader);
	Q_CHECK_PTR(downloader);

	Parser* parser=reinterpret_cast<Parser*>(pst_interface->ptr_parser);
	Q_CHECK_PTR(parser);

	struct taskInfo task_info;
	struct clientInfo client_info;

	bool update_flag=ptr_this->m_ptr_cfg_info->only_update;
	int32_t ret=0;

	int32_t start=SPIDER_DEFAULT_START;
	int32_t finish=SPIDER_DEFAULT_FINISH;
	int32_t step=SPIDER_DEFAULT_STEP;

	int32_t url_num=0;
	int32_t dup_num=0;

	ptr_trd->flag=1;

	while(!ptr_this->m_exit_flag)
	{
		ptr_trd->status=0;

		ptr_this->m_ptr_entry_trigger->wait();

		ptr_trd->status=1;

		do {
			for(int32_t i=0; i!=static_cast<int32_t>(ptr_this->m_entry_array.size()); ++i)
			{
				start=SPIDER_DEFAULT_START;
				finish=SPIDER_DEFAULT_FINISH;
				step=SPIDER_DEFAULT_STEP;

				if(ptr_this->m_entry_array[i].param_len)
					sscanf(ptr_this->m_entry_array[i].param, "%d:%d:%d", &start, &finish, &step);

				if(finish==SPIDER_DEFAULT_FINISH)
					finish=0x7FFFFFFF;

				dup_num=0;

				/* 宽度优先(breadth first) */
				for(int32_t now=start; now>=0&&now<=finish; now+=step)
				{
					ptr_trd->sw.start();

					/* declare */
					task_info.task_id=0;
					task_info.tid=ptr_this->m_entry_array[i].tid;
					task_info.ntid=ptr_this->m_entry_array[i].ntid;

					task_info.url_len=snprintf(task_info.url, SPIDER_URL_SIZE, ptr_this->m_entry_array[i].url, now);
					if(task_info.url_len<0||task_info.url_len>=SPIDER_URL_SIZE)
						continue;

					task_info.type=ptr_this->m_entry_array[i].type;
					task_info.extratype=ptr_this->m_entry_array[i].extratype;

					task_info.method=ptr_this->m_entry_array[i].method;
					task_info.is_cookie=ptr_this->m_entry_array[i].is_cookie;
					task_info.is_proxy=ptr_this->m_entry_array[i].is_proxy;

					/* download */
					client_info.request_buffer=ptr_this->m_ptr_pool_allocator->alloc();
					Q_CHECK_PTR(client_info.request_buffer);

					client_info.reply_buffer=ptr_this->m_ptr_pool_allocator->alloc();
					Q_CHECK_PTR(client_info.reply_buffer);

					try {
						ptr_this->m_ptr_logger->log(LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, 1, \
								"List page (%.*s) is processing now...", \
								task_info.url_len, \
								task_info.url);

						ret=ptr_this->combine_task(&task_info, \
								client_info.request_buffer, \
								client_info.request_buffer_size);
						if(ret<0) {
							ptr_this->m_ptr_logger->log(LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, 1, \
									"List page (%.*s) combine_task error, ret = (%d)!", \
									task_info.url_len, \
									task_info.url, \
									ret);
							throw -1;
						}

						ret=ptr_this->process_url(client_info.request_buffer, \
								ret, \
								downloader, \
								parser, \
								task_info.tid, \
								ptr_trd->ptr_buf, \
								ptr_trd->buf_size, \
								client_info.reply_buffer, \
								client_info.reply_buffer_size);
						if(ret<0) {
							ptr_this->m_ptr_logger->log(LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, 1, \
									"List page (%.*s) process_url error, ret = (%d)!", \
									task_info.url_len, \
									task_info.url, \
									ret);
							throw -2;
						}

						/* parse */
						ret=ptr_this->parse_detail_url(client_info.reply_buffer, ret, &task_info, url_num);
						if(ret<0) {
							ptr_this->m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
									"List page (%.*s) parse_detail_url error, ret = (%d)!", \
									task_info.url_len, \
									task_info.url, \
									ret);
							throw -3;
						} else if(ret==1) {
							++dup_num;
						} else {
							dup_num=0;
						}

						ptr_this->m_ptr_logger->log(LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, 1, \
								"List page (%.*s) process successful!", \
								task_info.url_len, \
								task_info.url);
					} catch(const int32_t err) {
						ptr_this->m_ptr_logger->log(LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, 1, \
								"List page (%.*s) process failed, err = (%d)!", \
								task_info.url_len, \
								task_info.url, \
								err);
					}

					ptr_this->m_ptr_pool_allocator->free(client_info.request_buffer);
					ptr_this->m_ptr_pool_allocator->free(client_info.reply_buffer);

					ptr_trd->sw.stop();

					if(update_flag) {
						q_sleep(ptr_this->m_ptr_cfg_info->update_interval);
					} else {
						q_sleep(ptr_this->m_ptr_cfg_info->download_interval);
					}

					/* last page or duplicated page when updating */
					if((url_num==0) || (dup_num>=100) || (ptr_this->m_ptr_cfg_info->auto_update && update_flag && ret==1))
						break;
				}
			}

			/* update next round */
			update_flag=true;
		} while(ptr_this->m_ptr_cfg_info->auto_update);
	}

	ptr_trd->flag=-1;
	return NULL;
}

// 抽取线程
Q_THREAD_T QSpiderServer::extract_thread(void* ptr_info)
{
	threadInfo* ptr_trd=reinterpret_cast<threadInfo*>(ptr_info);
	Q_CHECK_PTR(ptr_trd);

	QSpiderServer* ptr_this=reinterpret_cast<QSpiderServer*>(ptr_trd->pthis);
	Q_CHECK_PTR(ptr_this);

	interfaceInfo* pst_interface=reinterpret_cast<interfaceInfo*>(ptr_trd->for_worker);
	Q_CHECK_PTR(pst_interface);

	Downloader* downloader=reinterpret_cast<Downloader*>(pst_interface->ptr_downloader);
	Q_CHECK_PTR(downloader);

	Parser* parser=reinterpret_cast<Parser*>(pst_interface->ptr_parser);
	Q_CHECK_PTR(parser);

	struct taskInfo task_info;
	struct clientInfo client_info;
	int32_t ret=0;

	ptr_trd->flag=1;

	while(!ptr_this->m_exit_flag)
	{
		ptr_trd->status=0;

		ptr_this->m_ptr_dp_trigger->wait();

		while(ptr_this->m_ptr_dp_queue->pop_non_blocking(task_info)==0)
		{
			ptr_trd->status=1;
			ptr_trd->sw.start();

			client_info.request_buffer=ptr_this->m_ptr_pool_allocator->alloc();
			Q_CHECK_PTR(client_info.request_buffer);

			client_info.reply_buffer=ptr_this->m_ptr_pool_allocator->alloc();
			Q_CHECK_PTR(client_info.reply_buffer);

			try {
				ptr_this->m_ptr_logger->log(LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, 1, \
						"Detail page (%.*s) is processing now...", \
						task_info.url_len, \
						task_info.url);

				/* combine url */
				ret=ptr_this->combine_task(&task_info, \
						client_info.request_buffer, \
						client_info.request_buffer_size);
				if(ret<0) {
					ptr_this->m_ptr_logger->log(LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, 1, \
							"Detail page (%.*s) combine_task error, ret = (%d)!", \
							task_info.url_len, \
							task_info.url, \
							ret);
					throw -1;
				}

				ret=ptr_this->process_url(client_info.request_buffer, \
						ret, \
						downloader, \
						parser, \
						task_info.tid, \
						ptr_trd->ptr_buf, \
						ptr_trd->buf_size, \
						client_info.reply_buffer, \
						client_info.reply_buffer_size);
				if(ret<0) {
					ptr_this->m_ptr_logger->log(LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, 1, \
							"Detail page (%.*s) process_url error, ret = (%d)!", \
							task_info.url_len, \
							task_info.url, \
							ret);
					throw -2;
				}

				ret=ptr_this->pack_data(client_info.reply_buffer, ret, ptr_trd->ptr_buf, ptr_trd->buf_size);
				if(ret<0) {
					ptr_this->m_ptr_logger->log(LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, 1, \
							"Detail page (%.*s) pack data error, ret = (%d)!", \
							task_info.url_len, \
							task_info.url, \
							ret);
					throw -3;
				}

				ret=ptr_this->write_data_file(ptr_this->m_ptr_cfg_info->send_write_path, \
						ptr_this->m_send_write_fp, \
						ptr_trd->ptr_buf, \
						ret);
				if(ret<0) {
					ptr_this->m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
							"Detail page (%.*s) write_data_file error, ret = (%d)!", \
							task_info.url_len, \
							task_info.url, \
							ret);
					throw -4;
				}

				ptr_this->m_ptr_logger->log(LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, 1, \
						"Detail page (%.*s) process successful!", \
						task_info.url_len, \
						task_info.url);
			} catch(const int32_t err) {
				ptr_this->m_ptr_logger->log(LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, 1, \
						"Detail page (%.*s) process failed, err = (%d)!", \
						task_info.url_len, \
						task_info.url, \
						err);
			}

			ptr_this->m_ptr_pool_allocator->free(client_info.request_buffer);
			ptr_this->m_ptr_pool_allocator->free(client_info.reply_buffer);

			ptr_trd->sw.stop();

			q_sleep(ptr_this->m_ptr_cfg_info->download_interval);
		}
	}

	ptr_trd->flag=-1;
	return NULL;
}

// 发送线程
Q_THREAD_T QSpiderServer::send_thread(void* ptr_info)
{
	threadInfo* ptr_trd=reinterpret_cast<threadInfo*>(ptr_info);
	Q_CHECK_PTR(ptr_trd);

	QSpiderServer* ptr_this=reinterpret_cast<QSpiderServer*>(ptr_trd->pthis);
	Q_CHECK_PTR(ptr_this);

	replyHeader reply_header;
	landHeader land_header;

	Q_SOCKET_T sock_client=SPIDER_INVALID_SOCKET;
	uint64_t end_mark=0;

	uint64_t file_size=0;
	uint64_t current_size=0;

	int32_t ret=0;

	ptr_trd->flag=1;

	while(!ptr_this->m_exit_flag)
	{
		ptr_trd->status=0;

		if(!QFile::exists(ptr_this->m_ptr_cfg_info->send_read_path))
		{
			if(!QFile::exists(ptr_this->m_ptr_cfg_info->send_write_path)) {
				q_sleep(1000);
				continue;
			}

			ptr_this->m_send_file_mutex.lock();
			if(ptr_this->m_send_write_fp) {
				::fclose(ptr_this->m_send_write_fp);
				ptr_this->m_send_write_fp=NULL;
			}

			QFile::rename(ptr_this->m_ptr_cfg_info->send_write_path, ptr_this->m_ptr_cfg_info->send_read_path);
			ptr_this->m_send_file_mutex.unlock();
		}

		FILE *fp_r=NULL;
		do {
			fp_r=::fopen(ptr_this->m_ptr_cfg_info->send_read_path, "rb");
			if(fp_r)
				break;
			q_sleep(1000);
		} while(fp_r==NULL);

		ptr_trd->status=1;

		file_size=QFile::size(ptr_this->m_ptr_cfg_info->send_read_path);
		current_size=0;

		try {
			while(current_size<file_size) {
				ptr_trd->sw.start();

				if(fread(&land_header, sizeof(landHeader), 1, fp_r)!=1)
					throw -1;

				if(land_header.version!=PROTOCOL_HEAD_VERSION)
					throw -2;

				if(land_header.length>ptr_trd->buf_size)
					throw -3;

				if(fread(ptr_trd->ptr_buf, land_header.length, 1, fp_r)!=1)
					throw -4;

				if(fread(&end_mark, sizeof(end_mark), 1, fp_r)!=1)
					throw -5;

				if(end_mark!=ptr_this->I_SEND_FILE_MARK)
					throw -6;

				current_size+=sizeof(landHeader)+land_header.length+sizeof(end_mark);

				ptr_trd->sw.stop();

				for(;;) {
					ptr_trd->sw.start();

					ret=q_connect_socket(sock_client, ptr_this->m_ptr_cfg_info->send_ip, ptr_this->m_ptr_cfg_info->send_port);
					if(ret!=0) {
						Q_INFO("TCP socket [%s:%d] connection......", \
								ptr_this->m_ptr_cfg_info->send_ip, \
								ptr_this->m_ptr_cfg_info->send_port);
						q_sleep(1000);
						continue;
					}

					ret=q_set_overtime(sock_client, ptr_this->m_ptr_cfg_info->send_thread_timeout);
					if(ret!=0) {
						Q_INFO("TCP socket [%s:%d] set overtime......", \
								ptr_this->m_ptr_cfg_info->send_ip, \
								ptr_this->m_ptr_cfg_info->send_port);
						q_close_socket(sock_client);
						q_sleep(1000);
						continue;
					}

					ret=q_sendbuf(sock_client, ptr_trd->ptr_buf, land_header.length);
					if(ret!=0) {
						Q_INFO("TCP socket [%s:%d] send......", \
								ptr_this->m_ptr_cfg_info->send_ip, \
								ptr_this->m_ptr_cfg_info->send_port);
						q_close_socket(sock_client);
						q_sleep(1000);
						continue;
					}

					ret=q_recvbuf(sock_client, (char*)(&reply_header), sizeof(replyHeader));
					if(ret!=0) {
						Q_INFO("TCP socket [%s:%d] recv......", \
								ptr_this->m_ptr_cfg_info->send_ip, \
								ptr_this->m_ptr_cfg_info->send_port);
						q_close_socket(sock_client);
						q_sleep(1000);
						continue;
					}

					if(reply_header.version!=PROTOCOL_HEAD_VERSION) {
						Q_INFO("TCP socket [%s:%d] version = (%lu)", \
								ptr_this->m_ptr_cfg_info->send_ip, \
								ptr_this->m_ptr_cfg_info->send_port, \
								reply_header.version);
						q_close_socket(sock_client);
						q_sleep(1000);
						continue;
					}

					if(reply_header.length<=0) {
						Q_INFO("TCP socket [%s:%d] length = (%d)", \
								ptr_this->m_ptr_cfg_info->send_ip, \
								ptr_this->m_ptr_cfg_info->send_port, \
								reply_header.length);
						q_close_socket(sock_client);
						q_sleep(1000);
						continue;
					}

					if(reply_header.status_code!=0) {
						Q_INFO("TCP socket [%s:%d] status code = (%d)", \
								ptr_this->m_ptr_cfg_info->send_ip, \
								ptr_this->m_ptr_cfg_info->send_port, \
								reply_header.status_code);
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

			ptr_this->backup_file(ptr_this->m_ptr_cfg_info->send_read_path, ptr_trd->ptr_buf, ptr_trd->buf_size);
			QFile::remove(ptr_this->m_ptr_cfg_info->send_read_path);
		} catch(const int32_t err) {
			fclose(fp_r);
			fp_r=NULL;

			char str_file[1<<5]={0};
			q_snprintf(str_file, sizeof(str_file), "%s.%ld", ptr_this->m_ptr_cfg_info->send_read_path, time(NULL));
			QFile::rename(ptr_this->m_ptr_cfg_info->send_read_path, str_file);

			ptr_this->m_ptr_logger->log(LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, 1, \
					"send file (%s) error, err = (%d)", \
					ptr_this->m_ptr_cfg_info->send_read_path, err);
		}
	}

	ptr_trd->flag=-1;
	return NULL;
}

int32_t QSpiderServer::combine_task(struct taskInfo* task_info, char* ptr_buf, int32_t buf_size)
{
	if(task_info==NULL||ptr_buf==NULL||buf_size<=0)
		return -1;

	int32_t ret=0;
	ret=snprintf(ptr_buf, \
			buf_size, \
			"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" \
			"<doc>\r\n" \
			"<task method=\"%d\" cookie=\"%d\" proxy=\"%d\">\r\n" \
			"<type><![CDATA[%d]]></type>\r\n" \
			"<extratype><![CDATA[%d]]></extratype>\r\n" \
			"<srclink><![CDATA[%.*s]]></srclink>\r\n" \
			"</task>\r\n" \
			"</doc>\r\n", \
			task_info->method, \
			task_info->is_cookie, \
			task_info->is_proxy, \
			task_info->type, \
			task_info->extratype, \
			task_info->url_len, \
			task_info->url);
	if(ret<0||ret>=buf_size)
		return -2;
	return ret;
}

int32_t QSpiderServer::parse_detail_url(const char* ptr_xml, int32_t xml_len, struct taskInfo* list_task, int32_t& url_num)
{
	if(ptr_xml==NULL||xml_len<=0||list_task==NULL)
		return -1;

	/* detail url */
	struct taskInfo detail_task;
	uint64_t url_key=0;
	int32_t ret=0;

	XMLDocument doc;
	doc.Parse(ptr_xml, xml_len);
	if(doc.Error())
		return -2;

	XMLElement* docElement=doc.FirstChildElement("doc");
	if(docElement==NULL)
		return -3;

	XMLElement* listsElement=docElement->FirstChildElement("lists");
	if(listsElement==NULL)
		return -4;

	url_num=0;

	XMLElement* listElement=listsElement->FirstChildElement("list");
	while(listElement!=NULL)
	{
		XMLElement* urlElement=listElement->FirstChildElement("url");
		if(!urlElement||!urlElement->GetText()) {
			++url_num;
			listElement=listElement->NextSiblingElement();
			continue;
		}

		detail_task.task_id=0;
		detail_task.tid=list_task->ntid;
		strcpy(detail_task.url, urlElement->GetText());
		detail_task.url_len=strlen(detail_task.url);

		detail_task.method=list_task->method;
		detail_task.is_cookie=list_task->is_cookie;
		detail_task.is_proxy=list_task->is_proxy;

		detail_task.type=list_task->type;
		detail_task.extratype=list_task->extratype;

#if defined (__URL_UNIQ)
		url_key=make_md5(detail_task.url, detail_task.url_len);

		/* duplicated url */
		if(m_ptr_uniq_cache->searchKey_FL(url_key)==0)
		{
			ret=1;

			++url_num;
			listElement=listElement->NextSiblingElement();
			continue;
		} else {
			if(m_ptr_uniq_cache->addKey_FL(url_key))
				return -5;

			m_ptr_dp_queue->push(detail_task);
			m_ptr_dp_trigger->signal();

			++url_num;
			listElement=listElement->NextSiblingElement();
		}
#else
		m_ptr_dp_queue->push(detail_task);
		m_ptr_dp_trigger->signal();

		++url_num;
		listElement=listElement->NextSiblingElement();
#endif
	}

	return ret;
}

int32_t QSpiderServer::pack_data(const char* ptr_xml, int32_t xml_len, char* ptr_buf, int32_t buf_size)
{
	if(ptr_xml==NULL||xml_len<=0||ptr_buf==NULL||buf_size<=0)
		return -1;

	char* ptr_temp=ptr_buf;
	char* ptr_end=ptr_buf+buf_size;
	int32_t reply_len=0;

	requestHeader* request_header=reinterpret_cast<requestHeader*>(ptr_temp);
	request_header->version=PROTOCOL_HEAD_VERSION;
	request_header->protocol_type=1;
	request_header->source_type=1;
	request_header->cmd_type=1;

	ptr_temp+=sizeof(requestHeader);

	*(uint32_t*)ptr_temp=xml_len;
	ptr_temp+=sizeof(uint32_t);

	if(ptr_temp+xml_len>ptr_end)
		return -2;

	memcpy(ptr_temp, ptr_xml, xml_len);
	ptr_temp+=xml_len;

	reply_len=ptrdiff_t(ptr_temp-ptr_buf);
	request_header->length=reply_len-sizeof(uint64_t)-sizeof(uint32_t);

	return reply_len;
}

int32_t QSpiderServer::write_data_file(const char* ptr_file, FILE*& fp_w, const char* ptr_buf, int32_t buf_len)
{
	int32_t ret=0;
	int64_t offset=0;

	landHeader land_header;
	land_header.version=PROTOCOL_HEAD_VERSION;
	land_header.length=buf_len;

	m_send_file_mutex.lock();

	while(fp_w==NULL) {
		fp_w=::fopen(ptr_file, "wb");
		if(NULL!=fp_w)
			break;
		q_sleep(1000);
	}

	offset=::ftell(fp_w);

	try {
		if(fwrite(&land_header, sizeof(landHeader), 1, fp_w)!=1)
			throw -2;

		if(fwrite(ptr_buf, buf_len, 1, fp_w)!=1)
			throw -3;

		if(fwrite(&I_SEND_FILE_MARK, sizeof(I_SEND_FILE_MARK), 1, fp_w)!=1)
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

	m_send_file_mutex.unlock();

	return ret;
}

int32_t QSpiderServer::backup_file(const char* ptr_file, char* ptr_buf, int32_t buf_size)
{
	if(ptr_file==NULL||ptr_buf==NULL||buf_size<=0)
		return -1;

	time_t now=time(NULL);
	struct tm* ptm=localtime(&now);

	char bak_name[1<<7]={0};
	q_snprintf(bak_name, sizeof(bak_name)-1, "%s/%04d-%02d-%02d.snd", m_ptr_cfg_info->send_bak_path, ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday);

	FILE* fp_r=::fopen(ptr_file, "rb");
	if(fp_r==NULL)
		return -2;

	FILE* fp_w=::fopen(bak_name, "ab");
	if(fp_w==NULL) {
		fclose(fp_r);
		fp_r=NULL;
		return -3;
	}

	landHeader land_header;
	uint64_t end_mark=0;

	uint64_t file_size=QFile::size(ptr_file);
	uint64_t current_size=0;

	int32_t ret=0;
	try {
		while(current_size<file_size) {
			if(::fread(&land_header, sizeof(landHeader), 1, fp_r)!=1)
				throw -4;

			if(land_header.length>buf_size)
				throw -5;

			if(::fread(ptr_buf, land_header.length, 1, fp_r)!=1)
				throw -6;

			if(::fread(&end_mark, sizeof(end_mark), 1, fp_r)!=1)
				throw -7;

			if(end_mark!=I_SEND_FILE_MARK)
				throw -8;

			if(::fwrite(&land_header, sizeof(landHeader), 1, fp_w)!=1)
				throw -9;

			if(::fwrite(ptr_buf, land_header.length, 1, fp_w)!=1)
				throw -10;

			if(::fwrite(&end_mark, sizeof(end_mark), 1, fp_w)!=1)
				throw -11;

			current_size+=sizeof(landHeader)+land_header.length+sizeof(end_mark);
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

int32_t QSpiderServer::get_thread_state(void* ptr_info)
{
	QSpiderServer* ptr_this=reinterpret_cast<QSpiderServer*>(ptr_info);
	Q_CHECK_PTR(ptr_this);

	int32_t timeout_thread_num=0;
	for(int32_t i=0; i<ptr_this->m_thread_max; ++i)
	{
		if(ptr_this->m_ptr_trd_info[i].status!=1)
			continue;

		ptr_this->m_ptr_trd_info[i].sw.stop();

		if(ptr_this->m_ptr_trd_info[i].sw.elapsed_ms()>ptr_this->m_ptr_trd_info[i].timeout)
			++timeout_thread_num;
	}

	return timeout_thread_num;
}

