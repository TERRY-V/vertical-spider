/********************************************************************************************
**
** Copyright (C) 2010-2015 Terry Niu (Beijing, China)
** Filename:	qdenseindexmanager.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2015/02/13
**
*********************************************************************************************/

#ifndef __QDENSEINDEXMANAGER_H_
#define __QDENSEINDEXMANAGER_H_

#include "qglobal.h"
#include "qalgorithm.h"

Q_BEGIN_NAMESPACE

// QDenseIndexManager稠密索引类
// 稠密索引释义: 一个索引项对应数据表中的一个记录, 采用索引文件实现存储和检索.
// 索引文件由搜索表和数据表组成, 数据表存放文件中所有的数据记录, 索引表由索引项组成。
// 每个索引项索引一条数据记录, 保存该数据记录的关键码key和它在文件数据表中的开始地址.

class QDenseIndexManager {
	public:
		// @函数名: 稠密索引类构造函数
		QDenseIndexManager() :
			I_STORE_INDEX_MARK(*(uint64_t*)"@QDIM1.0"),
			I_STORE_BEGIN_MARK(0xBBBBBBBBBBBBBBBB),
			I_STORE_END_MARK(0xEEEEEEEEEEEEEEEE),
			m_pszKey(NULL),
			m_pszOffset(NULL),
			m_pszFlag(NULL),
			m_iMaxIndexSize(0),
			m_iNowIndexNum(0),
			m_fpIndex(NULL),
			m_fpData(NULL),
			m_isInitSystem(0)
		{}

		// @函数名: 稠密索引类析构函数
		virtual ~QDenseIndexManager()
		{
			if(m_pszKey)
				q_delete_array<uint64_t>(m_pszKey);
			if(m_pszOffset)
				q_delete_array<int64_t>(m_pszOffset);
			if(m_pszFlag)
				q_delete_array<uint8_t>(m_pszFlag);

			if(m_fpIndex)
				fclose(m_fpIndex);
			if(m_fpData)
				fclose(m_fpData);
		}

		// @函数名: 初始化函数
		// @参数01: 索引文件名称
		// @参数02: 索引表的最大索引数
		// @返回值: 成功返回0, 失败返回小于0的错误码
		int32_t init(const char* name, int32_t maxIndexSize=1<<20)
		{
			if(name==NULL||*name==0)
				return -1;

			if(q_snprintf(m_szIndexFile, sizeof(m_szIndexFile)-1, "%s.dim", name)<0)
				return -2;

			if(q_snprintf(m_szDataFile, sizeof(m_szDataFile)-1, "%s.dim.dat", name)<0)
				return -3;

			if(access(m_szIndexFile, 0)==0) {
				m_fpIndex=fopen(m_szIndexFile, "rb+");
				if(m_fpIndex==NULL) {
					Q_DEBUG("QDenseIndexManager: fopen (%s) error!", m_szIndexFile);
					return -4;
				}

				m_fpData=fopen(m_szDataFile, "rb+");
				if(m_fpData==NULL) {
					Q_DEBUG("QDenseIndexManager: fopen (%s) error!", m_szDataFile);
					return -5;
				}

				uint64_t magic_mark;
				if(fread(&magic_mark, sizeof(uint64_t), 1, m_fpIndex)!=1||magic_mark!=I_STORE_INDEX_MARK)
					return -6;

				if(fread(&m_iMaxIndexSize, sizeof(int32_t), 1, m_fpIndex)!=1)
					return -7;

				if(fread(&m_iNowIndexNum, sizeof(int32_t), 1, m_fpIndex)!=1)
					return -8;

				m_pszKey=q_new_array<uint64_t>(m_iMaxIndexSize);
				if(m_pszKey==NULL)
					return -9;

				m_pszOffset=q_new_array<int64_t>(m_iMaxIndexSize);
				if(m_pszOffset==NULL)
					return -10;

				m_pszFlag=q_new_array<uint8_t>(m_iMaxIndexSize);
				if(m_pszOffset==NULL)
					return -11;

				if(fread(m_pszKey, sizeof(uint64_t)*m_iNowIndexNum, 1, m_fpIndex)!=1)
					return -12;

				if(fread(m_pszOffset, sizeof(int64_t)*m_iNowIndexNum, 1, m_fpIndex)!=1)
					return -13;

				if(fread(m_pszFlag, sizeof(uint8_t)*m_iNowIndexNum, 1, m_fpIndex)!=1)
					return -14;

				if(fread(&magic_mark, sizeof(uint64_t), 1, m_fpIndex)!=1||magic_mark!=I_STORE_INDEX_MARK)
					return -15;
			} else {
				m_iMaxIndexSize=maxIndexSize;
				m_iNowIndexNum=0;

				m_fpIndex=fopen(m_szIndexFile, "w+");
				if(m_fpIndex==NULL) {
					Q_DEBUG("QDenseIndexManager: fopen (%s) error!", m_szIndexFile);
					return -5;
				}

				m_fpData=fopen(m_szDataFile, "w+");
				if(m_fpData==NULL) {
					Q_DEBUG("QDenseIndexManager: fopen (%s) error!", m_szDataFile);
					return -6;
				}

				m_pszKey=q_new_array<uint64_t>(m_iMaxIndexSize);
				if(m_pszKey==NULL)
					return -7;

				m_pszOffset=q_new_array<int64_t>(m_iMaxIndexSize);
				if(m_pszOffset==NULL)
					return -8;

				m_pszFlag=q_new_array<uint8_t>(m_iMaxIndexSize);
				if(m_pszOffset==NULL)
					return -9;
			}

			m_isInitSystem=1;

			return 0;
		}

