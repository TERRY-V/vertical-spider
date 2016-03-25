/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qqueue.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/05/07
**
*********************************************************************************************/

#ifndef __QQUEUE_H_
#define __QQUEUE_H_

#include "qglobal.h"

Q_BEGIN_NAMESPACE

// 线程安全循环队列类(最多存储max_size_-1个元素)
template <typename T_TYPE>
class QQueue {
	public:
		inline QQueue() :
			queue_(NULL),
			max_size_(0),
			front_(0),
			rear_(0)
		{}

		inline ~QQueue()
		{q_delete_array<T_TYPE>(queue_);}

		inline int32_t init(int32_t size)
		{
			if(size<=0)
				return -1;
			max_size_=size;
			queue_=q_new_array<T_TYPE>(max_size_);
			if(queue_==NULL)
				return -2;
			front_=rear_=0;
			return 0;
		}

		inline void clear()
		{
#ifdef __multi_thread
			mutex_.lock();
#endif
			front_=rear_=0;
#ifdef __multi_thread
			mutex_.unlock();
#endif
		}

		int32_t max_size() const
		{return max_size_;}

		int32_t size() const
		{return (rear_-front_+max_size_)%max_size_;}

		inline bool full()
		{return (rear_+1)%max_size_==front_;}

		inline bool empty()
		{return (front_==rear_);}

		void push(const T_TYPE& item)
		{
			Q_FOREVER {
#ifdef __multi_thread
				mutex_.lock();
#endif
				if(!full()) break;
#ifdef __multi_thread
				mutex_.unlock();
#endif
				q_sleep(1);
			}
			queue_[rear_]=item;
			rear_=(rear_+1)%max_size_;
#ifdef __multi_thread
			mutex_.unlock();
#endif
		}

		T_TYPE pop()
		{
			Q_FOREVER {
#ifdef __multi_thread
				mutex_.lock();
#endif
				if(!empty()) break;
#ifdef __multi_thread
				mutex_.unlock();
#endif
				q_sleep(1);
			}
			T_TYPE item=queue_[front_];
			front_=(front_+1)%max_size_;
#ifdef __multi_thread
			mutex_.unlock();
#endif
			return item;
		}

		int32_t push_non_blocking(const T_TYPE& item)
		{
#ifdef __multi_thread
			mutex_.lock();
#endif
			if(full()) {
#ifdef __multi_thread
				mutex_.unlock();
#endif
				return -1;
			}
			queue_[rear_]=item;
			rear_=(rear_+1)%max_size_;
#ifdef __multi_thread
			mutex_.unlock();
#endif
			return 0;
		}

		int32_t pop_non_blocking(T_TYPE& item)
		{
#ifdef __multi_thread
			mutex_.lock();
#endif
			if(empty()) {
#ifdef __multi_thread
				mutex_.unlock();
#endif
				return -1;
			}
			item=queue_[front_];
			front_=(front_+1)%max_size_;
#ifdef __multi_thread
			mutex_.unlock();
#endif
			return 0;
		}

	private:
		T_TYPE* queue_;
		int32_t max_size_;
		int32_t front_;
		int32_t rear_;
#ifdef __multi_thread
		QMutexLock mutex_;
#endif
};

// 线程安全链式队列类
template <typename T_TYPE>
class QListQueue {
	public:
		struct ListNode
		{
			T_TYPE data;
			ListNode* link;

			ListNode() :
				link(NULL)
			{}

			ListNode(T_TYPE d, ListNode* next=NULL) :
				data(d),
				link(next)
			{}
		};

		QListQueue(int32_t size=1024) :
			front_(NULL),
			rear_(NULL),
			now_size_(0),
			max_size_(size)
		{}

		virtual ~QListQueue()
		{clear();}

		void clear()
		{
			ListNode* p;
#ifdef __multi_thread
			mutex_.lock();
#endif
			while(front_!=NULL) {
				p=front_;
				front_=front_->link;
				q_delete<ListNode>(p);
				--now_size_;
			}
#ifdef __multi_thread
			mutex_.unlock();
#endif
		}

		int32_t push(const T_TYPE& item)
		{
			Q_FOREVER {
#ifdef __multi_thread
				mutex_.lock();
#endif
				if(!full()) break;
#ifdef __multi_thread
				mutex_.unlock();
#endif
				q_sleep(1);
			}
			if(front_==NULL) {
				front_=rear_=new(std::nothrow) ListNode(item);
				if(front_==NULL) {
#ifdef __multi_thread
					mutex_.unlock();
#endif
					return -1;
				}
			} else {
				rear_->link=new(std::nothrow) ListNode(item);
				if(rear_->link==NULL) {
#ifdef __multi_thread
					mutex_.unlock();
#endif
					return -2;
				}
#ifdef __multi_thread
				rear_=rear_->link;
#endif
			}
			++now_size_;
#ifdef __multi_thread
			mutex_.unlock();
#endif
			return 0;
		}

