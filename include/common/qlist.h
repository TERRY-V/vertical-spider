/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qlist.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/05/07
**
*********************************************************************************************/

#ifndef __QLIST_H_
#define __QLIST_H_

#include "qglobal.h"
#include "qallocator.h"
#include "qvector.h"

Q_BEGIN_NAMESPACE

template <typename T_TYPE>
class QList {
	public:
		struct NodeInfo {
			T_TYPE data_;
			NodeInfo *lLink_, *rLink_;

			NodeInfo(NodeInfo *left=NULL, NodeInfo *right=NULL) :
				lLink_(left), 
				rLink_(right)
			{}

			NodeInfo(T_TYPE value, NodeInfo *left=NULL, NodeInfo *right=NULL) :
				data_(value),
				lLink_(left),
				rLink_(right)
			{}
		};

		inline QList() :
			size_(0)
		{
			first_=new(std::nothrow) NodeInfo();
			Q_ASSERT(first_!=NULL, "QList: bad allocate, first_ = null!");
			first_->rLink_=first_->lLink_=first_;
			current_=first_;
		}

		inline QList(const T_TYPE uniqueVal) :
			size_(0)
		{
			first_=new(std::nothrow) NodeInfo(uniqueVal);
			Q_ASSERT(first_!=NULL, "QList: bad allocate, first_ = null!");
			first_->rLink_=first_->lLink_=first_;
			current_=first_;
		}

		virtual ~QList()
		{
			clear();
			delete first_;
			first_=NULL;
			current_=NULL;
		}

		void push_back(const T_TYPE& x)
		{
			NodeInfo* newNode=new(std::nothrow) NodeInfo(x);
			Q_ASSERT(newNode!=NULL, "QList: push_back alloc error!");
			newNode->lLink_=first_->lLink_;
			first_->lLink_=newNode;
			newNode->lLink_->rLink_=newNode;
			newNode->rLink_=first_;
			size_++;
		}

		void push_front(const T_TYPE& x)
		{
			NodeInfo* newNode=new(std::nothrow) NodeInfo(x);
			Q_ASSERT(newNode!=NULL, "QList: push_front alloc error!");
			newNode->rLink_=first_->rLink_;
			first_->rLink_=newNode;
			newNode->rLink_->lLink_=newNode;
			newNode->lLink_=first_;
			size_++;
		}

		void prepareTraversal()
		{current_=first_;}

		bool hasNext()
		{
			if(current_==NULL) return false;
			return (current_->rLink_!=first_?true:false);
		}

		T_TYPE next()
		{
			Q_ASSERT(current_->rLink_!=first_, "QList: next is null!");
			current_=current_->rLink_;
			return current_->data_;
		}

		bool hasPrev()
		{
			if(current_==NULL) return false;
			return (current_->lLink_!=first_?true:false);
		}

		T_TYPE prev()
		{
			Q_ASSERT(current_->lLink_!=first_, "QList: prev is null!");
			current_=current_->lLink_;
			return current_->data_;
		}

		QVector<T_TYPE> to_vector()
		{
			QVector<T_TYPE> elems;
			prepareTraversal();
			while(hasNext()) {
				T_TYPE item=next();
				elems.push_back(item);
			}
			return elems;
		}

		int32_t size() const
		{return size_;}

		void clear()
		{while(!empty()) erase();}

		bool empty()
		{return first_->rLink_==first_;}

		void erase()
		{
			if(current_!=NULL) {
				NodeInfo* temp=current_;
				current_=current_->rLink_;
				current_->lLink_=temp->lLink_;
				temp->lLink_->rLink_=current_;
				delete temp;
				temp=NULL;
				if(current_==first_) {
					if(empty()) current_=NULL;
					else current_=current_->rLink_;
				}
				size_--;
			}
		}

	protected:
		int32_t size_;
		NodeInfo* first_;
		NodeInfo* current_;
};

///////////////////////////////////////////////////////////////////////////////////////////

template <typename T_TYPE>
class QForwardList {
	public:
		struct NodeInfo {
			T_TYPE data_;
			NodeInfo* next_;

			NodeInfo(NodeInfo* next=NULL) :
				next_(next)
			{}

			NodeInfo(T_TYPE key, NodeInfo *next=NULL) :
				data_(key),
				next_(next)
			{}
		};

	public:
		QForwardList() :
			head_(0),
			size_(0)
		{}

		virtual ~QForwardList()
		{}

		int32_t init(int32_t max_size_)
		{
			int32_t max=BUFSIZ_1M/sizeof(NodeInfo);
			int32_t ret=allocator_.init(sizeof(NodeInfo), max_size_, max);
			return ret;
		}

		int32_t get(T_TYPE& data)
		{
			mutex_.lock();
			if(head_==NULL) {
				mutex_.unlock();
				return -1;
			}
			data=head_->data_;
			head_=head_->next_;
			size_--;
			mutex_.unlock();
			return 0;
		}

		int32_t add(const T_TYPE& data)
		{
			mutex_.lock();
			NodeInfo* node=(NodeInfo*)allocator_.alloc();
			if(node==NULL) {
				mutex_.unlock();
				return -1;
			}

			node->data_=data;
			node->next_=head_;
			size_++;
			head_=node;

			mutex_.unlock();
			return 0;
		}

		int32_t size()
		{
			mutex_.lock();
			int32_t num=size_;
			mutex_.unlock();
			return num;
		}

	protected:
		QPoolAllocator allocator_;
		NodeInfo* head_;
		QMutexLock mutex_;
		int32_t size_;
};

Q_END_NAMESPACE

#endif // __QLIST_H_
