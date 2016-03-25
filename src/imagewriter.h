/********************************************************************************************
**
** Copyright (C) 2010-2015 Terry Niu (Beijing, China)
** Filename:	imagewriter.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2015/07/17
**
*********************************************************************************************/

#ifndef __IMAGEWRITER_H_
#define __IMAGEWRITER_H_

#include "../include/common/qglobal.h"
#include "../include/common/qfunc.h"

#define WRITER_OK                     (0)
#define WRITER_ERR                    (-1)

#define WRITER_INVALID_SOCKET         (-1)

#define WRITER_ERR_ALLOC              (-2)
#define WRITER_ERR_SERVER_UNAVAILABLE (-3)
#define WRITER_ERR_SOCKET_INIT        (-5)
#define WRITER_ERR_SOCKET_CONNECT     (-6)
#define WRITER_ERR_SOCKET_TIMEOUT     (-7)
#define WRITER_ERR_SOCKET_SEND        (-8)
#define WRITER_ERR_SOCKET_RECV        (-9)
#define WRITER_ERR_PROTOCOL_VERSION   (-10)
#define WRITER_ERR_DATA_LEN           (-11)
#define WRITER_ERR_STATUS             (-12)
#define WRITER_ERR_XML_LEN            (-13)
#define WRITER_ERR_BUF_TOO_SMALL      (-14)

#define WRITER_DEFAULT_SERVER         ("124.205.208.198")
#define WRITER_DEFAULT_PORT           (8085)
#define WRITER_DEFAULT_TIMEOUT        (15000)

#define WRITER_DEFAULT_SEND_SIZE      (1<<20)
#define WRITER_DEFAULT_RECV_SIZE      (1<<20)

#define WRITER_PROTOCOL_VERSION       (*(uint64_t*)"YST1.0.0")
#define WRITER_PROTOCOL_TYPE          (1)
#define WRITER_SOURCE_TYPE            (0)
#define WRITER_COMMAND_TYPE           (1)

Q_BEGIN_NAMESPACE

class ImageWriter {
	public:
		ImageWriter();

		virtual ~ImageWriter();

		/*
		 * @函数名: 初始化函数
		 * @参数01: 图片服务器的IP地址
		 * @参数02: 图片服务器的端口
		 * @参数03: 图片服务器处理超时时间
		 * @参数04: 请求数据缓存空间大小
		 * @参数05: 响应数据缓存空间大小
		 * @返回值: 成功返回0, 失败返回<0的错误码
		 */
		int32_t init(const char* ip, \
				uint16_t port, \
				int32_t timeout=WRITER_DEFAULT_TIMEOUT, \
				int32_t request_buf_size=WRITER_DEFAULT_SEND_SIZE, \
				int32_t reply_buf_size=WRITER_DEFAULT_RECV_SIZE);

		/*
		 * @函数名: 图片存储函数
		 * @参数01: 图片扩展名类型
		 * @参数02: 图片数据内容
		 * @参数03: 图片数据内容的实际长度
		 * @参数04: 图片存储响应的相关信息
		 * @参数05: 图片存储响应的相关信息空间大小
		 * @返回值: 成功返回图片信息的实际长度，失败返回<0的错误码
		 */
		int32_t write(int32_t type, \
				char* ptr_image, \
				int32_t image_len, \
				char* ptr_buf, \
				int32_t buf_size);

	private:
		Q_DISABLE_COPY(ImageWriter);

	protected:
		char		ip_[16];
		uint16_t	port_;
		int32_t		timeout_;

		char*		request_buffer_;
		int32_t		request_buffer_size_;
		int32_t 	request_buffer_len_;

		char*		reply_buffer_;
		int32_t		reply_buffer_size_;
		int32_t		reply_buffer_len_;
};

Q_END_NAMESPACE

#endif // __IMAGEWRITEER_H_
