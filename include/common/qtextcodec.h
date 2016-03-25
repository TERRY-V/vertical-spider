/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qtextcodec.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Date:	2013/05/09
** Support:	http://blog.sina.com.cn/terrynotes
**
*********************************************************************************************/

#ifndef __QTEXTCODEC_H_
#define __QTEXTCODEC_H_

#include "qglobal.h"

Q_BEGIN_NAMESPACE

// 字符串编码转换类
class QTextCodec: public noncopyable
{
	public:
#ifdef WIN32
		static void utf82unicode(char* in_string, char* out_string /* WCHAR* */)
		{
			out_string[1] = ((in_string[0] & 0x0F) << 4) + ((in_string[1] >> 2) & 0x0F);
			out_string[0] = ((in_string[1] & 0x03) << 6) + (in_string[2] & 0x3F);
		}

		static void unicode2utf8(char* in_string /* WCHAR* */, char* out_string)
		{		
			out_string[0] = (0xE0 | ((in_string[1] & 0xF0) >> 4));
			out_string[1] = (0x80 | ((in_string[1] & 0x0F) << 2)) + ((in_string[0] & 0xC0) >> 6);
			out_string[2] = (0x80 | (in_string[0] & 0x3F));
		}

		static void unicode2gbk(WCHAR* in_string, char* out_string)
		{
			WideCharToMultiByte(CP_ACP, NULL, in_string, 1, out_string, sizeof(WCHAR), NULL,NULL);
		}

		static void gbk2unicode(char* in_string, WCHAR* out_string)
		{
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, in_string, 2, out_string, 1);
		}

		static void gbk2utf8(char* in_string, long in_strlen, char* out_string)
		{
			char tmpbuf[32];
			char* begpos = in_string;
			char* endpos = in_string + in_strlen;
			char* despos = out_string;
			while(begpos < endpos)
			{
				if(*begpos > 0) {
					*despos++ = *begpos++;
				} else {
					gbk2unicode(begpos, (WCHAR*)tmpbuf);
					unicode2utf8(tmpbuf, despos);
					begpos += 2;
					despos += 3;
				}
			}
			*despos = 0;
		}

		static void utf82gbk(char* in_string, long in_strlen, char* out_string)
		{
			char tmpbuf[32];
			char* begpos = in_string;
			char* endpos = in_string + in_strlen;
			char* despos = out_string;
			while(begpos < endpos)
			{
				if(*begpos > 0) {
					*despos++ = *begpos++;
				} else {
					utf82unicode(begpos, tmpbuf);
					unicode2gbk((WCHAR*)tmpbuf, despos);
					begpos += 3;
					despos += 2;
				}
			}
			*despos = 0;
		}

