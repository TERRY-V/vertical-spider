#include "qremotemonitor.h"

Q_BEGIN_NAMESPACE

QRemoteMonitor::QRemoteMonitor() :
	listen_sock_(-1),
	timeout_(8000),
	fun_argv_(NULL),
	success_flag_(0),
	display_log_(1)
{
}

QRemoteMonitor::~QRemoteMonitor()
{
	q_close_socket(listen_sock_);
}

int32_t QRemoteMonitor::init(uint16_t monitor_port, int32_t timeout, int32_t (*fun_state)(void* argv), void* fun_argv, int32_t display_log)
{
	this->monitor_port_ = monitor_port;
	this->timeout_ = timeout;

	this->fun_state_ = fun_state;
	this->fun_argv_ = fun_argv;
	this->display_log_ = display_log;

	if(q_init_socket()) {
		Q_INFO("QRemoteMonitor: socket init error!");
		return MONITOR_ERR;
	}

	if(q_TCP_server(listen_sock_, monitor_port_)) {
		Q_INFO("QRemoteMonitor: socket listen error!");
		return MONITOR_ERR;
	}

	if(q_create_thread(thread_monitor, this)) {
		Q_INFO("QRemoteMonitor: create thread error!");
		return MONITOR_ERR;
	}

	while(success_flag_==0)
		q_sleep(1);

	if(success_flag_<0) {
		Q_INFO("QRemoteMonitor: start thread error!");
		return MONITOR_ERR;
	}

	return MONITOR_OK;
}

Q_THREAD_T QRemoteMonitor::thread_monitor(void* ptr_info)
{
	QRemoteMonitor* ptr_this=reinterpret_cast<QRemoteMonitor*>(ptr_info);
	Q_CHECK_PTR(ptr_this);

	Q_SOCKET_T client;
	char client_ip[16] = {0};
	int32_t client_port;

	uint32_t cmd = 0;
	serverInfo server_info;

	ptr_this->success_flag_ = 1;

	for(;;) {
		// 接收客户端请求
		if(q_accept_socket(ptr_this->listen_sock_, client, client_ip, client_port)) {
			Q_INFO("QRemoteMonitor: accept request error!");
			continue;
		}

		try {
			// 设置请求超时时间
			if(q_set_overtime(client, ptr_this->timeout_)) {
				Q_INFO("QRemoteMonitor: set overtime error!");
				throw -1;
			}

			// 接收请求报文信息
			if(q_recvbuf(client, (char*)&cmd, sizeof(uint32_t)) || cmd!=*(uint32_t *)"PING") {
				Q_INFO("QRemoteMonitor: recv data error, magic mark = (%.*s)!", sizeof(uint32_t), (char*)&cmd);
				throw -2;
			}

			/******************************************************************************************/
			server_info.magic_mark = *(uint32_t*)"PONG";
			server_info.length = sizeof(serverInfo)-sizeof(int32_t);

			if(ptr_this->fun_state_(ptr_this->fun_argv_)==0) {
				server_info.error_type = TYPE_OK;
				server_info.error_level = LEVEL_OK;
			} else {
				server_info.error_type = TYPE_SERVICE;
				server_info.error_level = LEVEL_A;
			}

			server_info.load_average = q_get_load_avg();
			server_info.processor_num = q_get_cpu_processors();

			if(q_get_disk_usage("./", &server_info.used_disk_bytes, &server_info.total_disk_bytes)) {
				Q_INFO("QRemoteMonitor: q_get_disk_usage error!");
				throw -3;
			}

			if(q_get_mem_usage(&server_info.used_mem_bytes, &server_info.total_mem_bytes)) {
				Q_INFO("QRemoteMonitor: q_get_disk_usage error!");
				throw -4;
			}
			/******************************************************************************************/

			// 发送响应报文信息
			if(q_sendbuf(client, (char*)&server_info, sizeof(serverInfo))) {
				Q_INFO("QRemoteMonitor: send buffer error!");
				throw -5;
			}

			q_close_socket(client);

			if(ptr_this->display_log_) {
				Q_INFO("QRemoteMonitor: [%s:%05d] server status (%d), error level (%d), "
						"load average(%d), " \
						"processor num(%d), " \
						"disk bytes usage(%llu/%llu), " \
						"memory bytes usage(%llu/%llu)", \
						client_ip, \
						client_port, \
						server_info.error_type, \
						server_info.error_level, \
						server_info.load_average, \
						server_info.processor_num, \
						server_info.used_disk_bytes, \
						server_info.total_disk_bytes, \
						server_info.used_mem_bytes, \
						server_info.total_mem_bytes);
			}
		} catch(const int32_t errid) {
			q_close_socket(client);
		}
	}

	ptr_this->success_flag_ = -1;
	return NULL;
}

Q_END_NAMESPACE
