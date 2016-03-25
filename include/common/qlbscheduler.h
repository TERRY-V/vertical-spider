/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qlbscheduler.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2015/01/15
**
*********************************************************************************************/

#ifndef __QLBSCHEDULER_H_
#define __QLBSCHEDULER_H_

#include "qglobal.h"
#include "qfunc.h"

Q_BEGIN_NAMESPACE

// 集群服务器信息
struct serverInfo {
	std::string	addr;		// 集群服务器地址
	int32_t 	weight;		// 集群服务器的综合权重(0该服务器已挂掉)

	serverInfo() :
		weight(1)
	{}
};

// 连接调度算法
enum scheduling {
	RoundRobin,			// 轮询调度算法
	WeightedRoundRobin,		// 加权轮询调度算法
	LeastConnection,		// 最小连接调度算法
	WeightedLeastConnection,	// 加权最小连接调度算法
	LocalityBasedLeastConnection,	// 基于局部性的最少链接调度算法
	DestinationHashing,		// 目标地址散列调度算法
	SourceHashing			// 源地址散列调度算法
};

// 负载均衡调度器
class QLBScheduler: public noncopyable {
	public:
		// @函数名: 构造函数
		QLBScheduler();

		// @函数名: 析构函数
		virtual ~QLBScheduler();

		// @函数名: 初始化函数
		int32_t init(const char* cfg_file);

		// @函数名: 获取服务器
		int32_t get_server(struct serverInfo* server = NULL, scheduling sched = WeightedRoundRobin, uint64_t hash_key = 0);

		// @函数名: 追踪函数
		void trace();

	private:
		// @函数名: 轮询调度算法(容易导致服务器间负载不平衡)
		int32_t get_server_RR();

		// @函数名: 加权轮询调度算法
		int32_t get_server_WRR();

		// @函数名: 目标地址散列调度算法
		int32_t get_server_DH(uint64_t hash_key);

		// @函数名: 源地址散列调度算法
		int32_t get_server_SH(uint64_t hash_key);

		// @函数名: 右trim函数
		int32_t cancelEnter(char* buf, int32_t buf_len);

		// @函数名: 计算最大权值
		int32_t max_weight();

		// @函数名: 计算最小权值
		int32_t min_weight();

		// @函数名: 计算最大公约数
		int32_t gcd();

	protected:
		std::vector<serverInfo>	server_vec_;		// 集群服务器列表信息
		int32_t			server_iter_;		// 服务器指示变量
		int32_t			current_weight_;	// 当前调度的权值
		QMutexLock		mutex_;			// 调度算法用互斥锁
};

Q_END_NAMESPACE

#endif // __QLBSCHEDULER_H_
