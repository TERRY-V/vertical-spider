/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qconfigreader.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2013/11/24
**
*********************************************************************************************/

#ifndef __QCONFIGREADER_H_
#define __QCONFIGREADER_H_

#include "qglobal.h"

Q_BEGIN_NAMESPACE

// 配置文件读取类
class QConfigReader {
	public:
		inline QConfigReader() :
			fp(NULL)
		{}

		virtual ~QConfigReader()
		{
			if(NULL!=fp) {
				fclose(fp);
				fp=NULL;
			}
		}

		int32_t init(const char* pConfigFile)
		{
			if(NULL==pConfigFile)
				return -1;
			fp=fopen(pConfigFile, "r");
			if(NULL==fp)
				return -1;
			return 0;
		}

		int32_t getFieldInt8(const char* pFieldName, int8_t& fieldValue)
		{
			char s[BUFSIZ_1K]={0};
			if(getFieldString(pFieldName, s, BUFSIZ_1K)<=0)
				return -1;
			fieldValue=(int8_t)atoi(s);
			return 0;
		}

		int32_t getFieldUint8(const char* pFieldName, uint8_t& fieldValue)
		{
			char s[BUFSIZ_1K]={0};
			if(getFieldString(pFieldName, s, BUFSIZ_1K)<=0)
				return -1;
			fieldValue=(uint8_t)atoi(s);
			return 0;
		}

		int32_t getFieldInt16(const char* pFieldName, int16_t& fieldValue)
		{
			char s[BUFSIZ_1K]={0};
			if(getFieldString(pFieldName, s, BUFSIZ_1K)<=0)
				return -1;
			fieldValue=(int16_t)atoi(s);
			return 0;
		}

		int32_t getFieldUint16(const char* pFieldName, uint16_t& fieldValue)
		{
			char s[BUFSIZ_1K]={0};
			if(getFieldString(pFieldName, s, BUFSIZ_1K)<=0)
				return -1;
			fieldValue=(uint16_t)atoi(s);
			return 0;
		}

		int32_t getFieldInt32(const char* pFieldName, int32_t& fieldValue)
		{
			char s[BUFSIZ_1K]={0};
			if(getFieldString(pFieldName, s, BUFSIZ_1K)<=0)
				return -1;
			fieldValue=atoi(s);
			return 0;
		}

		int32_t getFieldUint32(const char* pFieldName, uint32_t& fieldValue)
		{
			char s[BUFSIZ_1K]={0};
			if(getFieldString(pFieldName, s, BUFSIZ_1K)<=0)
				return -1;
			fieldValue=(uint32_t)atol(s);
			return 0;
		}

		int32_t getFieldInt64(const char* pFieldName, int64_t& fieldValue)
		{
			char s[BUFSIZ_1K]={0};
			if(getFieldString(pFieldName, s, BUFSIZ_1K)<=0)
				return -1;
			fieldValue=strtoul(s, NULL, 10);
			return 0;
		}

		int32_t getFieldUint64(const char* pFieldName, uint64_t& fieldValue)
		{
			char s[BUFSIZ_1K]={0};
			if(getFieldString(pFieldName, s, BUFSIZ_1K)<=0)
				return -1;
			fieldValue=strtoul(s, NULL, 10);
			return 0;
		}

		int32_t getFieldFloat(const char* pFieldName, float& fieldValue)
		{
			char s[BUFSIZ_1K]={0};
			if(getFieldString(pFieldName, s, BUFSIZ_1K)<=0)
				return -1;
			fieldValue=strtod(s, NULL);
			return 0;
		}

		int32_t getFieldDouble(const char* pFieldName, double& fieldValue)
		{
			char s[BUFSIZ_1K]={0};
			if(getFieldString(pFieldName, s, BUFSIZ_1K)<=0)
				return -1;
			fieldValue=strtod(s, NULL);
			return 0;
		}

		int32_t getFieldYesNo(const char* pFieldName, int32_t& fieldValue)
		{
			char s[BUFSIZ_1K]={0};
			if(getFieldString(pFieldName, s, BUFSIZ_1K)<=0)
				return -1;
			if(!q_strcasecmp(s, "yes")) {
				fieldValue=1;
				return 0;
			} else if(!q_strcasecmp(s, "no")) {
				fieldValue=0;
				return 0;
			} else {
				return -1;
			}
		}

