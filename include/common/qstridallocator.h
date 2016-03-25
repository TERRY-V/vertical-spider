/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qstridallocator.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/05/23
**
*********************************************************************************************/

#ifndef __QSTRIDALLOCATOR_H_
#define __QSTRIDALLOCATOR_H_

#include "qglobal.h"
#include "qhashsearch.h"
#include "qallocator.h"
#include "qmd5.h"

Q_BEGIN_NAMESPACE

class QStrIDAllocator {
	public:
		QStrIDAllocator()
		{}

		virtual ~QStrIDAllocator()
		{}

		// @函数名: 初始化函数
		// @参数01: 允许的最多标签个数
		// @参数02: 数据文件路径
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t init(uint32_t string_max, const char* rdb_file)
		{
			if(string_max==0||rdb_file==NULL)
				return -1;

			string_max_=string_max;
			strcpy(rdb_file_, rdb_file);

			is_map_=q_new_array<char*>(string_max_);
			if(is_map_==NULL)
				return -2;
			memset(is_map_, 0, string_max_*sizeof(char*));

			int32_t bucket_size=(string_max_/2)+1;
			if(si_map_.init(bucket_size, sizeof(void*)))
				return -3;

			rdb_length_=0;
			string_now_=0;

			if(access(rdb_file_, 0)) {
				rdb_fp_=fopen(rdb_file_, "wb+");
				if(rdb_fp_==NULL)
					return -4;
				return 0;
			}

			rdb_fp_=fopen(rdb_file_, "rb+");
			if(rdb_fp_==NULL)
				return -5;

			char read_buf[16];
			while(1) {
				if(fread(read_buf, sizeof(read_buf), 1, rdb_fp_)!=1)
					break;

				int32_t size=16+*(int32_t*)(read_buf+12)+8;
				char* ptr=allocator_.alloc(size);
				if(ptr==NULL)
					return -6;

				memcpy(ptr, read_buf, 16);

				if(fread(ptr+16, *(int32_t*)(ptr+12)+8, 1, rdb_fp_)!=1)
					return -7;

				assert(string_now_==*(uint32_t*)ptr);

				if(si_map_.addKey_FL(*(uint64_t*)(ptr+4), &ptr)<0)
					return -8;

				is_map_[string_now_++]=ptr;
				rdb_length_+=size;
			}

			assert(rdb_length_==ftell(rdb_fp_));

			return 0;
		}

		// @函数名: 通过字符串获取映射ID
		// @参数01: 字符串指针
		// @参数02: 字符串长度
		// @参数03: 映射ID
		// @返回值: 成功返回0, 失败返回<0的错误码, 重复返回1
		int32_t str2id(const char* str, uint32_t str_len, uint32_t& id)
		{
			if(str==NULL||str_len==0)
				return -1;

			uint64_t md5=md5_.MD5Bits64((unsigned char*)str, str_len);

			mutex_.lock();

			void* val=NULL;
			if(si_map_.searchKey_FL(md5, &val)==0) {
				id=**(uint32_t**)val;
				mutex_.unlock();
				return 1;
			}

			if(string_now_>=string_max_) {
				mutex_.unlock();
				return -2;
			}

			uint32_t size=4+8+4+str_len+8;	// id, md5, len, str, flg

			char* buf=allocator_.alloc(size);
			if(buf==NULL) {
				mutex_.unlock();
				return -3;
			}

			*(uint32_t*)buf=string_now_;
			*(uint64_t*)(buf+4)=md5;
			*(uint32_t*)(buf+12)=str_len;
			memcpy(buf+16, str, str_len);
			*(uint64_t*)(buf+size-8)=*(uint64_t*)"!@#$$#@!";

			if(fwrite(buf, size, 1, rdb_fp_)!=1) {
				mutex_.unlock();
				return -4;
			}

			if(fflush(rdb_fp_)) {
				mutex_.unlock();
				return -5;
			}

			if(si_map_.addKey_FL(md5, (void*)&buf)) {
				mutex_.unlock();
				return -6;
			}

			rdb_length_+=size;
			is_map_[string_now_]=buf;
			id=string_now_++;
			mutex_.unlock();

			return 0;
		}

		// @函数名: 通过ID获取映射字符串
		// @参数01: ID号
		// @参数02: 字符串指针
		// @参数03: 字符串长度
		// @参数04: md5值
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t id2str(uint32_t id, const char*& str, uint32_t& str_len, uint64_t& md5)
		{
			if(id>=string_now_)
				return -1;
			
			char* buf=is_map_[id];
			if(buf==NULL)
				return -2;

			assert(id==*(uint32_t*)buf);

			md5=*(uint64_t*)(buf+4);
			str_len=*(uint32_t*)(buf+12);
			str=(char*)(buf+16);

			return 0;
		}

	private:
		uint32_t	string_max_;
		uint32_t	string_now_;

		char		rdb_file_[256];
		FILE*		rdb_fp_;
		int64_t		rdb_length_;

		char**		is_map_;
		QHashSearch<uint64_t> si_map_;

		QAllocator	allocator_;

		QMutexLock	mutex_;
		QMD5		md5_;
};

Q_END_NAMESPACE

#endif // __QSTRIDALLOCATOR_H_
