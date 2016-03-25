#include "qlbscheduler.h"

Q_BEGIN_NAMESPACE

QLBScheduler::QLBScheduler() :
	server_iter_(0),
	current_weight_(0)
{
}

QLBScheduler::~QLBScheduler()
{
}

int32_t QLBScheduler::init(const char* cfg_file)
{
	FILE* fp=fopen(cfg_file, "r");
	if(fp==NULL) {
		Q_INFO("QLBScheduler: open config file (%s) error!", cfg_file);
		return -1;
	}

	char buf[1024]={0};
	while(fgets(buf, sizeof(buf), fp))
	{
		if(strncmp(buf, "#", 1)==0 || strncmp(buf, "//", 2)==0)
			continue;

		uint32_t buf_len=cancelEnter(buf, strlen(buf));
		if(buf_len<=0)
			continue;

		std::vector<std::string> params;
		q_split_any(std::string(buf, buf_len), " \t", params);
		if(params.size()!=2)
			continue;

		struct serverInfo server_info;
		server_info.addr=params.at(0);
		server_info.weight=atoi(params.at(1).c_str());

		server_vec_.push_back(server_info);
	}

	return 0;
}

int32_t QLBScheduler::get_server(struct serverInfo* server, scheduling sched, uint64_t hash_key)
{
	if(server_vec_.size()==0)
		return -1;

	int32_t server_index(-1);
	switch(sched)
	{
		case RoundRobin:
			server_index=get_server_RR();
			break;
		case WeightedRoundRobin:
			server_index=get_server_WRR();
			break;
		case DestinationHashing:
			server_index=get_server_DH(hash_key);
			break;
		case SourceHashing:
			server_index=get_server_SH(hash_key);
			break;
		default:
			break;
	}

	if(server_index>=0 && server!=NULL)
		*server=server_vec_[server_index];

	return server_index;
}

void QLBScheduler::trace()
{
	Q_INFO("Server list:");
	for(uint32_t i=0; i<server_vec_.size(); ++i)
	{
		Q_INFO("Server [%02d]: addr = (%s), weight = (%d)!", \
				i, \
				server_vec_[i].addr.c_str(), \
				server_vec_[i].weight);
	}
}

int32_t QLBScheduler::get_server_RR()
{
	mutex_.lock();

	int32_t j=server_iter_;
	do {
		j=(j+1)%server_vec_.size();
		if(server_vec_[j].weight>0) {
			server_iter_=j;
			mutex_.unlock();
			return j;
		}
	} while(j!=server_iter_);

	mutex_.unlock();
	return -1;
}

int32_t QLBScheduler::get_server_WRR()
{
	mutex_.lock();

	int32_t j=server_iter_;
	for(;;) {
		j=(j+1)%server_vec_.size();
		if(j==0) {
			current_weight_-=gcd();
			if(current_weight_<=0) {
				current_weight_=max_weight();
				if(current_weight_==0) {
					mutex_.unlock();
					return -1;
				}
			}
		}

		if(server_vec_[j].weight>=current_weight_) {
			server_iter_=j;
			mutex_.unlock();
			return j;
		}
	}

	mutex_.unlock();
	return -1;
}

int32_t QLBScheduler::get_server_DH(uint64_t hash_key)
{
	int32_t i=hash_key%server_vec_.size();
	int32_t j=i;

	do {
		if(server_vec_[j].weight>0)
			return j;
		else
			++j;
	} while(j!=i);

	return -1;
}

int32_t QLBScheduler::get_server_SH(uint64_t hash_key)
{
	int32_t i=hash_key%server_vec_.size();
	int32_t j=i;

	do {
		if(server_vec_[j].weight>0)
			return j;
		else
			++j;
	} while(j!=i);

	return -1;
}

int32_t QLBScheduler::cancelEnter(char* buf, int32_t buf_len)
{
	if(NULL==buf||buf_len<=0)
		return 0;

	while(buf_len>0&&(buf[buf_len-1]==0x20||buf[buf_len-1]==0x0D||buf[buf_len-1]==0x0A))
		buf_len--;
	buf[buf_len]=0;
	return buf_len;
}

int32_t QLBScheduler::max_weight()
{
	int32_t weight=0;
	for(uint32_t i=1; i<server_vec_.size(); ++i)
	{
		if(weight<server_vec_[i].weight)
			weight=server_vec_[i].weight;
	}
	return weight;
}

int32_t QLBScheduler::min_weight()
{
	int32_t weight=server_vec_[0].weight;
	for(uint32_t i=1; i<server_vec_.size(); ++i)
	{
		if(weight>server_vec_[i].weight)
			weight=server_vec_[i].weight;
	}
	return weight;
}

int32_t QLBScheduler::gcd()
{
	int32_t min=min_weight();
	bool flag(false);

	while(min>=1)
	{
		flag=true;
		for(uint32_t i=0; i<server_vec_.size(); ++i) {
			if(server_vec_[i].weight%min!=0) {
				flag=false;
				break;
			}
		}
		if(flag) break;
		min--;
	}

	return min;
}

Q_END_NAMESPACE