		// @函数名: 新增加元素
		// @参数01: 元素键key
		// @参数02: 元素值value
		// @参数03: 元素值value长度
		// @返回值: 成功返回0, 失败返回<0的错误码, 重复键返回1
		int32_t addKey(uint64_t key, void* vpData, int32_t iDataLen)
		{
			if(vpData==NULL||iDataLen<=0||m_fpData==NULL||m_isInitSystem==0)
				return -1;

			if(m_iNowIndexNum+1>=m_iMaxIndexSize)
				return -2;

			if(fseek(m_fpData, 0, SEEK_END)==-1)
				return -3;
			int64_t offset=ftell(m_fpData);

			if(fwrite(&I_STORE_BEGIN_MARK, sizeof(uint64_t), 1, m_fpData)!=1)
				return -4;

			if(fwrite(&iDataLen, sizeof(int32_t), 1, m_fpData)!=1)
				return -5;

			if(fwrite(vpData, iDataLen, 1, m_fpData)!=1)
				return -6;

			if(fwrite(&I_STORE_END_MARK, sizeof(uint64_t), 1, m_fpData)!=1)
				return -7;

			m_pszKey[m_iNowIndexNum]=key;
			m_pszOffset[m_iNowIndexNum]=offset;
			m_pszFlag[m_iNowIndexNum]=0;

			++m_iNowIndexNum;

			return 0;
		}

