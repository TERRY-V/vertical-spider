/********************************************************************************************
**
** Copyright (C) 2010-2015 Terry Niu (Beijing, China)
** Filename:	qdiskcache.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2015/04/13
**
*********************************************************************************************/

#ifndef __QDISKCACHE_H_
#define __QDISKCACHE_H_

#include "qglobal.h"
#include "qallocator.h"

#define DEFAULT_BUCKET_SIZE	(1<<20)
#define DEFAULT_CACHE_INTERVAL	(30*60)
#define DEFAULT_DISK_CACHE_FILE	((char*)("__disk.cache"))
#define DEFAULT_DISK_SWAP_FILE	((char*)("__disk.last"))

Q_BEGIN_NAMESPACE

// 缓存类
template <typename Key>
class QDiskCache {
	public:
		inline QDiskCache() :
			data_len_(-1),
			data_num_(0),
			bucket_size_(0),
			hash_table_(NULL),
			allocator_FL_(NULL),
			allocator_VL_(NULL),
			traversal_index_(-1),
			traversal_pos_(NULL),
			cache_interval_(0),
			success_flag_(0),
			initialized_(0)
		{}

		virtual ~QDiskCache()
		{
			if(hash_table_!=NULL)
				q_delete_array<char*>(hash_table_);

			if(allocator_FL_!=NULL)
				q_delete<QPoolAllocator>(allocator_FL_);

			if(allocator_VL_!=NULL)
				q_delete<QAllocator>(allocator_VL_);
		}

		// @函数名: 缓存类初始化函数
		// @参数01: 缓存类桶的大小
		// @参数02: 附加数据长度, 小于0表示变长数据
		// @返回值: 成功返回0, 失败返回小于0的错误码
		int32_t init(int32_t bucket_size=DEFAULT_BUCKET_SIZE, int32_t data_len=0, int32_t cache_interval=DEFAULT_CACHE_INTERVAL)
		{
			data_len_=data_len;
			cache_interval_=cache_interval;

			data_num_=0;

			bucket_size_=bucket_size;
			hash_table_=q_new_array<char*>(bucket_size_);
			if(hash_table_==NULL)
				return -1;
			memset(hash_table_, 0, bucket_size_*sizeof(void*));

			if(data_len_<0) {
				allocator_VL_=q_new< QAllocator >();
				if(allocator_VL_==NULL)
					return -2;
			} else {
				allocator_FL_=q_new< QPoolAllocator >();
				if(allocator_FL_==NULL)
					return -3;
				if(allocator_FL_->init(sizeof(void*)+sizeof(Key)+data_len_))
					return -4;
			}

			traversal_index_=0;
			traversal_pos_=hash_table_[traversal_index_];

			initialized_=1;

			if(access(DEFAULT_DISK_CACHE_FILE, 0)==0&&loadDataFromDisk(DEFAULT_DISK_CACHE_FILE))
				return -5;

			if(q_create_thread(thread_save, this))
				return -6;

			while(success_flag_!=1)
				q_sleep(1);

			if(success_flag_==-1)
				return -7;

			return 0;
		}

