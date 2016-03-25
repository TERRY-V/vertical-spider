/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qbytearray.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/05/12
**
*********************************************************************************************/

#ifndef __QBYTEARRAY_H_
#define __QBYTEARRAY_H_

#include "qglobal.h"

Q_BEGIN_NAMESPACE

class QByteArray {
	public:
		class out_of_range: public std::exception {
			public:
				out_of_range(uint32_t p, uint32_t l, uint32_t s)
				{
					q_snprintf(errmsg_, MAX_ERROR_MSG_LEN, \
							"current position: (%u), field length: (%u), " \
							"out of range size (%u)", p, l, s);
				}

				virtual ~out_of_range() throw()
				{}

				virtual const char* what() const throw()
				{return errmsg_;}

			private:
				static const uint32_t MAX_ERROR_MSG_LEN=256;
				char errmsg_[MAX_ERROR_MSG_LEN];
		};

	public:
		explicit QByteArray() :
			data_(NULL),
			size_(0),
			position_(0),
			own_(true)
		{}

		explicit QByteArray(uint32_t size): data_(NULL), size_(size), position_(0), own_(true)
		{if(size_>0) data_=allocate(size_);}

		explicit QByteArray(const QByteArray& rhs): data_(NULL), size_(0), position_(0), own_(true)
		{reset(); copy(rhs.data_, 0, rhs.size_);}

		virtual ~QByteArray()
		{free();}

		QByteArray& assign(const char* data, uint32_t offset, uint32_t size)
		{
			reset();
			copy(data, offset, size);
			return *this;
		}

		// Caution!
		QByteArray& wrap(char* data, uint32_t offset, uint32_t size)
		{
			reset();
			own_=false;
			data_=data+offset;
			size_=size;
			return *this;
		}

		QByteArray& operator=(const QByteArray& rhs)
		{
			if(this==&rhs) return *this;
			else {
				reset();
				copy(rhs.data_, 0, rhs.size_);
			}
			return *this;
		}

		template<typename T> QByteArray& put(const T& e) throw(out_of_range)
		{
			if(position_+sizeof(T)>size_)
				throw out_of_range(position_, sizeof(T), size_);
			memcpy(data_+position_, &e, sizeof(T));
			position_+=sizeof(T);
			return *this;
		}

		template<typename T> QByteArray& get(T& e) throw(out_of_range)
		{
			if(position_+sizeof(T)>size_)
				throw out_of_range(position_, sizeof(T), size_);
			peek(e);
			position_+=sizeof(T);
			return *this;
		}

		template<typename T> QByteArray& operator<<(const T& e) throw(out_of_range)
		{return put(e);}

		template<typename T> QByteArray& operator>>(T& e) throw(out_of_range)
		{return get(e);}

		// peek doesn't change position value
		template<typename T> const QByteArray& peek(T& e) const throw(out_of_range)
		{
			if(position_+sizeof(T)>size_)
				throw out_of_range(position_, sizeof(T), size_);
			memcpy(&e, data_+position_, sizeof(T));
			return *this;
		}

		template<typename T> T get() throw(out_of_range)
		{
			T e;
			get(e);
			return e;
		}

		QByteArray& put(const std::string& v) throw(out_of_range)
		{
			put(static_cast<uint32_t>(v.length())+1);
			put(v.c_str(), 0, v.length()+1);
			return *this;
		}

		QByteArray& get(std::string& v) throw(out_of_range)
		{
			uint32_t size=0;
			get(size);
			if(size>0) {
				char* data=new(std::nothrow) char[size];
				Q_ASSERT(data!=NULL, "QByteArray::getString allocate error, data is null!");
				get(data, 0, size);
				v.assign(data);
				delete [] data;
			}
			return *this;
		}

		virtual QByteArray& put(const char* src, uint32_t offset, uint32_t size) throw(out_of_range)
		{
			if(position_+size>size_) throw out_of_range(position_, size, size_);
			memcpy(data_+position_, src+offset, size);
			position_+=size;
			return *this;
		}

		virtual QByteArray& get(char* dst, uint32_t offset, uint32_t size) throw(out_of_range)
		{
			if(position_+size>size_)
				throw out_of_range(position_, size, size_);
			memcpy(dst+offset, data_+position_, size);
			position_+=size;
			return *this;
		}

		QByteArray& getRef(int32_t index, const char* &dst, uint32_t size) throw(out_of_range)
		{
			rawData(index, dst, size);
			if(index<0) position_+=size;
			else position_=index+size;
			return *this;
		}

		const QByteArray& rawData(int32_t index, const char* &dst, uint32_t size) const throw(out_of_range)
		{
			if(index<0) index=position_;
			if(index+size>size_)
				throw out_of_range(index, size, size_);
			dst=data_+index;
			return *this;
		}

		template<typename T> QByteArray& getRef(int32_t index, T* &dst) throw(out_of_range)
		{
			const char* ref=0;
			getRef(index, ref, sizeof(T));
			dst=(T*)ref;
			return *this;
		}

		template<typename T> QByteArray& getRef(int32_t index, const T* &dst) throw(out_of_range)
		{
			const char* ref=0;
			getRef(index, ref, sizeof(T));
			dst=(const T*)ref;
			return *this;
		}

		void reset()
		{
			free();
			position_=0;
			size_=0;
			own_=true;
		}

		void reset(uint32_t size)
		{
			free();
			size_=size;
			if(size_>0) data_=allocate(size_);
		}

		// @Function: reset position_, return old position
		uint32_t position(uint32_t p) throw(out_of_range)
		{
			if(p>size_) throw out_of_range(position_, p, size_);
			uint32_t oldp=position_;
			position_=p;
			return oldp;
		}

		uint32_t position() const
		{return position_;}

		uint32_t size() const
		{return size_;}

		int32_t remaining() const
		{return size_-position_;}

	private:
		char* allocate(uint32_t size) const
		{
			char* data=(char*)malloc(size);
			Q_ASSERT(data!=NULL, "QByteArray::alocate error, data is null!");
			return data;
		}

		void free()
		{
			if(own_&&data_) ::free(data_);
			data_=NULL;
		}

		QByteArray& copy(const char* data, uint32_t offset, uint32_t size)
		{
			reset();
			size_=size;
			if(size_>0) {
				data_=allocate(size_);
				memcpy(data_, data+offset, size_);
			}
			return *this;
		}

	protected:
		char* data_;
		uint32_t size_;
		uint32_t position_;
		bool own_;
};

Q_END_NAMESPACE

#endif // __QBYTEARRAY_H_
