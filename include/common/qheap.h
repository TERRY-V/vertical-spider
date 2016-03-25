/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qheap_.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/05/10
**
*********************************************************************************************/

#ifndef __QHEAP_H_
#define __QHEAP_H_

#include "qglobal.h"

Q_BEGIN_NAMESPACE

// 最小堆类定义
template <typename T_TYPE>
class QMinHeap {
	public:
		explicit QMinHeap(int32_t size=100) :
			max_size_(size),
			size_(0)
		{
			heap_=q_new_array<T_TYPE>(max_size_);
			Q_ASSERT(heap_!=NULL, "QMinHeap: bad allocate, heap_ is null");
		}

		virtual ~QMinHeap()
		{q_delete_array<T_TYPE>(heap_);}

		inline int32_t size() const
		{return size_;}

		inline int32_t max_size() const
		{return max_size_;}

		void push(const T_TYPE& x)
		{
			Q_FOREVER {
				mutex_.lock();
				if(!full()) break;
				mutex_.unlock();
				q_sleep(1000);
			}
			heap_[size_]=x;
			siftUp(size_);
			size_++;
			mutex_.unlock();
		}
		
		T_TYPE pop()
		{
			Q_FOREVER {
				mutex_.lock();
				if(!empty()) break;
				mutex_.unlock();
				q_sleep(1000);
			}
			T_TYPE x=heap_[0];
			heap_[0]=heap_[size_-1];
			size_--;
			siftDown(0, size_-1);
			mutex_.unlock();
			return x;
		}

		int32_t push_non_blocking(const T_TYPE& x)
		{
			mutex_.lock();
			if(full()) {
				mutex_.unlock();
				return -1;
			}
			heap_[size_]=x;
			siftUp(size_);
			size_++;
			mutex_.unlock();
			return 0;
		}

		int32_t pop_non_blocking(T_TYPE& x)
		{
			mutex_.lock();
			if(empty()) {
				mutex_.unlock();
				return -1;
			}
			x=heap_[0];
			heap_[0]=heap_[size_-1];
			size_--;
			siftDown(0, size_-1);
			mutex_.unlock();
			return 0;
		}

		inline bool empty() const
		{return size_==0;}

		inline bool full() const
		{return size_==max_size_;}

		void clear()
		{
			mutex_.lock();
			size_=0;
			mutex_.unlock();
		}

	private:
		// 从start到0上滑调整成为最小堆
		void siftUp(int32_t start)
		{
			int32_t j=start, i=(j-1)/2;
			T_TYPE temp=heap_[j];
			while(j>0) {
				if(heap_[i]<=temp) break;
				heap_[j]=heap_[i];
				j=i;
				i=(i-1)/2;
			}
			heap_[j]=temp;
		}

		// 从start到m下滑调整成为最小堆
		void siftDown(int32_t start, int32_t m)
		{
			int32_t i=start, j=2*i+1;
			T_TYPE temp=heap_[i];
			while(j<=m) {
				if(j<m&&heap_[j]>heap_[j+1]) j++;
				if(temp<=heap_[j]) break;
				heap_[i]=heap_[j];
				i=j;
				j=2*j+1;
			}
			heap_[i]=temp;
		}

	protected:
		// 存放最小堆中元素的数组
		T_TYPE* heap_;
		// 最小堆中当前元素个数
		int32_t size_;
		// 最小堆最多运行元素个数
		int32_t max_size_;
		// 互斥锁
		QMutexLock mutex_;
};

// 最大堆的类定义
template <typename T_TYPE>
class QMaxHeap {
	public:
		explicit QMaxHeap(int32_t size=100) :
			max_size_(size),
			size_(0)
		{
			heap_=q_new_array<T_TYPE>(max_size_);
			Q_ASSERT(heap_!=NULL, "QMinHeap: bad allocate, heap_ is null");
		}

		virtual ~QMaxHeap()
		{q_delete_array<T_TYPE>(heap_);}

		inline int32_t size() const
		{return size_;}

		inline int32_t max_size() const
		{return max_size_;}

		void push(const T_TYPE& x)
		{
			Q_FOREVER {
				mutex_.lock();
				if(!full()) break;
				mutex_.unlock();
				q_sleep(1000);
			}
			heap_[size_]=x;
			siftUp(size_);
			size_++;
			mutex_.unlock();
		}
		
		T_TYPE pop()
		{
			Q_FOREVER {
				mutex_.lock();
				if(!empty()) break;
				mutex_.unlock();
				q_sleep(1000);
			}
			T_TYPE x=heap_[0];
			heap_[0]=heap_[size_-1];
			size_--;
			siftDown(0, size_-1);
			mutex_.unlock();
			return x;
		}

		int32_t push_non_blocking(const T_TYPE& x)
		{
			mutex_.lock();
			if(full()) {
				mutex_.unlock();
				return -1;
			}
			heap_[size_]=x;
			siftUp(size_);
			size_++;
			mutex_.unlock();
			return 0;
		}

		int32_t pop_non_blocking(T_TYPE& x)
		{
			mutex_.lock();
			if(empty()) {
				mutex_.unlock();
				return -1;
			}
			x=heap_[0];
			heap_[0]=heap_[size_-1];
			size_--;
			siftDown(0, size_-1);
			mutex_.unlock();
			return 0;
		}

		inline bool empty() const
		{return size_==0;}

		inline bool full() const
		{return size_==max_size_;}

		void clear()
		{
			mutex_.lock();
			size_=0;
			mutex_.unlock();
		}

	private:
		// 从start到0上滑调整成为最大堆
		void siftUp(int32_t start)
		{
			int32_t j=start, i=(j-1)/2;
			T_TYPE temp=heap_[j];
			while(j>0) {
				if(heap_[i]>=temp) break;
				heap_[j]=heap_[i];
				j=i;
				i=(i-1)/2;
			}
			heap_[j]=temp;
		}

		// 从start到m下滑调整成为最大堆
		void siftDown(int32_t start, int32_t m)
		{
			int32_t i=start, j=2*i+1;
			T_TYPE temp=heap_[i];
			while(j<=m) {
				if(j<m&&heap_[j]<heap_[j+1]) j++;
				if(temp>=heap_[j]) break;
				heap_[i]=heap_[j];
				i=j;
				j=2*j+1;
			}
			heap_[i]=temp;
		}

	protected:
		// 存放最大堆中元素的数组
		T_TYPE* heap_;
		// 最大堆中当前元素个数
		int32_t size_;
		// 最大堆最多运行元素个数
		int32_t max_size_;
		// 互斥锁
		QMutexLock mutex_;
};

Q_END_NAMESPACE

#endif // __QHEAP_H_
