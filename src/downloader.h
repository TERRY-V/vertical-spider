/********************************************************************************************
**
** Copyright (C) 2010-2015 Terry Niu (Beijing, China)
** Filename:	downloader.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2015/07/17
**
*********************************************************************************************/

#ifndef __DOWNLOADER_H_
#define __DOWNLOADER_H_

#include <qglobal.h>
#include <qmd5.h>
#include <qvector.h>
#include <ServerPicker.h>

#define DOWNLOAD_OK                     (0)
#define DOWNLOAD_ERR                    (-1)
#define DOWNLOAD_ERR_ALLOC              (-2)
#define DOWNLOAD_ERR_SERVER_UNAVAILABLE (-3)
#define DOWNLOAD_ERR_SOCKET_INIT        (-5)
#define DOWNLOAD_ERR_SOCKET_CONNECT     (-6)
#define DOWNLOAD_ERR_SOCKET_TIMEOUT     (-7)
#define DOWNLOAD_ERR_SOCKET_SEND        (-8)
#define DOWNLOAD_ERR_SOCKET_RECV        (-9)
#define DOWNLOAD_ERR_PROTOCOL_VERSION   (-10)
#define DOWNLOAD_ERR_DATA_LEN           (-11)
#define DOWNLOAD_ERR_STATUS             (-12)
#define DOWNLOAD_ERR_XML_LEN            (-13)
#define DOWNLOAD_ERR_BUF_TOO_SMALL      (-14)

#define DOWNLOAD_DEFAULT_SEND_SIZE      (1<<20)
#define DOWNLOAD_DEFAULT_RECV_SIZE      (1<<20)

#define DOWNLOAD_PROTOCOL_VERSION       (*(uint64_t*)"YST1.0.0")
#define DOWNLOAD_PROTOCOL_TYPE          (1)
#define DOWNLOAD_SOURCE_TYPE            (0)
#define DOWNLOAD_COMMAND_TYPE           (1)
#define DOWNLOAD_OPERATE_TYPE           (0)

Q_BEGIN_NAMESPACE

class Downloader {
	public:
		Downloader();

		virtual ~Downloader();

		/*
		 * @函数名: 初始化函数
		 * @返回值: 成功返回0, 失败返回<0的错误码
		 */
		int32_t init();

		/*
		 * @函数名: 设置远程连接服务端地址
		 * @参数01: 远程服务器相关信息
		 * @返回值: 成功返回0, 失败返回<0的错误码
		 */
		int32_t setServerInfo(ServerInfo& server_info);

		/*
		 * @函数名: 下载处理函数
		 * @参数01: 请求XML信息内容
		 * @参数02: 请求XML信息内容的实际长度
		 * @参数03: 下载返回html数据内容
		 * @参数04: 下载返回html数据内容的最大存储空间
		 * @返回值: 成功返回下载html数据内容的实际长度, 失败返回<0的错误码
		 */
		int32_t process(const char* req_xml, int32_t req_xml_len, char* ptr_buf, int32_t buf_size);

	protected:
		Q_DISABLE_COPY(Downloader);

		struct ServerInfo	server_info_;
		char*			request_buffer_;
		int32_t			request_buffer_size_;
		char*			reply_buffer_;
		int32_t			reply_buffer_size_;
};

Q_END_NAMESPACE

#endif // __DOWNLOADER_H_
