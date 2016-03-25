/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qserialization.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Date:	2014/10/28
** Support:	http://blog.sina.com.cn/terrynotes
**
*********************************************************************************************/

#ifndef __QSERIALIZATION_H_
#define __QSERIALIZATION_H_

#include "qglobal.h"

Q_BEGIN_NAMESPACE

class QSerialization {
	public:
		static int64_t get_string_length(const char* str)
		{return NULL==str?sizeof(int32_t):(strlen(str)==0)?sizeof(int32_t):strlen(str)+sizeof(int32_t)+1;}

		static int64_t get_string_length(const std::string& str)
		{return str.empty()?sizeof(int32_t):str.length()+sizeof(int32_t)+1;}

		static int32_t get_int8(const char* data, const int64_t data_len, int64_t& pos, int8_t* value)
		{
			int32_t iret=(NULL!=value&&NULL!=data&&(data_len-pos>=(int64_t)sizeof(int8_t))&&pos>=0)?0:-1;
			if(iret==0)
				*value=data[pos++];
			return iret;
		}

		static int32_t get_int16(const char* data, const int64_t data_len, int64_t& pos, int16_t* value)
		{
			int32_t iret=(NULL!=value&&NULL!=data&&(data_len-pos>=(int64_t)sizeof(int16_t))&&pos>=0)?0:-1;
			if(iret==0) {
				int64_t tmp=pos+=(int64_t)sizeof(int16_t);
				*value=static_cast<unsigned char>(data[--tmp]);
				*value<<=8;
				*value|=static_cast<unsigned char>(data[--tmp]);
			}
			return iret;
		}

		static int32_t get_int32(const char* data, const int64_t data_len, int64_t& pos, int32_t* value)
		{
			int32_t iret=(NULL!=value&&NULL!=data&&(data_len-pos>=(int64_t)sizeof(int32_t))&&pos>=0)?0:-1;
			if(iret==0) {
				int64_t tmp=pos+=(int64_t)sizeof(int32_t);
				*value=static_cast<unsigned char>(data[--tmp]);
				*value<<=8;
				*value|=static_cast<unsigned char>(data[--tmp]);
				*value<<=8;
				*value|=static_cast<unsigned char>(data[--tmp]);
				*value<<=8;
				*value|=static_cast<unsigned char>(data[--tmp]);
			}
			return iret;
		}

		static int32_t get_int64(const char* data, const int64_t data_len, int64_t& pos, int64_t* value)
		{
			int32_t iret=(NULL!=value&&NULL!=data&&(data_len-pos>=(int64_t)sizeof(int64_t))&&pos>=0)?0:-1;
			if(iret==0) {
				int64_t tmp=pos+=(int64_t)sizeof(int64_t);
				*value=static_cast<unsigned char>(data[--tmp]);
				*value<<=8;
				*value|=static_cast<unsigned char>(data[--tmp]);
				*value<<=8;
				*value|=static_cast<unsigned char>(data[--tmp]);
				*value<<=8;
				*value|=static_cast<unsigned char>(data[--tmp]);
				*value<<=8;
				*value|=static_cast<unsigned char>(data[--tmp]);
				*value<<=8;
				*value|=static_cast<unsigned char>(data[--tmp]);
				*value<<=8;
				*value|=static_cast<unsigned char>(data[--tmp]);
				*value<<=8;
				*value|=static_cast<unsigned char>(data[--tmp]);
			}
			return iret;
		}

		static int32_t get_bytes(const char* data, const int64_t data_len, int64_t& pos, void* buf, const int64_t buf_length)
		{
			int32_t iret=(NULL!=data&&NULL!=buf&&buf_length>0&&(data_len-pos>=buf_length)&&pos>=0)?0:-1;
			if(iret==0) {
				memcpy(buf, data+pos, buf_length);
				pos+=buf_length;
			}
			return iret;
		}

		static int32_t get_string(const char* data, const int64_t data_len, int64_t& pos, const int64_t str_buf_length, \
				char* str, int64_t& real_str_buf_length)
		{
			int32_t iret=(NULL!=data&&(data_len-pos>=(int64_t)sizeof(int32_t))&&pos>=0&&NULL!=str&&str_buf_length>0)?0:-1;
			if(iret==0) {
				str[0]='\0';
				real_str_buf_length=0;
				int32_t length=0;
				iret=get_int32(data, data_len, pos, &length);
				if(iret==0) {
					if(length>0) {
						iret=length<=str_buf_length?0:-2;
						if(iret==0) {
							iret=data_len-pos>=length?0:-3;
							if(iret==0) {
								memcpy(str, data+pos, length);
								pos+=length;
								real_str_buf_length=length-1;
							}
						}
					}
				}
			}
			return iret;
		}

