/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qhashsearch.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/06/24
**
*********************************************************************************************/

#ifndef __QHASHSEARCH_H_
#define __QHASHSEARCH_H_

#include "qglobal.h"
#include "qallocator.h"

Q_BEGIN_NAMESPACE

// 哈希表
template <typename Key>
class QHashSearch {
	public:
		// @函数名: 哈希表构造函数
		inline QHashSearch() :
			data_len_(-1),
			data_num_(0),
			bucket_size_(0),
			hash_table_(NULL),
			allocator_FL_(NULL),
			allocator_VL_(NULL),
			traversal_index_(-1),
			traversal_pos_(NULL),
			initialized_(0)
		{}

		// @函数名: 哈希表析构函数
		virtual ~QHashSearch()
		{
			if(hash_table_!=NULL)
				q_delete_array<char*>(hash_table_);

			if(allocator_FL_!=NULL)
				q_delete<QPoolAllocator>(allocator_FL_);

			if(allocator_VL_!=NULL)
				q_delete< QAllocator >(allocator_VL_);
		}

		// @函数名: 哈希表初始化函数
		// @参数01: 哈希表桶的大小
		// @参数02: 附加数据长度, 小于0表示变长数据
		// @返回值: 成功返回0, 失败返回小于0的错误码
		int32_t init(int32_t bucket_size, int32_t data_len=0)
		{
			data_len_=data_len;
			data_num_=0;

			bucket_size_=bucket_size;
			hash_table_=q_new_array<char*>(bucket_size_);
			if(hash_table_==NULL)
				return -1;
			memset(hash_table_, 0, bucket_size_*sizeof(void*));

			if(data_len_<0) {
				allocator_VL_=q_new<QAllocator>();
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

			return 0;
		}

		// @函数名: 插入定长值元素
		// @参数01: 元素键key
		// @参数02: 元素值value
		// @参数03: 哈希表中元素值的位置
		// @参数04: 哈希表中元素的桶号
		// @返回值: 成功返回0, 失败返回<0的错误码, 重复键key返回1
		int32_t addKey_FL(Key key, void* vpData=NULL, void** vppRetBuf=NULL, int32_t iIndex=-1)
		{
			// 计算元素在哈希表的桶号
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
					return 1;
				}
				p=*(char**)p;
			}

			// 不存在时需要分配空间
			p=allocator_FL_->alloc();
			if(p==NULL)
				return -1;

			// 拷贝key和value
			*(Key*)(p+sizeof(void*))=key;
			if(data_len_>0&&vpData)
				memcpy(p+sizeof(void*)+sizeof(Key), vpData, data_len_);

			// 将该元素插入链表
			*(char**)p=hash_table_[iIndex];
			hash_table_[iIndex]=p;

			// 总元素数量+1
			data_num_++;

			// 元素值value在哈希表中的位置
			if(vppRetBuf!=NULL) {
				if(data_len_==0)
					*vppRetBuf=NULL;
				else
					*vppRetBuf=(void*)(p+sizeof(void*)+sizeof(key));
			}

