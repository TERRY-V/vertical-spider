/********************************************************************************************
**
** Copyright (C) 2010-2016 Terry Niu (Beijing, China)
** Filename:	urlprocessor.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2016/08/10
**
*********************************************************************************************/

#ifndef __URLPROCESSOR_H_
#define __URLPROCESSOR_H_

#include "qglobal.h"
#include "qfunc.h"
#include "qmd5.h"
#include "qtcpsocket.h"
#include "htmlparser.h"
#include "templatemanager.h"

#define PROC_OK  (0)
#define PROC_ERR (-1)

Q_BEGIN_NAMESPACE

class URLProcessor : public noncopyable {
	public:
		// @函数名: URL处理类构造函数
		explicit URLProcessor();

		// @函数名: URL处理类析构函数
		virtual ~URLProcessor();

		/*
		 * @函数名: 初始化函数
		 * @参数01: 模板XML数据
		 * @参数02: 模板XML数据的实际长度
		 * @返回值: 成功返回0, 失败返回<0的错误码
		 */
		int32_t init(const char* templ_xml, int32_t templ_len);

		/*
		 * @函数名: url处理函数(下载+抽取)
		 * @参数01: 请求方法
		 * @参数02: 请求来路页面
		 * @参数03: 请求URL地址
		 * @参数04: 负载信息
		 * @参数05: 抽取信息内容
		 * @参数06: 抽取信息内容的最大长度
		 * @参数07: 是否存储标识
		 * @返回值: 成功返回0, 失败返回<0的错误码
		 */
		int32_t process(int32_t method, const char* referer, const char* url, const char* playload, char* out_buf, int32_t buf_len, int32_t* wflag);

	private:
		/*
		 * @函数名: url处理函数
		 * @参数01: 请求方法
		 * @参数02: 请求来路页面
		 * @参数03: 请求URL地址
		 * @参数04: 负载信息
		 * @参数05: html数据内容
		 * @返回值: 成功返回0, 失败返回<0的错误码
		 */
		int32_t downloadByCurl(int32_t method, const char* referer, const char* url, const char* playload, std::string& html);

	protected:
		QMD5               md5;
		QRandom            random;
		TemplateManager    templManager;
};

Q_END_NAMESPACE

#endif // __URLPROCESSOR_H_
