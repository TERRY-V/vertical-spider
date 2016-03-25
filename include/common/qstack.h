/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qstack.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/05/07
**
*********************************************************************************************/

#ifndef __QSTACK_H_
#define __QSTACK_H_

#include "qglobal.h"

Q_BEGIN_NAMESPACE

// 栈类
template <typename T_TYPE>
class QStack {
	public:
		inline QStack(int32_t size=1<<7) :
			max_size_(size),
			top_(-1)
		{
			data_=q_new_array<T_TYPE>(max_size_);
			Q_CHECK_PTR(data_);
		}

		inline ~QStack()
		{q_delete_array<T_TYPE>(data_);}

		inline int32_t max_size() const
		{return max_size_;}

		inline int32_t size() const
		{return top_+1;}

		inline bool empty() const
		{return (top_==-1);}

		inline void clear()
		{top_=-1;}

		bool push(const T_TYPE& item)
		{
			if(top_==max_size_-1)
				growStack(max_size_<<1);
			data_[++top_]=item;
			return true;
		}

		bool pop(T_TYPE& item)
		{
			if(top_==-1) return false;;
			item=data_[top_--];
			return true;
		}

	private:
		void growStack(int32_t size)
		{
			max_size_=(max_size_<size)?size:max_size_;

			T_TYPE* new_data_=q_new_array<T_TYPE>(max_size_);
			Q_CHECK_PTR(new_data_);

			int32_t n=top_+1;
			T_TYPE* srcptr=data_;
			T_TYPE* destptr=new_data_;
			while(n--) *destptr++=*srcptr++;

			q_delete_array<T_TYPE>(data_);
			data_=new_data_;
		}

	protected:
		T_TYPE* data_;
		int32_t max_size_;
		int32_t top_;
};

Q_END_NAMESPACE

#endif // __QSTACK_H_