#else

		// @函数名: unicode->utf8编码转换函数
		// @参数01: 待转换字符串
		// @参数02: 待转换字符串长度
		// @参数03: 转换后字符串
		// @参数04: 转换后字符串的最大长度
		// @返回值: 成功返回转换后字符串的实际长度, 失败返回<0的错误码
		static int32_t unicode2utf8(char* inBuf, int32_t inBytesLen, char* outBuf, int32_t outBytesSize)
		{
			return convert((char*)"unicode", (char*)"utf8", inBuf, inBytesLen, outBuf, outBytesSize);
		}

		// @函数名: utf8->unicode编码转换函数
		// @参数01: 待转换字符串
		// @参数02: 待转换字符串长度
		// @参数03: 转换后字符串
		// @参数04: 转换后字符串的最大长度
		// @返回值: 成功返回转换后字符串的实际长度, 失败返回<0的错误码
		static int32_t utf82unicode(char* inBuf, int32_t inBytesLen, char* outBuf, int32_t outBytesSize)
		{
			return convert((char*)"utf8", (char*)"unicode", inBuf, inBytesLen, outBuf, outBytesSize);
		}

		// @函数名: unicode->gbk编码转换函数
		// @参数01: 待转换字符串
		// @参数02: 待转换字符串长度
		// @参数03: 转换后字符串
		// @参数04: 转换后字符串的最大长度
		// @返回值: 成功返回转换后字符串的实际长度, 失败返回<0的错误码
		static int32_t unicode2gbk(char* inBuf, int32_t inBytesLen, char* outBuf, int32_t outBytesSize)
		{
			return convert((char*)"unicode", (char*)"gbk", inBuf, inBytesLen, outBuf, outBytesSize);
		}

		// @函数名: gbk->unicode编码转换函数
		// @参数01: 待转换字符串
		// @参数02: 待转换字符串长度
		// @参数03: 转换后字符串
		// @参数04: 转换后字符串的最大长度
		// @返回值: 成功返回转换后字符串的实际长度, 失败返回<0的错误码
		static int32_t gbk2unicode(char* inBuf, int32_t inBytesLen, char* outBuf, int32_t outBytesSize)
		{
			return convert((char*)"gbk", (char*)"unicode", inBuf, inBytesLen, outBuf, outBytesSize);
		}

		// @函数名: utf8->gbk编码转换函数
		// @参数01: 待转换字符串
		// @参数02: 待转换字符串长度
		// @参数03: 转换后字符串
		// @参数04: 转换后字符串的最大长度
		// @返回值: 成功返回转换后字符串的实际长度, 失败返回<0的错误码
		static int32_t utf82gbk(char* inBuf, int32_t inBytesLen, char* outBuf, int32_t outBytesSize)
		{
			return convert((char*)"utf8", (char*)"gbk", inBuf, inBytesLen, outBuf, outBytesSize);
		}

		// @函数名: utf8->gbk编码转换函数
		// @参数01: 待转换字符串
		// @返回值: 转换后字符串
		static std::string utf82gbk(const std::string& ss) throw (std::runtime_error)
		{
			char* p=new(std::nothrow) char[ss.length()+1];
			if(p==NULL) throw std::runtime_error("QTextCodec: bad allocate");
			int32_t ret=utf82gbk((char*)ss.c_str(), ss.length(), (char*)p, ss.length()+1);
			if(ret<0) {
				delete [] p;
				throw std::runtime_error("QTextCodec: bad convert");
			}
			std::string result(p, ret);
			delete [] p;
			return result;
		}

		// @函数名: gbk->utf8编码转换函数
		// @参数01: 待转换字符串
		// @参数02: 待转换字符串长度
		// @参数03: 转换后字符串
		// @参数04: 转换后字符串的最大长度
		// @返回值: 成功返回转换后字符串的实际长度, 失败返回<0的错误码
		static int32_t gbk2utf8(char* inBuf, int32_t inBytesLen, char* outBuf, int32_t outBytesSize)
		{
			return convert((char*)"gbk", (char*)"utf8", inBuf, inBytesLen, outBuf, outBytesSize);
		}

		// @函数名: gbk->utf8编码转换函数
		// @参数01: 待转换字符串
		// @返回值: 转换后字符串
		static std::string gbk2utf8(const std::string& ss) throw (std::runtime_error)
		{
			char* p=new(std::nothrow) char[(ss.length()+1)*6];
			if(p==NULL) throw std::runtime_error("QTextCodec: bad allocate");
			int32_t ret=gbk2utf8((char*)ss.c_str(), ss.length(), (char*)p, (ss.length()+1)*6);
			if(ret<0) {
				delete [] p;
				throw std::runtime_error("QTextCodec: bad convert");
			}
			std::string result(p, ret);
			delete [] p;
			return result;
		}

		// @函数名: utf8->wchar_t编码转换函数
		// @参数01: 待转换字符串
		// @参数02: 待转换字符串长度
		// @参数03: 转换后字符串
		// @参数04: 转换后字符串的最大长度
		// @返回值: 成功返回转换后字符串的实际长度, 失败返回<0的错误码
		static int32_t utf82wchar_t(char* inBuf, int32_t inBytesLen, char* outBuf, int32_t outBytesSize)
		{
			return convert((char*)"utf8", (char*)"wchar_t", inBuf, inBytesLen, outBuf, outBytesSize);
		}

		// @函数名: utf8->wstring编码转换函数
		// @参数01: 待转换字符串
		// @返回值: 转换后字符串
		static std::wstring utf82wstring(const std::string& ss) throw (std::runtime_error)
		{
			char* p=new(std::nothrow) char[(ss.length()+1)*sizeof(wchar_t)];
			if(p==NULL) throw std::runtime_error("QTextCodec: bad allocate");
			int32_t ret=utf82wchar_t((char*)ss.c_str(), ss.length(), (char*)p, (ss.length()+1)*sizeof(wchar_t));
			if(ret<0) {
				delete [] p;
				throw std::runtime_error("QTextCodec: bad convert");
			}
			std::wstring result((wchar_t*)p, ret/sizeof(wchar_t));
			delete [] p;
			return result;
		}

		// @函数名: wchar_t->utf8编码转换函数
		// @参数01: 待转换字符串
		// @参数02: 待转换字符串长度
		// @参数03: 转换后字符串
		// @参数04: 转换后字符串的最大长度
		// @返回值: 成功返回转换后字符串的实际长度, 失败返回<0的错误码
		static int32_t wchar_t2utf8(char* inBuf, int32_t inBytesLen, char* outBuf, int32_t outBytesSize)
		{
			return convert((char*)"wchar_t", (char*)"utf8", inBuf, inBytesLen, outBuf, outBytesSize);
		}

		// @函数名: wstring->utf8编码转换函数
		// @参数01: 待转换字符串
		// @返回值: 转换后字符串
		static std::string wstring2utf8(const std::wstring& ss) throw (std::runtime_error)
		{
			char* p=new(std::nothrow) char[(ss.length()+1)*6];
			if(p==NULL) throw std::runtime_error("QTextCodec: bad allocate");
			int32_t ret=wchar_t2utf8((char*)ss.c_str(), ss.length()*sizeof(wchar_t), (char*)p, (ss.length()+1)*6);
			if(ret<0) {
				delete [] p;
				throw std::runtime_error("QTextCodec: bad convert");
			}
			std::string result(p, ret);
			delete [] p;
			return result;
		}

		// @函数名: gbk->wchar_t编码转换函数
		// @参数01: 待转换字符串
		// @参数02: 待转换字符串长度
		// @参数03: 转换后字符串
		// @参数04: 转换后字符串的最大长度
		// @返回值: 成功返回转换后字符串的实际长度, 失败返回<0的错误码
		static int32_t gbk2wchar_t(char* inBuf, int32_t inBytesLen, char* outBuf, int32_t outBytesSize)
		{
			return convert((char*)"gbk", (char*)"wchar_t", inBuf, inBytesLen, outBuf, outBytesSize);
		}

		// @函数名: gbk->wstring编码转换函数
		// @参数01: 待转换字符串
		// @返回值: 转换后字符串
		static std::wstring gbk2wstring(const std::string& ss) throw (std::runtime_error)
		{
			char* p=new(std::nothrow) char[(ss.length()+1)*sizeof(wchar_t)];
			if(p==NULL) throw std::runtime_error("QTextCodec: bad allocate");
			int32_t ret=gbk2wchar_t((char*)ss.c_str(), ss.length(), (char*)p, (ss.length()+1)*sizeof(wchar_t));
			if(ret<0) {
				delete [] p;
				throw std::runtime_error("QTextCodec: bad convert");
			}
			std::wstring result((wchar_t*)p, ret/sizeof(wchar_t));
			delete [] p;
			return result;
		}

		// @函数名: wchar_t->gbk编码转换函数
		// @参数01: 待转换字符串
		// @参数02: 待转换字符串长度
		// @参数03: 转换后字符串
		// @参数04: 转换后字符串的最大长度
		// @返回值: 成功返回转换后字符串的实际长度, 失败返回<0的错误码
		static int32_t wchar_t2gbk(char* inBuf, int32_t inBytesLen, char* outBuf, int32_t outBytesSize)
		{
			return convert((char*)"wchar_t", (char*)"gbk", inBuf, inBytesLen, outBuf, outBytesSize);
		}

		// @函数名: wstring->gbk编码转换函数
		// @参数01: 待转换字符串
		// @返回值: 转换后字符串
		static std::string wstring2gbk(const std::wstring& ss) throw (std::runtime_error)
		{
			char* p=new(std::nothrow) char[(ss.length()+1)*sizeof(wchar_t)];
			if(p==NULL) throw std::runtime_error("QTextCodec: bad allocate");
			int32_t ret=wchar_t2gbk((char*)ss.c_str(), ss.length()*sizeof(wchar_t), (char*)p, (ss.length()+1)*sizeof(wchar_t));
			if(ret<0) {
				delete [] p;
				throw std::runtime_error("QTextCodec: bad convert");
			}
			std::string result(p, ret);
			delete [] p;
			return result;
		}

	private:
		// @函数名: 编码转换函数
		// @参数01: 待转换字符串编码
		// @参数02: 转换后字符串编码
		// @参数03: 待转换字符串
		// @参数04: 待转换字符串长度
		// @参数05: 转换后字符串
		// @参数06: 转换后字符串的最大长度
		// @返回值: 成功返回转换后字符串的实际长度, 失败返回<0的错误码
		static int32_t convert(char* from_charset, char* to_charset, char *inBuf, size_t inLen, char *outBuf, size_t outSize)
		{
			// 检查传入参数值
			if(from_charset==NULL||to_charset==NULL||inBuf==NULL||inLen<=0||outBuf==NULL||outSize<=0)
				return -1;

			// 初始化iconv编码函数
			iconv_t conv=::iconv_open(to_charset, from_charset);
			if(conv==(iconv_t)-1)
				return -2;

			// 编码转换函数赋值
			size_t inBytesLeft=inLen;
			size_t outBytesLeft=outSize;

			// 编码转换处理
			size_t ret=::iconv(conv, (char**)&inBuf, (size_t*)&inBytesLeft, (char**)&outBuf, (size_t*)&outBytesLeft);
			if(ret==(size_t)(-1)) {
				if(errno==E2BIG) {
					Q_DEBUG("QTextCodec::convert: no sufficient room");
				} else if(errno==EILSEQ) {
					Q_DEBUG("QTextCodec::convert: invalid multibyte sequence");
				} else if(errno==EINVAL) {
					Q_DEBUG("QTextCodec::convert: imcomplete multibyte sequence");
				} else {
					Q_DEBUG("QTextCodec::convert: unknown error");
				}
				::iconv_close(conv);
				return -3;
			}

			// 关闭iconv句柄
			::iconv_close(conv);

			return outSize-outBytesLeft;
		}
#endif
};

Q_END_NAMESPACE

#endif // __QTEXTCODEC_H_
