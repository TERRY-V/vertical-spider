/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qvector.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/05/07
**
*********************************************************************************************/

#ifndef __QVECTOR_H_
#define __QVECTOR_H_

#include "qglobal.h"
#include "qalgorithm.h"

Q_BEGIN_NAMESPACE

template <typename T_TYPE>
class QVector {
	public:
		inline explicit QVector(int32_t size=100)
		{
			Q_ASSERT(size>0, "QVector: size error, size = (%d)", size);
			max_size_=size;
			last_=-1;
			data_=q_new_array<T_TYPE>(max_size_);
			Q_CHECK_PTR(data_);
		}

		virtual ~QVector()
		{q_delete_array<T_TYPE>(data_);}

		int32_t max_size() const
		{return max_size_;}

		int32_t size() const
		{return last_+1;}

		int32_t search(T_TYPE& item) const
		{
			for(int32_t i=0; i<=last_; ++i)
				if(data_[i]==item) return i;
			return -1;
		}

		T_TYPE& at(int32_t pos) const
		{
			Q_ASSERT(pos>=0&&pos<=last_, "QVector: out of range, pos = (%d)", pos);
			return data_[pos];
		}

		void assign(int32_t pos, T_TYPE& item)
		{
			Q_ASSERT(pos>=0&&pos<=last_, "QVector: out of range, pos = (%d)", pos);
			data_[pos]=item;
		}

		void push_back(T_TYPE& item)
		{
			if(last_==max_size_-1)
				resize(max_size_<<1);
			data_[++last_]=item;
		}

		void insert(int32_t pos, T_TYPE& item)
		{
			Q_ASSERT(pos>=0&&pos<=last_, "QVector: out of range, pos = (%d)", pos);
			if(last_==max_size_-1)
				resize(max_size_<<1);;
			for(int32_t j=last_; j>pos; j--)
				data_[j+1]=data_[j];
			data_[pos+1]=item;
			last_++;
		}

		void erase(int32_t pos)
		{
			Q_ASSERT(pos>=0&&pos<=last_, "QVector: out of range, pos = (%d)", pos);
			for(int32_t j=pos; j<last_; j++)
				data_[j]=data_[j+1];
			last_--;
		}

		bool empty() const
		{return last_==-1;}

		void clear()
		{last_=-1;}

		T_TYPE& operator[](int32_t pos)
		{
			Q_ASSERT(pos>=0&&pos<=last_, "QVector: out of range, pos = (%d)", pos);
			return data_[pos];
		}

		void sort()
		{if(size()>1) Q_Quick_Sort(data_, 0, size()-1);}

	private:
		void resize(int32_t size)
		{
			if(size>max_size_) {
				T_TYPE* new_array_=q_new_array<T_TYPE>(size);
				Q_CHECK_PTR(new_array_);

				int32_t n=last_+1;
				T_TYPE* srcptr=data_;
				T_TYPE* destptr=new_array_;
				while(n--) *destptr++=*srcptr++;

				q_delete_array<T_TYPE>(data_);
				data_=new_array_;
				max_size_=size;
			}
		}

	protected:
		T_TYPE* data_;
		int32_t max_size_;
		int32_t last_;
};

Q_END_NAMESPACE

#endif // __QVECTOR_H_
