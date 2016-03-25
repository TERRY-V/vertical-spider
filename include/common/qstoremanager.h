/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qstoremanager.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/11/10
**
*********************************************************************************************/

#ifndef __QSTOREMANAGER_H_
#define __QSTOREMANAGER_H_

#include "qglobal.h"

Q_BEGIN_NAMESPACE

// QStoreManager散列溢出链磁盘存储类
// 散列文件按桶存放记录, 假若一个桶能存放m个记录, 则m个互为同义词的记录存放在同一地址的桶中。
// 当第m+1个同义词出现时, 发生溢出并存放在溢出桶中。
// 溢出桶和基桶的大小相同, 相互之间用指针连接, 当在基桶中没有检索到待查记录时, 就顺指针到溢出桶进行检索。
// 缺陷: 从基桶走到溢出桶可能需要移动磁盘的活动臂
class QStoreManager {
	public:
		// @函数名: 散列溢出链磁盘存储类构造函数
		inline QStoreManager() :
			I_STORE_FILE_MARK(*(uint64_t*)"@QHSM1.0"),
			I_STORE_BEGIN_MARK(0xBBBBBBBBBBBBBBBB),
			I_STORE_END_MARK(0xEEEEEEEEEEEEEEEE),
			m_iDataLen(-1),
			m_fpHash(NULL),
			m_fpData(NULL),
			m_iAllDataNum(0),
			m_iBucketNum(0),
			m_iBucketSize(0),
			m_pBucket(NULL),
			m_isInitSystem(0)
		{}

		// @函数名: 散列溢出链磁盘存储类析构函数
		virtual ~QStoreManager()
		{
			if(m_pBucket)
				q_delete_array<char>(m_pBucket);

			if(m_fpHash)
				fclose(m_fpHash);

			if(m_fpData)
				fclose(m_fpData);
		}

		// @函数名: 初始化函数
		// @参数01: 散列文件名称
		// @参数02: 散列文件的桶数
		// @参数03: 散列文件的桶容量
		// @参数03: 附加数据长度, 小于0为变长数据
		// @返回值: 成功返回0, 失败返回小于0的错误码
		int32_t init(const char* name, int32_t iBucketNum, int32_t iBucketSize=10, int32_t iDataLen=0)
		{
			if(name==NULL||*name==0||iBucketNum<=0||iBucketSize<=0)
				return -1;

			if(q_snprintf(m_szHashFile, sizeof(m_szHashFile)-1, "%s.hsm", name)<0)
				return -2;

			if(q_snprintf(m_szDataFile, sizeof(m_szDataFile)-1, "%s.hsm.dat", name)<0)
				return -3;

			if(access(m_szHashFile, 0)==0) {
				m_fpHash=fopen(m_szHashFile, "rb+");
				if(m_fpHash==NULL) {
					Q_DEBUG("QStoreManager: fopen (%s) error!", m_szHashFile);
					return -4;
				}

				uint64_t magic_mark;
				if(fread(&magic_mark, sizeof(uint64_t), 1, m_fpHash)!=1||magic_mark!=I_STORE_FILE_MARK)
					return -5;

				if(fread(&m_iBucketNum, sizeof(int32_t), 1, m_fpHash)!=1)
					return -6;

				if(fread(&m_iBucketSize, sizeof(int32_t), 1, m_fpHash)!=1)
					return -7;

				if(fread(&m_iDataLen, sizeof(int32_t), 1, m_fpHash)!=1)
					return -8;

				// magic number + bucket num + bucket size + data length
				m_iHeadLen=sizeof(uint64_t)+3*sizeof(int32_t);

				// key(8 bytes) + delete tag(1 byte) + value position(8 bytes)
				if(m_iDataLen) {
					m_iElementLen=sizeof(uint64_t)+sizeof(int8_t)+sizeof(int64_t);
				} else {
					m_iElementLen=sizeof(uint64_t)+sizeof(int8_t);
				}

				// element length * bucket size + overflow bucket index
				m_iBucketLen=(m_iElementLen)*m_iBucketSize+sizeof(int32_t);

				// hash bucket
				m_pBucket=q_new_array<char>(m_iBucketLen);
				if(m_pBucket==NULL)
					return -9;
				memset(m_pBucket, 0, m_iBucketLen);

				if(m_iDataLen) {
					m_fpData=fopen(m_szDataFile, "rb+");
					if(m_fpData==NULL) {
						Q_DEBUG("QStoreManager: fopen (%s) error!", m_szDataFile);
						return -10;
					}
				}
			} else {
				m_iBucketNum=iBucketNum;
				m_iBucketSize=iBucketSize;
				m_iDataLen=iDataLen;

				// magic number + bucket num + bucket size + data length
				m_iHeadLen=sizeof(uint64_t)+3*sizeof(int32_t);

				// key(8 bytes) + delete tag(1 byte) + value position(8 bytes)
				if(m_iDataLen) {
					m_iElementLen=sizeof(uint64_t)+sizeof(int8_t)+sizeof(int64_t);
				} else {
					m_iElementLen=sizeof(uint64_t)+sizeof(int8_t);
				}

				// element length * bucket size + overflow bucket index
				m_iBucketLen=(m_iElementLen)*m_iBucketSize+sizeof(int32_t);

				// hash bucket
				m_pBucket=q_new_array<char>(m_iBucketLen);
				if(m_pBucket==NULL)
					return -4;
				memset(m_pBucket, 0, m_iBucketLen);

				m_fpHash=fopen(m_szHashFile, "w+");
				if(m_fpHash==NULL) {
					Q_DEBUG("QStoreManager: fopen (%s) error!", m_szHashFile);
					return -5;
				}

				if(m_iDataLen) {
					m_fpData=fopen(m_szDataFile, "w+");
					if(m_fpData==NULL) {
						Q_DEBUG("QStoreManager: fopen (%s) error!", m_szDataFile);
						return -6;
					}
				}

				if(fwrite(&I_STORE_FILE_MARK, sizeof(uint64_t), 1, m_fpHash)!=1)
					return -7;

				if(fwrite(&m_iBucketNum, sizeof(int32_t), 1, m_fpHash)!=1)
					return -8;

				if(fwrite(&m_iBucketSize, sizeof(int32_t), 1, m_fpHash)!=1)
					return -9;

				if(fwrite(&m_iDataLen, sizeof(int32_t), 1, m_fpHash)!=1)
					return -10;

				for(int32_t i=0; i<m_iBucketNum; ++i) {
					if(fwrite(m_pBucket, m_iBucketLen, 1, m_fpHash)!=1)
						return -11;
				}
			}

			m_isInitSystem=1;

			return 0;
		}