		// @函数名: 插入定长值元素
		// @参数01: 元素键key
		// @参数02: 元素值value
		// @参数03: 缓存类中元素值的位置
		// @参数04: 缓存类中元素的桶号
		// @返回值: 成功返回0, 失败返回<0的错误码, 重复键key返回1
		int32_t addKey_FL(Key key, void* vpData=NULL, void** vppRetBuf=NULL, int32_t iIndex=-1)
		{
			mutex_.lock();

			// 计算元素在缓存类的桶号
			if(iIndex<0) {
				iIndex=(int32_t)(key%bucket_size_);
				if(iIndex<0)
					iIndex*=-1;
			}

			char* p=hash_table_[iIndex];
			while(p) {
				// 判断该元素的key是否存在
				if(*(Key*)(p+sizeof(void*))==key) {
					if(vppRetBuf) {
						if(data_len_==0)
							*vppRetBuf=NULL;
						else
							*vppRetBuf=(void*)(p+sizeof(void*)+sizeof(Key));
					}
					mutex_.unlock();
					return 1;
				}
				p=*(char**)p;
			}

			// 不存在时需要分配空间
			p=allocator_FL_->alloc();
			if(p==NULL) {
				mutex_.unlock();
				return -1;
			}

			// 拷贝key和value
			*(Key*)(p+sizeof(void*))=key;
			if(data_len_>0&&vpData)
				memcpy(p+sizeof(void*)+sizeof(Key), vpData, data_len_);

			// 将该元素插入链表
			*(char**)p=hash_table_[iIndex];
			hash_table_[iIndex]=p;

			// 总元素数量+1
			data_num_++;

			// 元素值value在缓存类中的位置
			if(vppRetBuf!=NULL) {
				if(data_len_==0)
					*vppRetBuf=NULL;
				else
					*vppRetBuf=(void*)(p+sizeof(void*)+sizeof(key));
			}

			mutex_.unlock();
			return 0;
		}

		// @函数名: 搜索定长值元素
		// @参数01: 元素键key
		// @参数02: 缓存类中元素值的位置
		// @参数04: 缓存类中元素的桶号
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t searchKey_FL(Key key, void** vppRetBuf=NULL, int32_t iIndex=-1)
		{
			mutex_.lock();

			// 计算元素在缓存类的桶号
			if(iIndex<0) {
				iIndex=(int32_t)(key%bucket_size_);
				if(iIndex<0)
					iIndex*=-1;
			}
			// 查找缓存类
			char* p=hash_table_[iIndex];
			while(p) {
				if(*(Key*)(p+sizeof(void*))==key) {
					if(vppRetBuf) {
						if(data_len_==0)
							*vppRetBuf=NULL;
						else
							*vppRetBuf=(void*)(p+sizeof(void*)+sizeof(key));
					}
					mutex_.unlock();
					return 0;
				}
				p=*(char**)p;
			}

			mutex_.unlock();
			return -1;
		}

		// @函数名: 更新定长值元素
		// @参数01: 元素键key
		// @参数02: 元素值value
		// @参数03: 缓存类中元素值的位置
		// @参数04: 缓存类中元素的桶号
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t updateKey_FL(Key key, void* vpData=NULL, void** vppRetBuf=NULL, int32_t iIndex=-1)
		{
			mutex_.lock();

			// 计算元素在缓存类的桶号
			if(iIndex<0) {
				iIndex=(int32_t)(key%bucket_size_);
				if(iIndex<0)
					iIndex*=-1;
			}

			char* p=hash_table_[iIndex];
			while(p) {
				// 判断该元素的key是否存在
				if(*(Key*)(p+sizeof(void*))==key) {
					if(data_len_>0&&vpData)
						memcpy(p+sizeof(void*)+sizeof(Key), vpData, data_len_);

					if(vppRetBuf) {
						if(data_len_==0)
							*vppRetBuf=NULL;
						else
							*vppRetBuf=(void*)(p+sizeof(void*)+sizeof(Key));
					}
					mutex_.unlock();
					return 0;
				}
				p=*(char**)p;
			}

			mutex_.unlock();
			return -1;
		}

		// @函数名: 删除键为key的定长值元素
		// @参数01: 元素键key
		// @参数02: 缓存类中该元素的桶号
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t deleteKey_FL(Key key, int32_t iIndex=-1)
		{
			mutex_.lock();

			// 计算元素在缓存类的桶号
			if(iIndex<0) {
				iIndex=(int32_t)(key%bucket_size_);
				if(iIndex<0)
					iIndex*=-1;
			}
			// 查找缓存类
			char* p=(char*)hash_table_[iIndex];
			char* q=(char*)(hash_table_+iIndex);
			while(p) {
				if(*(Key*)(p+sizeof(void*))==key) {
					*(char**)q=*(char**)p;
					*(char**)p=0;
					allocator_FL_->free(p);
					data_num_--;
					mutex_.unlock();
					return 0;
				}
				q=p;
				p=*(char**)p;
			}

			mutex_.unlock();
			return -1;
		}