		// @函数名: 索引项重排
		int32_t rearrangeIndex()
		{
			if(m_isInitSystem==0)
				return -1;

			// 多关键码索引项快排
			Q_Recursion_1K_2P<uint64_t, int64_t, uint8_t>(0, m_iNowIndexNum-1, m_pszKey, m_pszOffset, m_pszFlag);

			// 更新索引文件
			if(fseek(m_fpIndex, 0, SEEK_SET)==-1)
				return -2;

			if(fwrite(&I_STORE_INDEX_MARK, sizeof(uint64_t), 1, m_fpIndex)!=1)
				return -3;

			if(fwrite(&m_iMaxIndexSize, sizeof(int32_t), 1, m_fpIndex)!=1)
				return -4;
			
			if(fwrite(&m_iNowIndexNum, sizeof(uint32_t), 1, m_fpIndex)!=1)
				return -5;

			if(fwrite(m_pszKey, sizeof(uint64_t)*m_iNowIndexNum, 1, m_fpIndex)!=1)
				return -6;

			if(fwrite(m_pszOffset, sizeof(int64_t)*m_iNowIndexNum, 1, m_fpIndex)!=1)
				return -7;

			if(fwrite(m_pszFlag, sizeof(uint8_t)*m_iNowIndexNum, 1, m_fpIndex)!=1)
				return -8;

			if(fwrite(&I_STORE_INDEX_MARK, sizeof(uint64_t), 1, m_fpIndex)!=1)
				return -9;

			return 0;
		}

		// @函数名: 搜索元素
		// @参数01: 元素键
		// @参数02: 元素值外部内存
		// @参数03: 元素值外部内存最大空间
		// @返回值: 成功返回元素值的实际长度, 失败返回<0的错误码
		int32_t searchKey(uint64_t key, void* vpData, int32_t maxDataSize)
		{
			if(vpData==NULL||maxDataSize<=0||m_isInitSystem==0)
				return -1;

			char head[12];
			int32_t index=-1;
			uint64_t end_mark=0;

			index=Q_Binary_Search_Default<uint64_t>(0, m_iNowIndexNum-1, m_pszKey, key);
			if(index<0)
				return -2;

			if(m_pszFlag[index]==1)
				return -3;

			if(fseek(m_fpData, m_pszOffset[index], SEEK_SET)==-1)
				return -4;

			if(fread(head, sizeof(head), 1, m_fpData)!=1)
				return -5;

			if(*(uint64_t*)head!=I_STORE_BEGIN_MARK)
				return -6;

			if(*(int32_t*)(head+sizeof(uint64_t))>maxDataSize||*(int32_t*)(head+sizeof(uint64_t))<=0)
				return -7;

			if(fread(vpData, *(int32_t*)(head+8), 1, m_fpData)!=1)
				return -8;

			if(fread(&end_mark, sizeof(end_mark), 1, m_fpData)!=1||end_mark!=I_STORE_END_MARK)
				return -9;

			return *(int32_t*)(head+sizeof(uint64_t));
		}

		// @函数名: 删除元素
		// @参数01: 元素键key
		// @返回值: 成功返回0, 失败返回<0的错误码, 不存在返回1
		int32_t deleteKey(uint64_t key)
		{
			if(m_isInitSystem==0)
				return -1;

			int32_t index=Q_Binary_Search_Default<uint64_t>(0, m_iNowIndexNum-1, m_pszKey, key);
			if(index<0)
				return -2;

			m_pszFlag[index]=1;
			return 0;
		}

	protected:
		uint64_t	I_STORE_INDEX_MARK;	// 索引文件标识符
		uint64_t	I_STORE_BEGIN_MARK;	// 数据文件记录头标识符
		uint64_t	I_STORE_END_MARK;	// 数据文件记录尾标识符

		char		m_szIndexFile[1<<7];	// 索引文件名称
		char		m_szDataFile[1<<7];	// 数据文件名称

		uint64_t*	m_pszKey;		// 索引项信息之索引项key
		int64_t*	m_pszOffset;		// 索引项信息之索引项在文件数据表的起始位置
		uint8_t*	m_pszFlag;		// 索引项信息之索引项删除标识

		int32_t		m_iMaxIndexSize;	// 当前内存最大索引数目
		int32_t		m_iNowIndexNum;		// 当前内存实际索引数目

		FILE*		m_fpIndex;		// 索引文件句柄
		FILE*		m_fpData;		// 数据文件句柄

		int32_t		m_isInitSystem;		// 系统初始化标识符
};

Q_END_NAMESPACE

#endif // __QDENSEINDEXMANAGER_H_