		// @函数名: 插入定长值元素
		// @参数01: 元素键key
		// @参数02: 元素值value
		// @返回值: 成功返回0, 失败返回<0的错误码, 重复键返回1
		int32_t addKey_FL(uint64_t key, void* vpData=NULL)
		{
			Q_ASSERT(m_isInitSystem==1, "QStoreManager: sure you have inited?");

			int32_t bucketIndex=(int32_t)(key%m_iBucketNum);
			int64_t bucketPos=get_bucket_position(bucketIndex);

			uint64_t readKey=0;
			int8_t deleteFlag=0;
			int32_t overflowBucketIndex=0;

			int64_t vPosition=0;

			Q_FOREVER {
				if(fseek(m_fpHash, bucketPos, SEEK_SET)==-1)
					return -1;

				for(int32_t i=0; i<m_iBucketSize; ++i) {
					if(fread(&readKey, sizeof(uint64_t), 1, m_fpHash)!=1)
						return -2;

					if(readKey==key)
						return 1;

					if(readKey==0) {
						if(fseek(m_fpHash, -8, SEEK_CUR)==-1)
							return -3;

						if(fwrite(&key, sizeof(uint64_t), 1, m_fpHash)!=1)
							return -4;

						if(fwrite(&deleteFlag, sizeof(int8_t), 1, m_fpHash)!=1)
							return -5;

						if(m_iDataLen>0) {
							if(fseek(m_fpData, 0, SEEK_END)==-1)
								return -6;
							vPosition=ftell(m_fpData);

							if(fwrite(&I_STORE_BEGIN_MARK, sizeof(uint64_t), 1, m_fpData)!=1)
								return -7;

							if(fwrite(vpData, m_iDataLen, 1, m_fpData)!=1)
								return -8;

							if(fwrite(&I_STORE_END_MARK, sizeof(uint64_t), 1, m_fpData)!=1)
								return -9;

							if(fwrite(&vPosition, sizeof(int64_t), 1, m_fpHash)!=1)
								return -10;

						}
						return 0;
					} else {
						if(fseek(m_fpHash, m_iElementLen-sizeof(uint64_t), SEEK_CUR)==-1)
							return -11;
					}
				}

				// 读取溢出桶号
				if(fread(&overflowBucketIndex, sizeof(int32_t), 1, m_fpHash)!=1)
					return -12;

				// 判断是否需要开辟新的溢出桶
				if(overflowBucketIndex==0) {
					++m_iBucketOverflowNum;

					bucketIndex=m_iBucketNum+m_iBucketOverflowNum;
					bucketPos=get_bucket_position(bucketIndex);

					// back 4 bytes
					if(fseek(m_fpHash, -4, SEEK_CUR)==-1)
						return -13;

					if(fwrite(&bucketIndex, sizeof(int32_t), 1, m_fpHash)!=1)
						return -14;

					if(fseek(m_fpHash, bucketPos, SEEK_SET)==-1)
						return -15;

					if(fwrite(m_pBucket, m_iBucketLen, 1, m_fpHash)!=1)
						return -16;
				} else {
					bucketPos=get_bucket_position(overflowBucketIndex);
				}
			}

			return 0;
		}