		// @函数名: 插入非定常值的元素
		// @参数01: 元素键key
		// @参数02: 元素值value
		// @参数03: 元素值的长度
		// @参数04: 元素值在缓存类中的位置
		// @参数05: 缓存类中元素值的长度
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t addKey_VL(Key key, void* vpData, int32_t data_len, void** vppRetBuf, int32_t* lpRetDataLen)
		{
			mutex_.lock();

			// 检查传入的参数值
			if(data_len<0)
				return -1;
			// 计算元素在缓存类中的桶号
			int32_t iIndex=(int32_t)(key%bucket_size_);
			if(iIndex<0)
				iIndex*=-1;
			// 查找并放入缓存类
			char* p=hash_table_[iIndex];
			while(p) {
				if(*(Key*)(p+sizeof(void*))==key) {
					*lpRetDataLen=*(int32_t*)(p+sizeof(void*)+sizeof(Key));
					if(*lpRetDataLen==0)
						*vppRetBuf=NULL;
					else
						*vppRetBuf=(void*)(p+sizeof(void*)+sizeof(Key)+sizeof(int32_t));
					mutex_.unlock();
					return 1;
				}
				p=*(char**)p;
			}

			// 不存在时重新分配指定长度的空间
			p=allocator_VL_->alloc(sizeof(void*)+sizeof(Key)+sizeof(int32_t)+data_len);
			if(p==NULL) {
				mutex_.unlock();
				return -1;
			}

			// 拷贝键值对
			*(Key*)(p+sizeof(void*))=key;
			*(int32_t*)(p+sizeof(void*)+sizeof(key))=data_len;
			if(data_len>0)
				memcpy(p+sizeof(void*)+sizeof(Key)+sizeof(int32_t), vpData, data_len);

			*(char**)p=hash_table_[iIndex];
			hash_table_[iIndex]=p;

			// 总元素数量++1
			data_num_++;

			// 返回缓存类中元素值的长度和位置
			*lpRetDataLen=*(int32_t*)(p+sizeof(void*)+sizeof(Key));

			if(*lpRetDataLen==0)
				*vppRetBuf=NULL;
			else
				*vppRetBuf=(void*)(p+sizeof(void*)+sizeof(Key)+sizeof(int32_t));

			mutex_.unlock();
			return 0;
		}

		// @函数名: 搜索非定常值的元素
		// @参数01: 元素键key
		// @参数02: 元素值在缓存类中的位置
		// @参数03: 缓存类中元素值的长度
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t searchKey_VL(Key key, void** vppRetBuf, int32_t* lpRetDataLen)
		{
			mutex_.lock();

			// 计算元素在缓存类中的桶号
			int32_t iIndex=(int32_t)(key%bucket_size_);
			if(iIndex<0)
				iIndex*=-1;

			// 查找缓存类
			char* p=(char*)hash_table_[iIndex];
			while(p) {
				if(*(Key*)(p+sizeof(void*))==key) {
					*lpRetDataLen=*(int32_t*)(p+sizeof(void*)+sizeof(Key));
					if(*lpRetDataLen==0)
						*vppRetBuf=NULL;
					else
						*vppRetBuf=(void*)(p+sizeof(void*)+sizeof(Key)+sizeof(int32_t));
					mutex_.unlock();
					return 0;
				}
				p=*(char**)p;
			}

			mutex_.unlock();
			return -1;
		}

		// 函数名: 通过哈希函数获取哈希值
		int32_t getHashValue(Key key)
		{
			int32_t iIndex=(int32_t)(key%bucket_size_);
			if(iIndex<0)
				iIndex*=-1;
			return iIndex;
		}