		int32_t push_urgent(const T_TYPE& item)
		{
			Q_FOREVER {
#ifdef __multi_thread
				mutex_.lock();
#endif
				if(!full()) break;
#ifdef __multi_thread
				mutex_.unlock();
#endif
				q_sleep(1);
			}
			if(front_==NULL) {
				front_=rear_=new(std::nothrow) ListNode(item);
				if(front_==NULL) {
#ifdef __multi_thread
					mutex_.unlock();
#endif
					return -1;
				}
			} else {
				ListNode* p=front_;
				front_=new(std::nothrow) ListNode(item, p);
				if(p==NULL) {
#ifdef __multi_thread
					mutex_.unlock();
#endif
					return -2;
				}
			}
			++now_size_;
#ifdef __multi_thread
			mutex_.unlock();
#endif
			return 0;
		}

		T_TYPE pop()
		{
			Q_FOREVER {
#ifdef __multi_thread
				mutex_.lock();
#endif
				if(!empty()) break;
#ifdef __multi_thread
				mutex_.unlock();
#endif
				q_sleep(1);
			}
			ListNode* p=front_;
			T_TYPE item=front_->data;
			front_=front_->link;
			q_delete<ListNode>(p);
			--now_size_;
#ifdef __multi_thread
			mutex_.unlock();
#endif
			return item;
		}

		int32_t size() const
		{return now_size_;}

		int32_t max_size() const
		{return max_size_;}

		bool empty() const
		{return now_size_==0;}

		bool full() const
		{return now_size_==max_size_;}

	protected:
		ListNode* front_;
		ListNode* rear_;
		int32_t now_size_;
		int32_t max_size_;
#ifdef __multi_thread
		QMutexLock mutex_;
#endif
};

// 数组优先级队列类, 时间复杂度较高, 建议选用heap堆作为优先级队列的存储结构
template <typename T_TYPE>
class QPriorityQueue {
	public:
		inline QPriorityQueue() :
			queue_(NULL),
			max_size_(0),
			size_(0)
		{}

		inline ~QPriorityQueue()
		{q_delete_array<T_TYPE>(queue_);}

		inline int32_t init(int32_t size)
		{
			if(size<=0)
				return -1;
			max_size_=size;
			queue_=q_new_array<T_TYPE>(max_size_);
			if(queue_==NULL)
				return -2;
			size_=0;
			return 0;
		}

		void push(const T_TYPE& item)
		{
			Q_FOREVER {
#ifdef __multi_thread
				mutex_.lock();
#endif
				if(!full()) break;
#ifdef __multi_thread
				mutex_.unlock();
#endif
				q_sleep(1);
			}
			queue_[size_++]=item;
			adjust();
#ifdef __multi_thread
			mutex_.unlock();
#endif
		}

		T_TYPE pop()
		{
			Q_FOREVER {
#ifdef __multi_thread
				mutex_.lock();
#endif
				if(!empty()) break;
#ifdef __multi_thread
				mutex_.unlock();
#endif
				q_sleep(1);
			}
			T_TYPE item=queue_[0];
			for(int32_t i=1; i<size_; ++i)
				queue_[i-1]=queue_[i];
			size_--;
#ifdef __multi_thread
			mutex_.unlock();
#endif
			return item;
		}

		int32_t push_non_blocking(const T_TYPE& item)
		{
#ifdef __multi_thread
			mutex_.lock();
#endif
			if(size_==max_size_) {
#ifdef __multi_thread
				mutex_.unlock();
#endif
				return -1;
			}
			queue_[size_++]=item;
			// 按优先权自动调整
			adjust();
#ifdef __multi_thread
			mutex_.unlock();
#endif
			return 0;
		}

		int32_t pop_non_blocking(T_TYPE& item)
		{
#ifdef __multi_thread
			mutex_.lock();
#endif
			if(empty()) {
#ifdef __multi_thread
				mutex_.unlock();
#endif
				return -1;
			}
			item=queue_[0];
			for(int32_t i=1; i<size_; ++i)
				queue_[i-1]=queue_[i];
			size_--;
#ifdef __multi_thread
			mutex_.unlock();
#endif
			return 0;
		}

		int32_t max_size() const
		{return max_size_;}

		int32_t size() const
		{return size_;}

		bool empty() const
		{return size_==0;}

		bool full() const
		{return size_==max_size_;}

		void clear()
		{size_=0;}

	private:
		// @函数名: 将队尾元素按其优先权大小调整到适当位置，保持所有元素按优先权从大到小排序
		void adjust()
		{
			T_TYPE temp=queue_[size_-1];
			int32_t j;
			for(j=size_-2; j>=0; --j) {
				if(queue_[j]>=temp) break;
				else queue_[j+1]=queue_[j];
			}
			queue_[j+1]=temp;
		}
		
	protected:
		T_TYPE* queue_;
		int32_t size_;
		int32_t max_size_;
#ifdef __multi_thread
		QMutexLock mutex_;
#endif
};

Q_END_NAMESPACE

#endif // __QQUEUE_H_
