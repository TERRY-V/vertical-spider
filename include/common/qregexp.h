/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qregexp.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/04/12
**
*********************************************************************************************/

#ifndef __QREGEXP_H_
#define __QREGEXP_H_

#include <pcre.h>
#include "qglobal.h"

Q_BEGIN_NAMESPACE

enum {DEFAULT_REGEX_POS_SIZE=1<<7};

// QRegExp正则表达式类, 使用pcre正则库
class QRegExp {
	public:
		inline QRegExp() :
			reg(NULL),
			pat(NULL),
			error(NULL),
			error_offset(0)
		{}

		explicit QRegExp(const char* pattern) :
			error(NULL),
			error_offset(0)
		{
			Q_ASSERT(pattern!=NULL, "QRegExp: invalid null pattern!");
			pat=const_cast<char*>(pattern);
			reg=pcre_compile(pat, 0, (const char**)&error, &error_offset, NULL);
			Q_ASSERT(reg!=NULL, "PCRE compilation (%s) failed at offset %d : %s", pattern, error_offset, error);
		}

		virtual ~QRegExp()
		{clear();}

		bool isEmpty() const
		{return (pat==NULL)?true:false;}

		bool isValid() const
		{return (pat&&error==NULL)?true:false;}

		void clear()
		{
			pcre_free(reg);
			pat=NULL;
			error=NULL;
		}

		char* pattern() const
		{return pat;}

		void setPattern(const char* pattern)
		{
			Q_ASSERT(pattern!=NULL, "QRegExp: invalid null pattern");
			pat=const_cast<char*>(pattern);
			error=NULL;
			error_offset=0;
			reg=pcre_compile(pat, 0, (const char**)&error, &error_offset, NULL);
			Q_ASSERT(reg!=NULL, "PCRE compilation (%s) failed at offset %d : %s", pattern, error_offset, error);
		}

		bool regex_match(const char* src, int32_t srclen)
		{
			if(src==NULL||srclen<=0||!isValid())
				return false;
			int32_t pos[DEFAULT_REGEX_POS_SIZE];
			int32_t rc=pcre_exec(reg, NULL, src, srclen, 0, 0, pos, DEFAULT_REGEX_POS_SIZE);
			if(rc<=0)
				return false;
			return true;
		}

		bool regex_match(const std::string& src)
		{return regex_match(src.c_str(), src.length());}

		// @函数名: 正则表达式子串匹配函数(仅匹配一次)
		// @参数01: 待匹配字符串
		// @参数02: 待匹配字符串长度
		// @参数03: 匹配的位置数组
		// @参数04: 位置数组的最大空间
		// @返回值: 匹配成功返回匹配的子串数目, 失败返回<0的错误码
		int32_t regex_sub(const char* src, int32_t srclen, int32_t pos[], int32_t pos_vec_size)
		{
			if(src==NULL||srclen<=0||!isValid())
				return -1;
			int32_t rc=pcre_exec(reg, NULL, src, srclen, 0, 0, pos, pos_vec_size);
			if(rc<=0)
				return -2;
			return rc;
		}

		// @函数名: 正则表达式子串匹配函数(仅匹配一次)
		std::vector<std::string> regex_sub(const std::string& src)
		{
			std::vector<std::string> subs;
			int32_t pos[DEFAULT_REGEX_POS_SIZE];
			int32_t ret=regex_sub(src.c_str(), src.length(), pos, DEFAULT_REGEX_POS_SIZE);
			for(int32_t i=0; i<ret; ++i)
				subs.push_back(src.substr(pos[2*i], pos[2*i+1]-pos[2*i]));
			return subs;
		}

		// @函数名: 正则表达式子串循环匹配函数(匹配多次)
		// @参数01: 待匹配字符串
		// @参数02: 待匹配字符串长度
		// @参数03: 子串单元数
		// @参数04: 匹配的位置数组
		// @参数05: 位置数组的最大空间
		// @返回值: 匹配成功返回匹配的子串数目, 失败返回<0的错误码
		int32_t regex_sub_all(const char* src, int32_t srclen, int32_t& unit_size, int32_t pos[], int32_t pos_vec_size)
		{
			if(src==NULL||srclen<=0||!isValid())
				return -1;
			int32_t offset=0, rc=0;
			int32_t ret=0;
			unit_size=0;
			do {
				rc=pcre_exec(reg, NULL, src, srclen, offset, 0, pos, pos_vec_size);
				if(rc>1) {
					offset=pos[2*rc-1];
					pos+=rc*2;
					pos_vec_size-=rc*2;
					ret+=rc;
					unit_size=rc;
				}
			} while(rc>1);
			return ret;
		}