		// @函数名: 获取键的数量
		int32_t getKeyNum() const
		{
			return data_num_;
		}

		// @函数名: 清空缓存类
		int32_t clear()
		{
			mutex_.lock();
			data_num_=0;

			if(allocator_VL_)
				allocator_VL_->resetAllocator();
			if(allocator_FL_)
				allocator_FL_->resetAllocator();

			memset(hash_table_, 0, bucket_size_*sizeof(void*));

			traversal_index_=0;
			traversal_pos_=hash_table_[traversal_index_];

			mutex_.unlock();
			return 0;
		}

		// @函数名: 内部结构遍历缓存类
		int32_t prepareTraversal()
		{
			traversal_index_=0;
			traversal_pos_=(char*)hash_table_[traversal_index_];
			return 0;
		}

		// @函数名: 遍历元素
		// @参数01: 元素键key
		// @参数02: 元素值value指针
		// @参数03: 数据长度
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t traverse(Key& key, void*& vprRetBuf, int32_t& data_len)
		{
			if(traversal_pos_==NULL) {
				for(traversal_index_++; traversal_index_<bucket_size_; ++traversal_index_) {
					traversal_pos_=(char*)hash_table_[traversal_index_];
					if(traversal_pos_==NULL)
						continue;
					else
						break;
				}
				if(traversal_index_==bucket_size_)
					return -1;
			}

			key=*(Key*)(traversal_pos_+sizeof(void*));
			if(data_len_<0) {
				data_len=*(int32_t*)(traversal_pos_+sizeof(void*)+sizeof(Key));
				vprRetBuf=(void*)(traversal_pos_+sizeof(void*)+sizeof(Key)+sizeof(int32_t));
			} else if(data_len_>0) {
				data_len=data_len_;
				vprRetBuf=(void*)(traversal_pos_+sizeof(void*)+sizeof(Key));
			}
			traversal_pos_=*(char**)traversal_pos_;

			return 0;
		}

	private:
		static Q_THREAD_T thread_save(void* argv)
		{
			QDiskCache* ptr_this=static_cast<QDiskCache*>(argv);
			Q_CHECK_PTR(ptr_this);

			FILE* fp=NULL;
			uint32_t beginFlag=0xEEEEEEEE;
			uint32_t endFlag=0xFFFFFFFF;

			ptr_this->success_flag_=1;

			Q_FOREVER {
				q_sleep(ptr_this->cache_interval_*1000);

				ptr_this->mutex_.lock();

				if(access(DEFAULT_DISK_CACHE_FILE, 0)==0)
					q_swap_file(DEFAULT_DISK_CACHE_FILE, DEFAULT_DISK_SWAP_FILE);

				do {
					fp=fopen(DEFAULT_DISK_CACHE_FILE, "wb");
					if(fp)
						break;
					q_sleep(1000);
				} while(fp==NULL);

				if(fwrite(&beginFlag, sizeof(int32_t), 1, fp)!=1)
					goto err;

				if(fwrite(&ptr_this->bucket_size_, sizeof(int32_t), 1, fp)!=1)
					goto err;
				if(fwrite(&ptr_this->data_num_, sizeof(int32_t), 1, fp)!=1)
					goto err;
				if(fwrite(&ptr_this->data_len_, sizeof(int32_t), 1, fp)!=1)
					goto err;

				if(ptr_this->data_len_<0) {
					for(int32_t i=0; i<ptr_this->bucket_size_; ++i) {
						if(ptr_this->hash_table_[i]==NULL) {
							continue;
						} else {
							char* p=(char*)ptr_this->hash_table_[i];
							while(p) {
								if(fwrite(p+sizeof(void*), sizeof(Key), 1, fp)!=1)
									goto err;
								if(fwrite(p+sizeof(void*)+sizeof(Key), sizeof(int32_t), 1, fp)!=1)
									goto err;
								if(fwrite(p+sizeof(void*)+sizeof(Key)+sizeof(int32_t), *(int32_t*)(p+sizeof(void*)+sizeof(Key)), 1, fp)!=1)
									goto err;
								p=*(char**)p;
							}
						}
					}
				} else {
					for(int32_t i=0; i<ptr_this->bucket_size_; ++i) {
						if(ptr_this->hash_table_[i]==NULL) {
							continue;
						} else {
							char* p=(char*)ptr_this->hash_table_[i];
							while(p) {
								if(fwrite(p+sizeof(void*), sizeof(Key), 1, fp)!=1)
									goto err;
								if(ptr_this->data_len_&&fwrite(p+sizeof(void*)+sizeof(Key), ptr_this->data_len_, 1, fp)!=1)
									goto err;
								p=*(char**)p;
							}
						}
					}
				}

				fseek(fp, 0, SEEK_SET);
				if(fwrite(&endFlag, sizeof(int32_t), 1, fp)!=1)
					goto err;

err:
				fclose(fp);

				ptr_this->mutex_.unlock();
			}

			ptr_this->success_flag_=-1;
			return 0;
		}

