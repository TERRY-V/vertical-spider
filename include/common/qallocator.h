/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qallocator.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/05/23
**
*********************************************************************************************/

#ifndef __QALLOCATOR_H_
#define __QALLOCATOR_H_

#include "qglobal.h"

Q_BEGIN_NAMESPACE

// 内存分配器
class QAllocator: public noncopyable {
	public:
		// @函数名: 内存分配器类构造函数
		// @参数01: 最大内存块数
		// @参数02: 每个内存块的最大长度
		inline QAllocator(int64_t maxBlockNum=2000, int64_t maxBlockLen=1<<20) : 
			mMaxBlockNum(maxBlockNum), 
			mMaxBlockLen(maxBlockLen)
		{
			pBlockLibrary=NULL;
			mNowBlockNum=-1;
			mNowBlockLen=mMaxBlockLen;
			mTotalLen=0;
		}

		// @函数名: 析构函数
		virtual ~QAllocator();

		// @函数名: 内存分配函数, 成功返回分配头指针，失败返回NULL值
		char* alloc(int64_t allocLen=1<<10);

		// @函数名: 获取当前已分配的长度
		int64_t getAllocLength() const;

		// @函数名: 获取当前已使用buffer长度
		int64_t getBufferLength() const;

		// @函数名: 重置分配器
		void resetAllocator();

		// @函数名: 释放分配器
		int64_t freeAllocator();

	protected:
		char**		pBlockLibrary;
		int64_t		mMaxBlockNum;
		int64_t		mMaxBlockLen;
		int64_t		mNowBlockNum;
		int64_t		mNowBlockLen;
		int64_t		mTotalLen;
};

// 内存池分配器
class QPoolAllocator: public noncopyable {
	public:
		// @函数名: 内存分配器类构造函数
		inline QPoolAllocator() :
			pBlockLibrary(NULL),
			pRecycleLibrary(NULL),
			mMaxChunkNum(0),
			mNowChunkNum(-1),
			mMaxBlockLen(0),
			mNowBlockLen(-1),
			mChunkSize(0),
			mMaxBlockNum(0),
			mNowBlockNum(-1),
			mRecycleNum(0),
			mAllocNum(0)
		{}

		virtual ~QPoolAllocator();

		// @函数名: 初始化函数
		// @参数01: 每个chunk的长度
		// @参数02: 最大block数
		// @参数03: 每个block的chunk数目
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t init(int32_t lChunkSize=1<<20, int64_t lMaxBlockNum=1000, int64_t lChunkNumPerBlock=1000);

		// @函数名: 内存分配函数, 成功返回首指针, 失败返回NULL
		char* alloc();

		// @函数名: 释放内存
		void free(char* buffer);

		// @函数名: 重置分配器
		void resetAllocator();

		// @函数名: 释放分配器
		void freeAllocator();

		// @函数名: 跟踪内存分配器
		void trace(const char* name);

		// @函数名: 获取chunk的大小
		int64_t getChunkSize() const;

	private:
		char**		pBlockLibrary;
		char*		pRecycleLibrary;
		QMutexLock	mMutex;
		int64_t		mMaxChunkNum;
		int64_t		mNowChunkNum;
		int64_t		mMaxBlockLen;
		int64_t		mNowBlockLen;
		int64_t		mChunkSize;
		int64_t		mMaxBlockNum;
		int64_t		mNowBlockNum;
		int64_t 	mRecycleNum;
		int64_t		mAllocNum;
};

Q_END_NAMESPACE

#endif // __QALLOCATOR_H_
