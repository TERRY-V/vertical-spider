/********************************************************************************************
**
** Copyright (C) 2010-2015 Terry Niu (Beijing, China)
** Filename:	parser.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2015/07/27
**
*********************************************************************************************/

#ifndef __PARSER_H_
#define __PARSER_H_

#include <qglobal.h>
#include <qdir.h>
#include <qfunc.h>
#include <qmd5.h>
#include <qnetworkaccessmanager.h>
#include <qqueue.h>
#include <md5compress.h>
#include <qlogger.h>
#include <tinyxml2.h>
#include <imagewriter.h>
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

#define PARSE_PRICE_LABEL	("goodsid")
#define PARSE_PRICE_LABEL_LEN   (7)

Q_BEGIN_NAMESPACE

using namespace tinyxml2;

/* xml info */
struct xmlInfo {
	char*		srcid;
	int32_t		srcid_len;
	char*		srclink;
	int32_t		srclink_len;
	int32_t         type;
	int32_t         extratype;
	char*		gather_data;
	int32_t		gather_data_len;

	xmlInfo() :
		srcid(NULL),
		srcid_len(0),
		srclink(NULL),
		srclink_len(0),
		type(PARSE_DEFAULT_TYPE),
		extratype(PARSE_DEFAULT_EXTRATYPE),
		gather_data(NULL),
		gather_data_len(0)
	{}
};

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
struct imageInfo {
	char*		data;
	int32_t		data_size;
	int32_t		data_length;
	char*		info;
	int32_t		info_size;
	int32_t		info_length;
	std::string	imgid;
	std::string	imgpath;

	imageInfo() :
		data(NULL),
		data_size(PARSE_DEFAULT_IMAGE_SIZE),
		data_length(0),
		info(NULL),
		info_size(PARSE_DEFAULT_INFO_SIZE),
		info_length(0),
		imgid(""),
		imgpath("")
	{}
};

/* html解析器 */
class Parser {
	public:
		inline Parser() :
			m_ptr_img_info(NULL),
			m_retry_max(PARSE_DEFAULT_RETRY_NUM),
			m_timeout(PARSE_DOWNLOAD_TIMEOUT),
			m_ptr_img_writer(NULL),
			m_ptr_logger(NULL)
		{}

		virtual ~Parser();

		/*
		 * @函数名: 初始化函数
		 * @参数01: 图片服务器的IP地址
		 * @参数02: 图片服务器的端口号
		 * @返回值: 成功返回0, 失败返回<0的错误码
		 */
		int32_t init(const char* img_server, int32_t img_port);

		/*
		 * @函数名: 解析器处理函数
		 * @参数01: 待处理XML数据
		 * @参数02: 待处理XML数据的实际长度
		 * @参数03: 抽取模板
		 * @参数04: 输出内容
		 * @参数05: 输出内容的实际长度
		 * @返回值: 成功返回输出内容的实际长度，失败返回<0的错误码
		 */
		int32_t process(const char* req_xml, int32_t req_xml_len, templInfo* tpl_info, char* out_buf, int32_t out_buf_size);

	private:
		/*
		 * @函数名: XML数据解析函数
		 * @参数01: 请求XML数据
		 * @参数02: 请求XML数据的实际长度
		 * @参数03: XMLDocument句柄
		 * @参数04: XML信息结构体
		 * @返回值: 成功返回0，失败返回<0的错误码
		 */
		int32_t parseXML(const char* req_xml, int32_t req_xml_len, XMLDocument& doc, xmlInfo& xml_info);

		/*
		 * @函数名: 正则项匹配函数
		 * @参数01: 模板项信息
		 * @参数02: XML信息结构体
		 * @参数03: 输出内容
		 * @参数04: 输出内容的实际长度
		 * @返回值: 成功返回输出内容的实际长度，失败返回<0的错误码
		 */
		int32_t parseItemWithRegex(itemInfo* item_info, xmlInfo* xml_info, char* ptr_buf, int32_t buf_size);

		/*
		 * @函数名: 获取图片类型
		 * @参数01: 图片链接地址
		 * @返回值: 图片类型
		 */
		imageType getImageType(const char* url);

		/*
		 * @函数名: 获取指定图片类型的扩展名
		 * @参数01: 图片类型
		 * @返回值: 图片类型的扩展名
		 */
		const char* getImageTypeName(imageType image_type);

		/*
		 * @函数名: 获取图片相关信息
		 * @参数01: 获取图片的id
		 * @参数02: 获取图片存储路径
		 * @返回值: 成功返回0, 失败返回<0的错误码
		 */
		int32_t getImageInfo(const std::string& info, std::string& iid, std::string& ipath);

		/*
		 * @函数名: URL重写函数
		 * @参数01: 待重写URL
		 * @参数02: 改写后的URL地址
		 */
		std::string rewriteImageLink(std::string str);

	protected:
		Q_DISABLE_COPY(Parser);

		/* image */
		imageInfo*	m_ptr_img_info;
		int32_t 	m_retry_max;
		int32_t		m_timeout;

		/* storage */
		ImageWriter*	m_ptr_img_writer;
		char*		m_img_server;
		int32_t		m_img_port;

		/* logger */
		QLogger*	m_ptr_logger;
};

Q_END_NAMESPACE

#endif // __PARSER_H_
