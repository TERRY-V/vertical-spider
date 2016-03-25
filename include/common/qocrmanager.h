/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qocrmanager.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/08/01
**
*********************************************************************************************/

#ifndef __QOCRMANAGER_H_
#define __QOCRMANAGER_H_

#include <tesseract/baseapi.h>
#include <tesseract/basedir.h>
#include <tesseract/strngs.h>

#include "qglobal.h"
#include "qfunc.h"

Q_BEGIN_NAMESPACE

// OCR图像识别类, 使用tesseract开源图像识别库
// 在linux需要安装的库, tesseract-ocr, libtesseract-dev
// 编译时需带上-ltesseract选项
// Linux命令识别方法: tesseract ***.png image -l eng
class QOcrManager {
	public:
		// OCR图像识别类构造函数和析构函数
		inline QOcrManager() :
			ocr_api(NULL)
		{
		}

		virtual ~QOcrManager()
		{
			if(ocr_api!=NULL)
				q_delete<tesseract::TessBaseAPI>(ocr_api);
		}

		// @函数名: 初始化函数
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t init()
		{
			ocr_api=q_new<tesseract::TessBaseAPI>();
			if(ocr_api==NULL)
				return -1;

			int32_t rc=ocr_api->Init(NULL, "eng", tesseract::OEM_DEFAULT);
			if(rc)
				return -2;
			return 0;
		}

		// @函数名: 设置白名单
		// @参数01: 白名单字符串
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t setWhiteList(const char* pszWhiteList=NULL)
		{
			if(pszWhiteList!=NULL)
				ocr_api->SetVariable("tessedit_char_whitelist", pszWhiteList);
			else
				ocr_api->SetVariable("tessedit_char_whitelist", "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
			return 0;
		}

		// @函数名: 设置黑名单, 忽略黑名单中的字符
		// @函数名: 黑名单字符串
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t setBlackList(const char* pszBlackList=NULL)
		{
			if(pszBlackList==NULL)
				return -1;
			ocr_api->SetVariable("tessedit_char_blacklist", pszBlackList);
			return 0;
		}

		// @函数名: OCR文本识别函数
		// @参数01: 图片的实际路径
		// @参数02: 识别的具体内容,外部空间
		// @参数03: pszOutText的最大空间
		// @返回值: 成功返回获取到的实际长度
		int32_t process_image(const char* pszImagePath, char* pszOutText, int32_t max_size=BUFSIZ_1K)
		{
			// 检查参数是否正确
			if(pszImagePath==NULL||pszOutText==NULL||max_size<=0)
				return -1;

			/*
			FILE* fin=fopen(pszImagePath, "rb");
			if(fin==NULL)
				return -2;
			fclose(fin);

			PIX *pixs;
			// 是否为tesseract支持的图片类型
			if((pixs=pixRead(pszImagePath))==NULL)
				return -3;
			pixDestroy(&pixs);
			*/

			STRING out_string;
			if(!ocr_api->ProcessPages(pszImagePath, NULL, 0, &out_string))
				return -4;

			int32_t out_string_len=q_right_trim((char*)(out_string.string()), out_string.length());
			if(out_string_len>max_size)
				return -5;

			strncpy(pszOutText, (char *)(out_string.string()), out_string_len);
			return out_string_len;
		}

		// @函数名: 获取tesseract版本
		const char* getVersion() const
		{
			return ocr_api->Version();
		}

	private:
		// Tesseract_OCR识别句柄
		tesseract::TessBaseAPI *ocr_api;
};

Q_END_NAMESPACE

#endif // __QOCRMANAGER_H_