		// @函数名: 取字段的时间值
		// @参数01: 字段名
		// @参数02: 小时值
		// @参数03: 分钟值
		// @参数04: 秒值
		// @返回值: 成功返回总秒数, 失败返回<0的错误码
		int32_t getFieldTime(const char* pFieldName, int32_t& hour, int32_t& minute, int32_t& second)
		{
			char s[BUFSIZ_1K]={0};
			if(getFieldString(pFieldName, s, BUFSIZ_1K)<=0)
				return -1;

			hour=atol(s);

			char* p=strchr(s, ':');
			if(p==NULL)
				return -1;
			p++;
			minute=atol(p);

			p=strchr(s, ':');
			if(p==NULL)
				return -1;
			p++;
			second=atol(p);

			return hour*3600+minute*60+second;
		}

		int32_t getFieldString(const char* pFieldName, char* pFieldValue, int32_t fieldValueSize)
		{
			if(NULL==pFieldName||NULL==fp||NULL==pFieldValue||fieldValueSize<=0)
				return -1;

			pFieldValue[0]=0;
			int32_t fieldNameLength=strlen(pFieldName);
			if(fieldNameLength<=0)
				return -1;

			if(fseek(fp, 0, SEEK_SET)!=0)
				return -1;

			char readBuf[BUFSIZ_1K], *p1=NULL, *p2=NULL;
			while(fgets(readBuf, BUFSIZ_1K, fp)) {
				if(q_strncasecmp(readBuf, pFieldName, fieldNameLength)!=0)
					continue;

				uint32_t readBufLength=cancelEnter(readBuf, strlen(readBuf));
				if(readBufLength<=0)
					continue;

				p1=readBuf+fieldNameLength;
				p2=readBuf+readBufLength;

				while((p1<p2)&&(*p1==' ')) p1++;
				if(*p1!='=') continue;
				p1++;
				while((p1<p2)&&(*p1==' ')) p1++;

				int32_t currentLength=p2-p1;
				if(currentLength>fieldValueSize)
					return -1;

				strncpy(pFieldValue, p1, currentLength);
				pFieldValue[currentLength]=0;
				return currentLength;
			}

			return -1;
		}

		int32_t getFieldString(const char* pFieldName, char*& pFieldValue)
		{
			if(NULL==pFieldName||NULL==fp)
				return -1;

			int32_t fieldNameLength=strlen(pFieldName);
			if(fieldNameLength<=0)
				return -1;

			if(fseek(fp, 0, SEEK_SET)!=0)
				return -1;

			char readBuf[BUFSIZ_1K], *p1=NULL, *p2=NULL;
			while(fgets(readBuf, BUFSIZ_1K, fp)) {
				if(q_strncasecmp(readBuf, pFieldName, fieldNameLength)!=0)
					continue;

				uint32_t readBufLength=cancelEnter(readBuf, strlen(readBuf));
				if(readBufLength<=0)
					continue;

				p1=readBuf+fieldNameLength;
				p2=readBuf+readBufLength;

				while((p1<p2)&&(*p1==' ')) p1++;
				if(*p1!='=') continue;
				p1++;
				while((p1<p2)&&(*p1==' ')) p1++;

				pFieldValue=q_strdup(p1);
				if(pFieldValue==NULL)
					return -1;

				return p2-p1;
			}

			return -1;
		}

		int32_t getFieldString(const char* pFieldName, std::string& fieldValue)
		{
			char s[BUFSIZ_1K]={0};
			if(getFieldString(pFieldName, s, BUFSIZ_1K)<=0)
				return -1;
			fieldValue.assign(s);
			return 0;
		}

	private:
		uint32_t cancelEnter(char* pBuffer, uint32_t bufferLength)
		{
			if(NULL==pBuffer||bufferLength<=0)
				return 0;
			while(bufferLength>0&&(pBuffer[bufferLength-1]==0x20||pBuffer[bufferLength-1]==0x0D||pBuffer[bufferLength-1]==0x0A))
				bufferLength--;
			pBuffer[bufferLength]=0;
			return bufferLength;
		}

	protected:
		FILE *fp;
};

Q_END_NAMESPACE

#endif // __QCONFIGREADER_H_