		// @函数名: 从持久化文件中载入缓存类数据(虽然有load方法, 但init方法的调用是必须的)
		// @参数01: 持久化数据文件名
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t loadDataFromDisk(char* fileName)
		{
			char head[32];
			void* vpRetBuf=NULL;
			int32_t lRetLen=0;

			uint32_t iFlag=0;
			int32_t bucket_size=0;
			int32_t data_num=0;
			int32_t data_len=0;

			FILE* fp=::fopen(fileName, "rb");
			if(fp==NULL)
				return -2;

			char* buffer=q_new_array<char>(1<<20);
			if(buffer==NULL)
				return -3;

			if(fread(&iFlag, sizeof(int32_t), 1, fp)!=1||iFlag!=0xFFFFFFFF)
				goto err;
			if(fread(&bucket_size, sizeof(int32_t), 1, fp)!=1)
				goto err;
			if(fread(&data_num, sizeof(int32_t), 1, fp)!=1)
				goto err;
			if(fread(&data_len, sizeof(int32_t), 1, fp)!=1)
				goto err;

			if(data_len_<0) {
				int32_t lOneLen=sizeof(Key)+sizeof(int32_t);
				while(fread(head, lOneLen, 1, fp)) {
					if(fread(buffer, *(int32_t*)(head+sizeof(Key)), 1, fp)!=1)
						goto err;
					if(addKey_VL(*(Key*)head, buffer, *(int32_t*)(head+sizeof(Key)), &vpRetBuf, &lRetLen))
						goto err;
				}
			} else {
				Key key;
				while(fread(&key, sizeof(Key), 1, fp)) {
					if(data_len_&&fread(buffer, data_len_, 1, fp)!=1)
						goto err;
					if(addKey_FL(key, buffer, &vpRetBuf))
						goto err;
				}
			}

err:
			q_delete_array<char>(buffer);
			fclose(fp);

			if(getKeyNum()!=data_num)
				return -4;

			return 0;
		}

	protected:
		int32_t			data_len_;		// 缓存类数据长度, 定长和变长标识
		int32_t			data_num_;		// 缓存类总元素数量

		int32_t			bucket_size_;		// 缓存类桶的大小
		char**			hash_table_;		// 内存缓存类

		QPoolAllocator*		allocator_FL_;		// 定长数据内存池
		QAllocator*		allocator_VL_;		// 变长数据内存池

		int32_t			traversal_index_;	// 缓存类遍历桶索引
		char*			traversal_pos_;		// 缓存类遍历链表位置

		int32_t			cache_interval_;	// 缓存备份间隔时间

		QMutexLock		mutex_;			// 缓存类互斥锁
		int32_t			success_flag_;		// 线程启动成功标志
		int32_t			initialized_;		// 缓存类初始化标识
};

Q_END_NAMESPACE

#endif // __QDISKCACHE_H_