		// @函数名: 搜索定长值元素
		// @参数01: 元素键
		// @参数02: 元素值外部内存
		// @参数03: 元素值外部内存最大空间
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t searchKey_FL(uint64_t key, void* vpData=NULL, int32_t maxDataSize=0)
		{
			Q_ASSERT(m_isInitSystem==1, "QStoreManager: sure you have inited?");

			int32_t bucketIndex=(int32_t)(key%m_iBucketNum);
			int64_t bucketPos=get_bucket_position(bucketIndex);

			uint64_t readKey=0;
			uint8_t deleteFlag=0;
			int32_t overflowBucketIndex=0;

			uint64_t magic_mark=0;
			int64_t dataOffset=0;

			Q_FOREVER {
				if(fseek(m_fpHash, bucketPos, SEEK_SET)==-1)
					return -1;

				for(int32_t i=0; i<m_iBucketSize; ++i) {
					if(fread(&readKey, sizeof(uint64_t), 1, m_fpHash)!=1)
						return -2;

					if(fread(&deleteFlag, sizeof(uint8_t), 1, m_fpHash)!=1)
						return -3;

					if(readKey==key&&deleteFlag==0) {
						if(m_iDataLen>0&&vpData) {
							if(m_iDataLen>maxDataSize) {
								Q_DEBUG("QStoreManager: maxDataSize (%d) is smaller than m_iDataLen (%d)", maxDataSize, m_iDataLen);
								return -4;
							}

							if(fread(&dataOffset, sizeof(int64_t), 1, m_fpHash)!=1)
								return -5;

							if(fseek(m_fpData, dataOffset, SEEK_SET)==-1)
								return -6;

							if(fread(&magic_mark, sizeof(uint64_t), 1, m_fpData)!=1||magic_mark!=I_STORE_BEGIN_MARK)
								return -7;

							if(fread(vpData, m_iDataLen, 1, m_fpData)!=1)
								return -8;

							if(fread(&magic_mark, sizeof(uint64_t), 1, m_fpData)!=1||magic_mark!=I_STORE_END_MARK)
								return -9;
						}
						return 0;
					}

					if(readKey==0) {
						return -10;
					} else {
						if(fseek(m_fpHash, m_iElementLen-sizeof(uint64_t)-sizeof(int8_t), SEEK_CUR)==-1)
							return -11;
					}
				}

				// 读取溢出桶号
				if(fread(&overflowBucketIndex, sizeof(int32_t), 1, m_fpHash)!=1)
					return -12;

				if(overflowBucketIndex==0) {
					return -13;
				} else {
					bucketPos=get_bucket_position(overflowBucketIndex);
				}
			}

			return 0;
		}

		// @函数名: 删除定长值元素
		// @参数01: 元素键key
		// @返回值: 成功返回0, 失败返回<0的错误码, 不存在返回1
		int32_t deleteKey_FL(uint64_t key)
		{
			Q_ASSERT(m_isInitSystem==1, "QStoreManager: sure you have inited?");

			int32_t bucketIndex=(int32_t)(key%m_iBucketNum);
			int64_t bucketPos=get_bucket_position(bucketIndex);

			uint64_t readKey=0;
			int8_t deleteFlag=1;
			int32_t overflowBucketIndex=0;

			Q_FOREVER {
				if(fseek(m_fpHash, bucketPos, SEEK_SET)==-1)
					return -1;

				for(int32_t i=0; i<m_iBucketSize; ++i) {
					if(fread(&readKey, sizeof(uint64_t), 1, m_fpHash)!=1)
						return -2;

					if(readKey==key) {
						if(fwrite(&deleteFlag, sizeof(int8_t), 1, m_fpHash)!=1)
							return -3;
						return 0;
					}

					if(readKey==0) {
						return 1;
					} else {
						if(fseek(m_fpHash, m_iElementLen-sizeof(uint64_t), SEEK_CUR)==-1)
							return -4;
					}
				}

				// 读取溢出桶号
				if(fread(&overflowBucketIndex, sizeof(int32_t), 1, m_fpHash)!=1)
					return -5;

				// 判断是否需要开辟新的溢出桶
				if(overflowBucketIndex==0) {
					return -6;
				} else {
					bucketPos=get_bucket_position(overflowBucketIndex);
				}
			}

			return 0;
		}