		// @函数名: 正则表达式子串循环匹配函数(匹配多次)
		std::vector<std::string> regex_sub_all(const std::string& src, int32_t& unit_size)
		{
			std::vector<std::string> subs;
			int32_t pos[DEFAULT_REGEX_POS_SIZE];
			int32_t ret=regex_sub_all(src.c_str(), src.length(), unit_size, pos, DEFAULT_REGEX_POS_SIZE);
			for(int32_t i=0; i<ret; ++i)
				subs.push_back(src.substr(pos[2*i], pos[2*i+1]-pos[2*i]));
			return subs;
		}

	private:
		Q_DISABLE_COPY(QRegExp);

	protected:
		pcre* reg;
		char* pat;
		char* error;
		int32_t error_offset;
};

// PCRE正则表达式集合封装库
class QRegExp2 {
	public:
		inline QRegExp2() :
			current_size(0),
			max_size(0),
			reg_handle(NULL)
		{}

		virtual ~QRegExp2()
		{
			if(reg_handle!=NULL) {
				for(int32_t i=0; i!=current_size; ++i)
					pcre_free(reg_handle[i]);
				q_delete_array<pcre*>(reg_handle);
			}
		}

		int32_t init(int32_t size)
		{
			if(size<=0)
				return -1;
			max_size=size;
			reg_handle=q_new_array<pcre*>(max_size);
			if(reg_handle==NULL)
				return -2;
			for(int32_t i=0; i!=max_size; ++i)
				reg_handle[i]=NULL;
			return 0;
		}

		int32_t compile(const char* pattern, int32_t pattern_len)
		{
			if(pattern==NULL||pattern_len==0)
				return -1;

			if(current_size==max_size)
				return -2;

			pcre *reg=NULL;
			char* error=NULL;
			int32_t error_offset=0;

			reg=pcre_compile(pattern, 0, (const char**)&error, &error_offset, NULL);
			if(reg==NULL) {
				Q_DEBUG("PCRE compilation (%s) failed at offset %d : %s", pattern, error_offset, error);
				return -3;
			}
			reg_handle[current_size++]=reg;

			return 0;
		}

		int32_t compile_from_file(const char* pFileName)
		{
			if(pFileName==NULL)
				return -1;

			FILE *fp=fopen(pFileName, "r");
			if(fp==NULL)
				return -2;

			char readBuf[BUFSIZ_1K]={0};
			int32_t ret=0;
			while(fgets(readBuf, sizeof(readBuf), fp)) {
				if(strncasecmp(readBuf, "#", 1)==0)
					continue;
				uint32_t readBufLength=cancelEnter(readBuf, strlen(readBuf));
				if(readBufLength<=0)
					continue;
				ret=compile(readBuf, readBufLength);
				if(ret<0)
					return -3;
			}

			fclose(fp);
			fp=NULL;

			return 0;
		}

		// @函数名: 正则表达式集合类匹配函数
		// @参数01: 原始字符串
		// @参数02: 原始字符串的长度
		// @参数03: 匹配的位置数组
		// @参数04: 位置数组的最大空间
		// @返回值: 失败返回-1, 成功返回匹配的子串结果数量
		int32_t exec(const char* src, int32_t src_len, int32_t pos_vec[], int32_t pos_vec_size)
		{
			if(src==NULL||src_len==0)
				return -1;

			int32_t ret=0;
			for(int32_t i=0; i!=current_size; ++i) {
				ret=pcre_exec(reg_handle[i], NULL, src, src_len, 0, 0, pos_vec, pos_vec_size);
				if(ret<0) {
					// 匹配无结果，使用新的正则进行匹配
					if(ret==PCRE_ERROR_NOMATCH)
						continue;
					else
						return -2;
				} else {
					// 匹配成功
					return ret;
				}
			}
			return 0;
		}