		static int32_t get_string(const char* data, const int64_t data_len, int64_t& pos, std::string& str)
		{
			int32_t iret=(NULL!=data&&(data_len-pos>=(int64_t)sizeof(int32_t))&&pos>=0)?0:-1;
			if(iret==0) {
				int32_t length=0;
				iret=get_int32(data, data_len, pos, &length);
				if(iret==0) {
					if(length>0) {
						iret=data_len-pos>=length?0:-2;
						if(iret==0) {
							str.assign(data+pos, length-1);
							pos+=length;
						}
					} else {
						str.clear();
					}
				}
			}
			return iret;
		}

		static int32_t set_int8(char* data, const int64_t data_len, int64_t& pos, const int8_t value)
		{
			int32_t iret=NULL!=data&&data_len-pos>=(int64_t)sizeof(int8_t)&&pos>=0?0:-1;
			if(iret==0)
				data[pos++]=value;
			return iret;
		}

		static int32_t set_int16(char* data, const int64_t data_len, int64_t& pos, const int16_t value)
		{
			int32_t iret=NULL!=data&&data_len-pos>=(int64_t)sizeof(int16_t)&&pos>=0?0:-1;
			if(iret==0) {
				data[pos++]=value&0xFF;
				data[pos++]=(value>>8)&0xFF;
			}
			return iret;
		}

		static int32_t set_int32(char* data, const int64_t data_len, int64_t& pos, const int32_t value)
		{
			int32_t iret=NULL!=data&&data_len-pos>=(int64_t)sizeof(int32_t)&&pos>=0?0:-1;
			if(iret==0) {
				data[pos++]=value&0xFF;
				data[pos++]=(value>>8)&0xFF;
				data[pos++]=(value>>16)&0xFF;
				data[pos++]=(value>>24)&0xFF;
			}
			return iret;
		}

		static int32_t set_int64(char* data, const int64_t data_len, int64_t& pos, const int64_t value)
		{
			int32_t iret=NULL!=data&&data_len-pos>=(int64_t)sizeof(int64_t)&&pos>=0?0:-1;
			if(iret==0) {
				data[pos++]=value&0xFF;
				data[pos++]=(value>>8)&0xFF;
				data[pos++]=(value>>16)&0xFF;
				data[pos++]=(value>>24)&0xFF;
				data[pos++]=(value>>32)&0xFF;
				data[pos++]=(value>>40)&0xFF;
				data[pos++]=(value>>48)&0xFF;
				data[pos++]=(value>>56)&0xFF;
			}
			return iret;
		}

		static int32_t set_string(char* data, const int64_t data_len, int64_t& pos, const std::string& str)
		{
			int32_t iret=NULL!=data&&pos<data_len&&pos>=0?0:-1;
			if(iret==0) {
				int64_t length=str.empty()?0:str.length()+1;	// include the '\0' length
				iret=data_len-pos>=(length+(int64_t)sizeof(int32_t))?0:-2;
				if(iret==0) {
					iret=set_int32(data, data_len, pos, length);
					if(iret==0) {
						if(length>0) {
							memcpy(data+pos, str.c_str(), length-1);
							pos+=length;
							data[pos-1]='\0';
						}
					}
				}
			}
			return iret;
		}

		static int32_t set_string(char* data, const int64_t data_len, int64_t& pos, const char* str)
		{
			int32_t iret=NULL!=data&&pos<data_len&&pos>=0?0:-1;
			if(iret==0) {
				int64_t length=(NULL==str)?0:strlen(str)==0?0:strlen(str)+1;
				iret=data_len-pos>=(length+(int64_t)sizeof(int32_t))?0:-1;
				if(iret==0) {
					iret=set_int32(data, data_len, pos, length);
					if(iret==0) {
						if(length>0) {
							memcpy((data+pos), str, length-1);
							pos+=length;
							data[pos-1]='\0';
						}
					}
				}
			}
			return iret;
		}

		static int32_t set_bytes(char* data, const int64_t data_len, int64_t& pos, const void* buf, const int64_t buf_length)
		{
			int32_t iret=NULL!=data&&buf_length>0&&NULL!=buf&&data_len-pos>=buf_length&&pos>=0?0:-1;
			if(iret==0) {
				memcpy(data+pos, buf, buf_length);
				pos+=buf_length;
			}
			return iret;
		}
};

Q_END_NAMESPACE

#endif // __QSERIALIZATION_H_