		// @函数名: 插入变长值元素
		// @参数01: 元素键key
		// @参数02: 元素值value
		// @参数03: 元素值value长度
		// @返回值: 成功返回0, 失败返回<0的错误码, 重复键返回1
		int32_t addKey_VL(uint64_t key, void* vpData, int32_t iDataLen)
		{
			Q_ASSERT(m_isInitSystem==1, "QStoreManager: sure you have inited?");

			int32_t bucketIndex=(int32_t)(key%m_iBucketNum);
			int64_t bucketPos=get_bucket_position(bucketIndex);

			uint64_t readKey=0;
			int8_t deleteFlag=0;
			int32_t overflowBucketIndex=0;

			int64_t vPosition=0;

			Q_FOREVER {
				if(fseek(m_fpHash, bucketPos, SEEK_SET)==-1)
					return -1;

				for(int32_t i=0; i<m_iBucketSize; ++i) {
					if(fread(&readKey, sizeof(uint64_t), 1, m_fpHash)!=1)
						return -2;

					if(readKey==key)
						return 1;

					if(readKey==0) {
						if(fseek(m_fpHash, -8, SEEK_CUR)==-1)
							return -3;

						if(fwrite(&key, sizeof(uint64_t), 1, m_fpHash)!=1)
							return -4;

						if(fwrite(&deleteFlag, sizeof(int8_t), 1, m_fpHash)!=1)
							return -5;

						if(fseek(m_fpData, 0, SEEK_END)==-1)
							return -6;
						vPosition=ftell(m_fpData);

						if(fwrite(&I_STORE_BEGIN_MARK, sizeof(uint64_t), 1, m_fpData)!=1)
							return -7;

						if(fwrite(&iDataLen, sizeof(int32_t), 1, m_fpData)!=1)
							return -8;

						if(fwrite(vpData, iDataLen, 1, m_fpData)!=1)
							return -9;

						if(fwrite(&I_STORE_END_MARK, sizeof(uint64_t), 1, m_fpData)!=1)
							return -10;

						if(fwrite(&vPosition, sizeof(int64_t), 1, m_fpHash)!=1)
							return -11;

						return 0;
					} else {
						if(fseek(m_fpHash, m_iElementLen-sizeof(uint64_t), SEEK_CUR)==-1)
							return -12;
					}
				}

				// 读取溢出桶号
				if(fread(&overflowBucketIndex, sizeof(int32_t), 1, m_fpHash)!=1)
					return -13;

				// 判断是否需要开辟新的溢出桶
				if(overflowBucketIndex==0) {
					++m_iBucketOverflowNum;

					bucketIndex=m_iBucketNum+m_iBucketOverflowNum;
					bucketPos=get_bucket_position(bucketIndex);

					// back 4 bytes
					if(fseek(m_fpHash, -4, SEEK_CUR)==-1)
						return -14;

					if(fwrite(&bucketIndex, sizeof(int32_t), 1, m_fpHash)!=1)
						return -15;

					if(fseek(m_fpHash, bucketPos, SEEK_SET)==-1)
						return -16;

					if(fwrite(m_pBucket, m_iBucketLen, 1, m_fpHash)!=1)
						return -17;
				} else {
					bucketPos=get_bucket_position(overflowBucketIndex);
				}
			}

			return 0;
		}

