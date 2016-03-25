/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qobjectpool.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/05/10
**
*********************************************************************************************/

#ifndef __QOBJECTPOOL_H_
#define __QOBJECTPOOL_H_

#include "qglobal.h"

Q_BEGIN_NAMESPACE

// 对象池类，避免在程序中创建和删除大量对象
template <typename T_TYPE>
class QObjectPool {
	public:
		QObjectPool(uint32_t chunkSize=101)
		{
			Q_ASSERT(chunkSize>0, "QObjectPool chunkSize = (%d)", chunkSize);
			mChunkSize=chunkSize;
			allocateChunk();
		}

		virtual ~QObjectPool()
		{clear();}

		T_TYPE* acquireObject()
		{
			mMutexLock.lock();
			if(mFreeList.empty())
				allocateChunk();
			T_TYPE* object=mFreeList.front();
			mFreeList.pop();
			mMutexLock.unlock();
			return object;
		}

		void releaseObject(T_TYPE* object)
		{
			mMutexLock.lock();
			mFreeList.push(object);
			mMutexLock.unlock();
		}

		void clear()
		{
			mMutexLock.lock();
			while(!mFreeList.empty()) {
				T_TYPE* object=mFreeList.front();
				mFreeList.pop();
				delete object;
			}
			mMutexLock.unlock();
		}

	private:
		void allocateChunk() throw(std::bad_alloc)
		{
			for(uint32_t i=0; i<mChunkSize; ++i)
				mFreeList.push(new T_TYPE());
		}

		QObjectPool(const QObjectPool& src);
		QObjectPool<T_TYPE>& operator=(const QObjectPool<T_TYPE>& rhs);

	private:
		std::queue<T_TYPE*> mFreeList;
		uint32_t mChunkSize;
		QMutexLock mMutexLock;
};

Q_END_NAMESPACE

#endif // __QOBJECTPOOL_H_
