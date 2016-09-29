/********************************************************************************************
**
** Copyright (C) 2010-2015 Terry Niu (Beijing, China)
** Filename:	htmlparser.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2015/07/27
**
*********************************************************************************************/

#ifndef __HTMLPARSER_H_
#define __HTMLPARSER_H_

#include <qglobal.h>
#include <qdir.h>
#include <qfunc.h>
#include <qlogger.h>
#include <qmd5.h>
#include <qnetworkaccessmanager.h>
#include <qqueue.h>
#include <qtcpsocket.h>
#include <md5compress.h>
#include <tinyxml2.h>
#include <templatemanager.h>

#define PARSE_OK                (0)
#define PARSE_ERR               (-1)

#define PARSE_DEFAULT_TYPE      (-1)
#define PARSE_DEFAULT_EXTRATYPE (-1)

#define PARSE_DEFAULT_RETRY_NUM (3)
#define PARSE_DEFAULT_PATH_SIZE (1<<8)
#define PARSE_DEFAULT_IMAGE_SIZE (1<<20)
#define PARSE_DEFAULT_INFO_SIZE (1<<10)

#define PARSE_DOWNLOAD_TIMEOUT  (15000)

#define PARSE_IMGLINK_LABEL     ("imglink")
#define PARSE_IMGLINK_LABEL_LEN (7)

#define PARSE_URL_LABEL		("url")
#define PARSE_URL_LABEL_LEN     (3)

Q_BEGIN_NAMESPACE

using namespace tinyxml2;

/* image type */
enum imageType {
	TYPE_JPG	= 0x00,
	TYPE_PNG	= 0x01,
	TYPE_BMP	= 0x02,
	TYPE_GIF	= 0x03,
	TYPE_TIF	= 0x04,
	TYPE_UNKNOWN	= 0x05
};

/* image info */
class imageInfo {
	public:
		char*		data;
		int32_t		data_size;
		int32_t		data_length;
		std::string	imgid;
		std::string	imgpath;
		std::string	imgsize;

		imageInfo() :
			data(NULL),
			data_size(PARSE_DEFAULT_IMAGE_SIZE),
			data_length(0)
		{data=q_new_array<char>(data_size); Q_CHECK_PTR(data);}

		virtual ~imageInfo()
		{q_delete_array<char>(data);}
};

/* html解析器 */
class HtmlParser {
	public:
		/*
		 * @函数名: 解析器处理函数
		 * @参数01: URL编号
		 * @参数02: URL信息
		 * @参数03: 待处理html数据
		 * @参数04: 待处理html数据的实际长度
		 * @参数05: 抽取模板
		 * @参数06: 输出内容
		 * @参数07: 输出内容的实际长度
		 * @返回值: 成功返回输出内容的实际长度，失败返回<0的错误码
		 */
		static int32_t process(uint64_t urlid, const char* url, const char* html, int32_t html_len, templInfo* tpl_info, char* out_buf, int32_t out_buf_size);

	private:
		/*
		 * @函数名: 正则项匹配函数
		 * @参数01: html信息
		 * @参数02: html实际长度
		 * @参数03: 模板项信息
		 * @参数04: 输出内容
		 * @参数05: 输出内容的实际长度
		 * @返回值: 成功返回输出内容的实际长度，失败返回<0的错误码
		 */
		static int32_t parseItemWithRegex(const char* html, int32_t html_len, itemInfo* item_info, char* ptr_buf, int32_t buf_size);

		/*
		 * @函数名: 图片存储函数
		 * @参数01: 图片url地址
		 * @参数02: 图片数据内容
		 * @参数03: 图片大小
		 * @参数04: 图片存储信息
		 * @返回值: 成功返回输出内容的实际长度，失败返回<0的错误码
		 */
		static int32_t saveImage(const char* url, const char* ptr_data, int32_t data_len, imageInfo& img_info);

		/*
		 * @函数名: 获取图片类型
		 * @参数01: 图片链接地址
		 * @返回值: 图片类型
		 */
		static imageType getImageType(const char* url);

		/*
		 * @函数名: 获取指定图片类型的扩展名
		 * @参数01: 图片类型
		 * @返回值: 图片类型的扩展名
		 */
		static const char* getImageTypeName(imageType image_type);

		/*
		 * @函数名: URL重写函数
		 * @参数01: 待重写URL
		 * @参数02: 改写后的URL地址
		 */
		static std::string rewriteImageLink(std::string str);

	protected:
		// @函数名: html解析器构造函数
		HtmlParser();
		HtmlParser(const HtmlParser&);
};

Q_END_NAMESPACE

#endif // __HTMLPARSER_H_