		// @函数名: 搜索变长值元素
		// @参数01: 元素键
		// @参数02: 元素值外部内存
		// @参数03: 元素值外部内存最大空间
		// @返回值: 成功返回元素值的实际长度, 失败返回<0的错误码
		int32_t searchKey_VL(uint64_t key, void* vpData, int32_t maxDataSize)
		{
			Q_ASSERT(m_isInitSystem==1, "QStoreManager: sure you have inited?");

			int32_t bucketIndex=(int32_t)(key%m_iBucketNum);
			int64_t bucketPos=get_bucket_position(bucketIndex);

			uint64_t readKey=0;
			uint8_t deleteFlag=0;
			int32_t overflowBucketIndex=0;

			uint64_t magic_mark=0;
			int64_t dataOffset=0;
			int32_t dataLen=0;

			Q_FOREVER {
				if(fseek(m_fpHash, bucketPos, SEEK_SET)==-1)
					return -1;

				for(int32_t i=0; i<m_iBucketSize; ++i) {
					if(fread(&readKey, sizeof(uint64_t), 1, m_fpHash)!=1)
						return -2;

					if(fread(&deleteFlag, sizeof(uint8_t), 1, m_fpHash)!=1)
						return -3;

					if(fread(&dataOffset, sizeof(int64_t), 1, m_fpHash)!=1)
						return -4;

					if(readKey==key&&deleteFlag==0) {
						if(fseek(m_fpData, dataOffset, SEEK_SET)==-1)
							return -5;

						if(fread(&magic_mark, sizeof(uint64_t), 1, m_fpData)!=1||magic_mark!=I_STORE_BEGIN_MARK)
							return -6;

						if(fread(&dataLen, sizeof(int32_t), 1, m_fpData)!=1)
							return -7;

						if(dataLen>maxDataSize||dataLen<=0) {
							Q_DEBUG("QStoreManager: maxDataSize (%d), dataLen (%d)", maxDataSize, dataLen);
							return -8;
						}

						if(fread(vpData, dataLen, 1, m_fpData)!=1)
							return -9;

						if(fread(&magic_mark, sizeof(uint64_t), 1, m_fpData)!=1||magic_mark!=I_STORE_END_MARK)
							return -10;

						return dataLen;
					}

					if(readKey==0)
						return -11;
				}

				// 读取溢出桶号
				if(fread(&overflowBucketIndex, sizeof(int32_t), 1, m_fpHash)!=1)
					return -12;

				if(overflowBucketIndex==0) {
					return -13;
				} else {
					bucketPos=get_bucket_position(overflowBucketIndex);
				}
			}

			return 0;
		}

		// @函数名: 删除变长值元素
		// @参数01: 元素键key
		// @返回值: 成功返回0, 失败返回<0的错误码, 不存在返回1
		int32_t deleteKey_VL(uint64_t key)
		{
			Q_ASSERT(m_isInitSystem==1, "QStoreManager: sure you have inited?");

			int32_t bucketIndex=(int32_t)(key%m_iBucketNum);
			int64_t bucketPos=get_bucket_position(bucketIndex);

			uint64_t readKey=0;
			int8_t deleteFlag=1;
			int32_t overflowBucketIndex=0;

			Q_FOREVER {
				if(fseek(m_fpHash, bucketPos, SEEK_SET)==-1)
					return -1;

				for(int32_t i=0; i<m_iBucketSize; ++i) {
					if(fread(&readKey, sizeof(uint64_t), 1, m_fpHash)!=1)
						return -2;

					if(readKey==key) {
						if(fwrite(&deleteFlag, sizeof(int8_t), 1, m_fpHash)!=1)
							return -3;
						return 0;
					}

					if(readKey==0) {
						return 1;
					} else {
						if(fseek(m_fpHash, m_iElementLen-sizeof(uint64_t), SEEK_CUR)==-1)
							return -4;
					}
				}

				// 读取溢出桶号
				if(fread(&overflowBucketIndex, sizeof(int32_t), 1, m_fpHash)!=1)
					return -5;

				if(overflowBucketIndex==0) {
					return -6;
				} else {
					bucketPos=get_bucket_position(overflowBucketIndex);
				}
			}

			return 0;
		}

	private:
		// @函数名: 获取散列文件中桶的位置
		int64_t get_bucket_position(int32_t bucketIndex)
		{
			return m_iHeadLen+bucketIndex*m_iBucketLen;
		}

	protected:
		uint64_t	I_STORE_FILE_MARK;	// 散列文件存储标识符
		uint64_t	I_STORE_BEGIN_MARK;	// 数据文件存储头标识符
		uint64_t	I_STORE_END_MARK;	// 数据文件存储尾标识符

		int32_t		m_iDataLen;		// 数据长度

		char		m_szHashFile[1<<7];	// 散列文件名称
		char		m_szDataFile[1<<7];	// 数据文件名称

		FILE*		m_fpHash;		// 散列文件句柄
		FILE*		m_fpData;		// 数据文件句柄

		int32_t		m_iAllDataNum;		// 元素数量

		int32_t		m_iBucketNum;		// 散列文件的桶数
		int32_t		m_iBucketOverflowNum;	// 散列文件溢出桶数量
		int32_t		m_iBucketSize;		// 散列文件每个桶的容量

		char*		m_pBucket;		// 散列文件的桶

		int32_t		m_iHeadLen;		// 散列文件头信息长度
		int32_t		m_iElementLen;		// 散列文件每个元素的长度
		int32_t		m_iBucketLen;		// 散列文件每个桶的长度

		int32_t		m_isInitSystem;		// 系统初始化标识符
};

Q_END_NAMESPACE

#endif // __QSTOREMANAGER_H_