		// @函数名: 正则表达式集合类循环匹配函数
		// @参数01: 原始字符串
		// @参数02: 原始字符串的长度
		// @参数03: 每个单位的大小
		// @参数04: 匹配的位置数组
		// @参数05: 位置数组的最大空间
		// @返回值: 失败返回-1, 成功返回匹配的子串结果数量
		int32_t exec_looping(const char* src, int32_t src_len, int32_t& unit_size, int32_t pos_vec[], int32_t pos_vec_size)
		{
			if(src==NULL||src_len==0)
				return -1;

			int32_t offset=0, rc=0;
			int32_t ret=0;
			unit_size=0;
			// 正则循环匹配子串, 仅选择单条正则表达式匹配
			do {
				rc=pcre_exec(reg_handle[0], NULL, src, src_len, offset, 0, pos_vec, pos_vec_size);
				if(rc>1) {
					// 更改每次匹配的偏移位置
					offset=pos_vec[2*rc-1];
					pos_vec+=rc*2;
					pos_vec_size-=rc*2;
					ret+=rc;
					unit_size=rc;
				}
			} while(rc>1);
			// 返回匹配串的数量
			return ret;
		}

	private:
		uint32_t cancelEnter(char* buffer, uint32_t length)
		{
			if(NULL==buffer||length<=0)
				return 0;
			while(length>0&&(buffer[length-1]==0x20||buffer[length-1]==0x0D||buffer[length-1]==0x0A))
				length--;
			buffer[length]=0;
			return length;
		}

	private:
		Q_DISABLE_COPY(QRegExp2);

	protected:
		int32_t current_size;
		int32_t max_size;
		pcre** reg_handle;
};

// QRegExp3正则匹配信息
typedef struct __regexp_info
{
	// 正则标签名称
	std::string name;
	// 正则匹配集合
	std::vector<std::string> value;
	// 清空匹配信息
	void clear() {
		name.clear();
		value.clear();
	}
} REGEXP_INFO;

// QRegExp3正则表达式库，基于pcre正则库
class QRegExp3 {
	public:
		inline QRegExp3()
		{}

		virtual ~QRegExp3()
		{for(int32_t i=0; i!=(int32_t)(regVec.size()); ++i) pcre_free(regVec[i]);}

		// @函数名: 正则表达式编译
		// @参数01: 正则标签名称
		// @参数02: 正则表达式
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t compile(const char* name, const char* pattern)
		{
			if(name==NULL||pattern==NULL)
				return -1;

			pcre *reg=NULL;
			char* error=NULL;
			int32_t error_offset=0;

			// 编译正则表达式句柄
			reg=pcre_compile(pattern, 0, (const char**)&error, &error_offset, NULL);
			if(reg==NULL) {
				Q_DEBUG("PCRE compilation (%s) failed at offset %d : %s", pattern, error_offset, error);
				return -2;
			} else {
				regVec.push_back(reg);
				patNameVec.push_back(std::string(name));
			}

			return 0;
		}

		// @函数名: 正则表达式匹配函数
		// @参数01: 待匹配字符串
		// @参数02: 待匹配字符串的长度
		// @参数03: 匹配结果数据信息
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t exec(const char* src, int32_t src_len, std::vector<REGEXP_INFO>& regInfoVec)
		{
			if(src==NULL||src_len<=0)
				return -1;

			// 依次匹配正则表达式
			REGEXP_INFO regInfo;
			for(int32_t i=0; i!=(int32_t)regVec.size(); ++i) {
				regInfo.clear();
				regInfo.name=patNameVec.at(i);
				// 循环匹配子串
				int32_t cur_pos=0;
				int32_t rc=0;
				while(cur_pos<src_len&&(rc=pcre_exec(regVec[i], NULL, src, src_len, cur_pos, PCRE_NOTEMPTY, ovector, DEFAULT_REGEX_POS_SIZE))>=0) {
					for(int32_t j=0; j<rc; j++)
						regInfo.value.push_back(std::string(src+ovector[2*j], ovector[2*j+1]-ovector[2*j]));
					cur_pos=ovector[1];
				}
				regInfoVec.push_back(regInfo);
			}

			return 0;
		}

		void reset()
		{
			for(int32_t i=0; i!=(int32_t)regVec.size(); ++i)
				pcre_free(regVec[i]);
			regVec.clear();
			patNameVec.clear();
		}

		int32_t size() const
		{return regVec.size();}

	private:
		Q_DISABLE_COPY(QRegExp3);

	protected:
		int32_t ovector[DEFAULT_REGEX_POS_SIZE];
		std::vector<pcre*> regVec;
		std::vector<std::string> patNameVec;
};

Q_END_NAMESPACE

#endif // __QREGEXP_H_