			return 0;
		}

		// @函数名: 搜索定长值元素
		// @参数01: 元素键key
		// @参数02: 哈希表中元素值的位置
		// @参数04: 哈希表中元素的桶号
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t searchKey_FL(Key key, void** vppRetBuf=NULL, int32_t iIndex=-1)
		{
			// 计算元素在哈希表的桶号
			if(iIndex<0) {
				iIndex=(int32_t)(key%bucket_size_);
				if(iIndex<0)
					iIndex*=-1;
			}
			// 查找哈希表
			char* p=hash_table_[iIndex];
			while(p) {
				if(*(Key*)(p+sizeof(void*))==key) {
					if(vppRetBuf) {
						if(data_len_==0)
							*vppRetBuf=NULL;
						else
							*vppRetBuf=(void*)(p+sizeof(void*)+sizeof(key));
					}
					return 0;
				}
				p=*(char**)p;
			}
			return -1;
		}

		// @函数名: 更新定长值元素
		// @参数01: 元素键key
		// @参数02: 元素值value
		// @参数03: 哈希表中元素值的位置
		// @参数04: 哈希表中元素的桶号
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t updateKey_FL(Key key, void* vpData=NULL, void** vppRetBuf=NULL, int32_t iIndex=-1)
		{
			// 计算元素在哈希表的桶号
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
					return 0;
				}
				p=*(char**)p;
			}

			return -1;
		}

		// @函数名: 删除键为key的定长值元素
		// @参数01: 元素键key
		// @参数02: 哈希表中该元素的桶号
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t deleteKey_FL(Key key, int32_t iIndex=-1)
		{
			// 计算元素在哈希表的桶号
			if(iIndex<0) {
				iIndex=(int32_t)(key%bucket_size_);
				if(iIndex<0)
					iIndex*=-1;
			}
			// 查找哈希表
			char* p=(char*)hash_table_[iIndex];
			char* q=(char*)(hash_table_+iIndex);
			while(p) {
				if(*(Key*)(p+sizeof(void*))==key) {
					*(char**)q=*(char**)p;
					*(char**)p=0;
					allocator_FL_->free(p);
					data_num_--;
					return 0;
				}
				q=p;
				p=*(char**)p;
			}
			return -1;
		}

		// @函数名: 插入非定常值的元素
		// @参数01: 元素键key
		// @参数02: 元素值value
		// @参数03: 元素值的长度
		// @参数04: 元素值在哈希表中的位置
		// @参数05: 哈希表中元素值的长度
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t addKey_VL(Key key, void* vpData, int32_t data_len, void** vppRetBuf, int32_t* lpRetDataLen)
		{
			// 检查传入的参数值
			if(data_len<0)
				return -1;
			// 计算元素在哈希表中的桶号
			int32_t iIndex=(int32_t)(key%bucket_size_);
			if(iIndex<0)
				iIndex*=-1;
			// 查找并放入哈希表
			char* p=hash_table_[iIndex];
			while(p) {
				if(*(Key*)(p+sizeof(void*))==key) {
					*lpRetDataLen=*(int32_t*)(p+sizeof(void*)+sizeof(Key));
					if(*lpRetDataLen==0)
						*vppRetBuf=NULL;
					else
						*vppRetBuf=(void*)(p+sizeof(void*)+sizeof(Key)+sizeof(int32_t));
					return 1;
				}
				p=*(char**)p;
			}

			// 不存在时重新分配指定长度的空间
			p=allocator_VL_->alloc(sizeof(void*)+sizeof(Key)+sizeof(int32_t)+data_len);
			if(p==NULL)
				return -1;

			// 拷贝键值对
			*(Key*)(p+sizeof(void*))=key;
			*(int32_t*)(p+sizeof(void*)+sizeof(key))=data_len;
			if(data_len>0)
				memcpy(p+sizeof(void*)+sizeof(Key)+sizeof(int32_t), vpData, data_len);

			*(char**)p=hash_table_[iIndex];
			hash_table_[iIndex]=p;

			// 总元素数量++1
			data_num_++;

			// 返回哈希表中元素值的长度和位置
			*lpRetDataLen=*(int32_t*)(p+sizeof(void*)+sizeof(Key));

			if(*lpRetDataLen==0)
				*vppRetBuf=NULL;
			else
				*vppRetBuf=(void*)(p+sizeof(void*)+sizeof(Key)+sizeof(int32_t));

			return 0;
		}

		// @函数名: 搜索非定常值的元素
		// @参数01: 元素键key
		// @参数02: 元素值在哈希表中的位置
		// @参数03: 哈希表中元素值的长度
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t searchKey_VL(Key key, void** vppRetBuf, int32_t* lpRetDataLen)
		{
			// 计算元素在哈希表中的桶号
			int32_t iIndex=(int32_t)(key%bucket_size_);
			if(iIndex<0)
				iIndex*=-1;
			// 查找哈希表
			char* p=(char*)hash_table_[iIndex];
			while(p) {
				if(*(Key*)(p+sizeof(void*))==key) {
					*lpRetDataLen=*(int32_t*)(p+sizeof(void*)+sizeof(Key));
					if(*lpRetDataLen==0)
						*vppRetBuf=NULL;
					else
						*vppRetBuf=(void*)(p+sizeof(void*)+sizeof(Key)+sizeof(int32_t));
					return 0;
				}
				p=*(char**)p;
			}
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

		// @函数名: 清空哈希表
		int32_t clear()
		{
			data_num_=0;

			if(allocator_VL_)
				allocator_VL_->resetAllocator();
			if(allocator_FL_)
				allocator_FL_->resetAllocator();

			memset(hash_table_, 0, bucket_size_*sizeof(void*));

			traversal_index_=0;
			traversal_pos_=hash_table_[traversal_index_];

			return 0;
		}

		// @函数名: 内部结构遍历哈希表
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

		// @函数名: 哈希表持久化(在指定的时间间隔内生成数据集的时间点快照, 适用于灾难恢复)
		// @参数01: 持久化文件名
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t save(char* filename)
		{
			if(initialized_==0)
				return -1;

			FILE* fp=fopen(filename, "wb");
			if(fp==NULL)
				return -2;

			int32_t iFlag=0xEEEEEEEE;
			if(fwrite(&iFlag, sizeof(int32_t), 1, fp)!=1)
				goto err;

			if(fwrite(&bucket_size_, sizeof(int32_t), 1, fp)!=1)
				goto err;
			if(fwrite(&data_num_, sizeof(int32_t), 1, fp)!=1)
				goto err;
			if(fwrite(&data_len_, sizeof(int32_t), 1, fp)!=1)
				goto err;

			if(data_len_<0) {
				for(int32_t i=0; i<bucket_size_; ++i) {
					if(hash_table_[i]==NULL) {
						continue;
					} else {
						char* p=(char*)hash_table_[i];
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
				for(int32_t i=0; i<bucket_size_; ++i) {
					if(hash_table_[i]==NULL) {
						continue;
					} else {
						char* p=(char*)hash_table_[i];
						while(p) {
							if(fwrite(p+sizeof(void*), sizeof(Key), 1, fp)!=1)
								goto err;
							if(data_len_&&fwrite(p+sizeof(void*)+sizeof(Key), data_len_, 1, fp)!=1)
								goto err;
							p=*(char**)p;
						}
					}
				}
			}

			fseek(fp, 0, SEEK_SET);
			iFlag=0xFFFFFFFF;
			if(fwrite(&iFlag, sizeof(int32_t), 1, fp)!=1)
				goto err;

err:
			fclose(fp);

			return 0;
		}

		// @函数名: 从持久化文件中载入哈希表数据(虽然有load方法, 但init方法的调用是必须的)
		// @参数01: 持久化数据文件名
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t load(char* fileName)
		{
			if(initialized_==1)
				return -1;

			char head[32];
			void* vpRetBuf=NULL;
			int32_t lRetLen=0;

			int32_t iFlag=0;
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

	private:
		int32_t			data_len_;		// 哈希表数据长度, 定长和变长标识
		int32_t			data_num_;		// 哈希表总元素数量

		int32_t			bucket_size_;		// 哈希表桶的大小
		char**			hash_table_;		// 内存哈希表

		QPoolAllocator*		allocator_FL_;		// 定长数据内存池
		QAllocator*		allocator_VL_;		// 变长数据内存池

		int32_t			traversal_index_;	// 哈希表遍历桶索引
		char*			traversal_pos_;		// 哈希表遍历链表位置

		int32_t			initialized_;		// 哈希表初始化标识
};

Q_END_NAMESPACE

#endif // __QHASHSEARCH_H_
