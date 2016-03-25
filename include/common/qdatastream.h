/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qdatastream.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/10/28
**
*********************************************************************************************/

#ifndef __QDATASTREAM_H_
#define __QDATASTREAM_H_

#include "qglobal.h"
#include "qbuffer.h"
#include "qserialization.h"

Q_BEGIN_NAMESPACE

class QDataStream {
	public:
		inline QDataStream()
		{}
		explicit QDataStream(const int64_t length)
		{if(length>0) buffer_.expand(length);}
		virtual ~QDataStream()
		{}

		char* get_data() const
		{return buffer_.get_data();}
		int64_t get_data_length() const
		{return buffer_.get_data_length();}

		char* get_free() const
		{return buffer_.get_free();}
		int64_t get_free_length() const
		{return buffer_.get_free_length();}

		inline int32_t drain(const int64_t length)
		{return buffer_.drain(length);}
		inline int32_t pour(const int64_t length)
		{return buffer_.pour(length);}

		int32_t set_int8(const int8_t value)
		{
			int64_t pos=0;
			if(buffer_.get_free_length()<(int64_t)sizeof(int8_t))
				buffer_.expand(sizeof(int8_t));
			int32_t iret=QSerialization::set_int8(buffer_.get_free(), buffer_.get_free_length(), pos, value);
			if(iret==0)
				buffer_.pour(sizeof(int8_t));
			return iret;
		}

		int32_t set_int16(const int16_t value)
		{
			int64_t pos=0;
			if(buffer_.get_free_length()<(int64_t)sizeof(int16_t))
				buffer_.expand(sizeof(int16_t));
			int32_t iret=QSerialization::set_int16(buffer_.get_free(), buffer_.get_free_length(), pos, value);
			if(iret==0)
				buffer_.pour(sizeof(int16_t));
			return iret;
		}

		int32_t set_int32(const int32_t value)
		{
			int64_t pos=0;
			if(buffer_.get_free_length()<(int64_t)sizeof(int32_t))
				buffer_.expand(sizeof(int32_t));
			int32_t iret=QSerialization::set_int32(buffer_.get_free(), buffer_.get_free_length(), pos, value);
			if(iret==0)
				buffer_.pour(sizeof(int32_t));
			return iret;
		}

		int32_t set_int64(const int64_t value)
		{
			int64_t pos=0;
			if(buffer_.get_free_length()<(int64_t)sizeof(int64_t))
				buffer_.expand(sizeof(int64_t));
			int32_t iret=QSerialization::set_int64(buffer_.get_free(), buffer_.get_free_length(), pos, value);
			if(iret==0)
				buffer_.pour(sizeof(int64_t));
			return iret;
		}

		int32_t set_bytes(const void* data, const int64_t length)
		{
			int64_t pos=0;
			if(buffer_.get_free_length()<length)
				buffer_.expand(length);
			int32_t iret=QSerialization::set_bytes(buffer_.get_free(), buffer_.get_free_length(), pos, data, length);
			if(iret==0)
				buffer_.pour(length);
			return iret;
		}

		int32_t set_string(const char* str)
		{
			int64_t pos=0;
			int64_t length=QSerialization::get_string_length(str);
			if(buffer_.get_free_length()<length)
				buffer_.expand(length);
			int32_t iret=QSerialization::set_string(buffer_.get_free(), buffer_.get_free_length(), pos, str);
			if(iret==0)
				buffer_.pour(length);
			return iret;
		}

		int32_t set_string(const std::string& str)
		{
			int64_t pos=0;
			int64_t length=QSerialization::get_string_length(str);
			if(buffer_.get_free_length()<length+(int64_t)sizeof(int32_t))
				buffer_.expand(length);
			int32_t iret=QSerialization::set_string(buffer_.get_free(), buffer_.get_free_length(), pos, str.c_str()); 
			if(iret==0)
				buffer_.pour(length);
			return iret;
		}

		int32_t get_int8(int8_t* value)
		{
			int64_t pos=0;
			int32_t iret=QSerialization::get_int8(buffer_.get_data(), buffer_.get_data_length(), pos, value);
			if(iret==0)
				buffer_.drain(sizeof(int8_t));
			return iret;
		}

		int32_t get_int16(int16_t* value)
		{
			int64_t pos=0;
			int32_t iret=QSerialization::get_int16(buffer_.get_data(), buffer_.get_data_length(), pos, value);
			if(iret==0)
				buffer_.drain(sizeof(int16_t));
			return iret;
		}

		int32_t get_int32(int32_t* value)
		{
			int64_t pos=0;
			int32_t iret=QSerialization::get_int32(buffer_.get_data(), buffer_.get_data_length(), pos, value);
			if(iret==0)
				buffer_.drain(sizeof(int32_t));
			return iret;
		}

		int32_t get_int64(int64_t* value)
		{
			int64_t pos=0;
			int32_t iret=QSerialization::get_int64(buffer_.get_data(), buffer_.get_data_length(), pos, value);
			if(iret==0)
				buffer_.drain(sizeof(int64_t));
			return iret;
		}

		int32_t get_bytes(void* data, const int64_t length)
		{
			int64_t pos=0;
			int32_t iret=QSerialization::get_bytes(buffer_.get_data(), buffer_.get_data_length(), pos, data, length);
			if(iret==0)
				buffer_.drain(length);
			return iret;
		}

		int32_t get_string(const int64_t buf_length, char* str, int64_t& real_length)
		{
			int64_t pos=0;
			int32_t iret=QSerialization::get_string(buffer_.get_data(), buffer_.get_data_length(), pos, buf_length, str, real_length);
			if(iret==0)
				buffer_.drain(real_length);
			return iret;
		}

		int32_t get_string(std::string& str)
		{
			int64_t pos=0;
			int32_t iret=QSerialization::get_string(buffer_.get_data(), buffer_.get_data_length(), pos, str);
			if(iret==0)
				buffer_.drain(QSerialization::get_string_length(str));
			return iret;
		}

	private:
		void expand(const int64_t length)
		{buffer_.expand(length);}

		void clear()
		{buffer_.clear();}

	protected:
		Q_DISABLE_COPY(QDataStream);
		QBuffer buffer_;
};

Q_END_NAMESPACE

#endif // __QDATASTREAM_H_
