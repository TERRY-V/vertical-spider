/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qremotemonitor.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/06/01
**
*********************************************************************************************/

#ifndef __QREMOTEMONITOR_H_
#define __QREMOTEMONITOR_H_

#include "qglobal.h"

#define MONITOR_OK     (0)
#define MONITOR_ERR    (-1)

Q_BEGIN_NAMESPACE

// 错误类型
enum errorType {
	TYPE_OK = 0,				// 运行正常
	TYPE_MONITOR,				// monitor错误
	TYPE_NETWORK,				// 网络错误
	TYPE_SERVICE,				// 服务错误
	TYPE_OTHER				// 其它错误
};

// 错误级别
enum errorLevel {
	LEVEL_OK = 0,				// 正常
	LEVEL_A,				// 严重
	LEVEL_B,				// 重要
	LEVEL_C,				// 一般
	LEVEL_D					// 可忽略
};

class QRemoteMonitor : public noncopyable {
	public:
#pragma pack(1)
		struct serverInfo {
			uint32_t     magic_mark;		// 魔数
			int32_t	     length;			// 后续数据总长度
			int32_t	     error_type;		// 错误类型
			int32_t	     error_level;		// 错误级别
			uint32_t     load_average;		// 负载均值
			uint8_t	     processor_num;		// 处理器数量
			uint64_t     used_disk_bytes;		// 已使用磁盘空间
			uint64_t     total_disk_bytes;		// 总计磁盘空间
			uint64_t     used_mem_bytes;		// 已使用内存信息
			uint64_t     total_mem_bytes;		// 总计内存信息
		};
#pragma pack()

		// @函数名: 构造函数
		QRemoteMonitor();

		// @函数名: 析构函数
		virtual ~QRemoteMonitor();

		// @函数名: 初始化函数
		int32_t init(uint16_t monitor_port, int32_t timeout, int32_t (*fun_state)(void* argv), void* fun_argv, int32_t display_log = 1);

	private:
		// @函数名: 监控线程
		static Q_THREAD_T thread_monitor(void* ptr_info);

	protected:
		Q_SOCKET_T	listen_sock_;
		uint16_t	monitor_port_;
		int32_t		timeout_;
		int32_t		(*fun_state_)(void* argv);
		void*		fun_argv_;
		int32_t		success_flag_;
		int32_t		display_log_;
};

Q_END_NAMESPACE

#endif // __QREMOTEMONITOR_H_
