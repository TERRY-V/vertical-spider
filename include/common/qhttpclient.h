/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qhttpclient.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2015/01/19
**
*********************************************************************************************/

#ifndef __QHTTPCLIENT_H_
#define __QHTTPCLIENT_H_

#include "qglobal.h"
#include "qfunc.h"
#include "qdatastream.h"
#include "qzlibmanager.h"

Q_BEGIN_NAMESPACE

// HTTP协议请求头信息
class QHttpRequestHeader {
	public:
		enum Method {
			GET=0,		// GET请求
			POST,		// POST请求
			HEAD,		// HEAD请求
			PUT,		// 上传某个资源
			DELETE,		// 删除某个资源
			PATCH		// 对某个资源做部分修改
		};

		explicit QHttpRequestHeader(const char* url=NULL, Method method=GET)
		{
			setURL(url);
			setMethod(method);
			setVersion();
			setUserAgent();
		}

		virtual ~QHttpRequestHeader()
		{}

		bool setURL(const char* url)
		{
			if(!url||!*url)
				return false;
			this->url_=url;
			if(parse_url())
				return false;
			return true;
		}

		const char* getURL() const
		{
			return url_.c_str();
		}

		bool setHost(const char* host)
		{
			if(!host||!*host)
				return false;
			this->host_=host;
			return true;
		}

		const char* getHost() const
		{
			return host_.c_str();
		}

		void setPort(int32_t port=80)
		{
			this->port_=port;
		}

		int32_t getPort() const
		{
			return port_;
		}

		bool setVersion(const char* version="HTTP/1.1")
		{
			if(strcmp(version, "HTTP/1.0")==0||strcmp(version, "HTTP/1.1")==0) {
				version_=version;
				return true;
			}
			return false;
		}

		const char* getVersion() const
		{
			return version_.c_str();
		}

		void setMethod(Method method=GET)
		{
			if(method==GET) {
				method_.assign("GET");
			} else if(method==POST) {
				method_.assign("POST");
			}
		}

		bool setReferer(const char* referer)
		{
			if(!referer||!*referer)
				return false;
			this->referer_=referer;
			return true;
		}

		const char* getReferer() const
		{
			return referer_.c_str();
		}

		bool setUserAgent(const char* user_agent=NULL)
		{
			if(user_agent==NULL) {
				this->user_agent_="Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/30.0.1599.10";
			} else {
				this->user_agent_=user_agent;
			}
			return true;
		}

		const char* getUserAgent() const
		{
			return user_agent_.c_str();
		}

		QStringBuffer& to_buffer()
		{
			buffer_.append(q_format("%s /%s %s\r\n", method_.c_str(), path_.c_str(), version_.c_str()).c_str());
			buffer_.append(q_format("Host: %s\r\n", host_.c_str()).c_str());
			buffer_.append("Connection: keep-alive\r\n");
			buffer_.append("Cache-Control: max-age=0\r\n");

			if(!referer_.empty())
				buffer_.append(q_format("Referer: %s\r\n", referer_.c_str()).c_str());
			if(!user_agent_.empty())
				buffer_.append(q_format("User-Agent: %s\r\n", user_agent_.c_str()).c_str());

			buffer_.append("Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n");
			buffer_.append("Accept-Encoding: gzip,deflate,sdch\r\n");
			buffer_.append("Accept-Language: zh-CN,zh;q=0.8\r\n");
			buffer_.append("\r\n");
			return buffer_;
		}

	private:
		int32_t parse_url()
		{
			char* pA=NULL;
			char* pB=NULL;
			char* pHost=NULL;

			// 分析URL标识符
			pA=const_cast<char*>(url_.c_str());
			if(!strncmp(pA, "http://", strlen("http://"))) {
				pA+=strlen("http://");
			} else if(!strncmp(pA, "https://", strlen("https://"))) {
				pA+=strlen("https://");
			}

			// 获取URL中的主机地址和请求路径
			pB=::strchr(pA, '/');
			if(pB) {
				host_.assign(pA, (ptrdiff_t)(pB-pA));
				if(pB+1)
					path_.assign(pB+1, strlen(pB-1));
			} else {
				host_.assign(pA, strlen(pA));
			}

			// 获取HOST中的端口号, 默认端口号80
			pHost=const_cast<char*>(host_.c_str());
			pA=::strchr(pHost, ':');
			if(pA) {
				setPort(q_stoi(pA+1, strlen(pA+1)));
				host_=host_.substr(0, (ptrdiff_t)(pA-pHost));
			} else {
				setPort(80);
			}

			return 0;
		}

	protected:
		std::string	version_;			// HTTP协议版本号
		std::string	method_;			// HTTP协议请求方法

		std::string	url_;				// HTTP请求URL
		std::string	host_;				// HTTP请求HOST
		std::string	path_;				// HTTP请求路径
		int32_t		port_;				// HTTP请求端口号

		std::string	referer_;			// HTTP协议来路页面
		std::string	user_agent_;			// HTTP协议访问浏览器信息

		QStringBuffer	buffer_;			// HTTP协议请求头信息缓存区
};

// HTTP协议返回头信息
class QHttpReplyHeader {
	public:
		QHttpReplyHeader() :
			status_(0),
			content_length_(0)
		{}

		virtual ~QHttpReplyHeader()
		{}

		int32_t getStatus() const
		{
			return status_;
		}

		int32_t getContentLength() const
		{
			return content_length_;
		}

		const char* getServer() const
		{
			return server_.c_str();
		}

		const char* getContentEncoding() const
		{
			return content_encoding_.c_str();
		}

		const char* getContentType() const
		{
			return content_type_.c_str();
		}

		const char* getConnectiopn() const
		{
			return connection_.c_str();
		}

		bool isGzip() const
		{
			if(::strstr(content_encoding_.c_str(), "gzip")||::strstr(content_encoding_.c_str(), "deflate"))
				return true;
			return false;
		}

		bool isChunked() const
		{
			if(::strstr(transfer_encoding_.c_str(), "chunked"))
				return true;
			return false;
		}

		int32_t parse_reply_header(const char* header)
		{
			if(!header||!*header)
				return -1;

			char* pHeader=const_cast<char*>(header);
			char* pTemp=NULL;
			while(*pHeader) {
				if(q_strncasecmp(pHeader, "HTTP/1.1", 8)==0||q_strncasecmp(pHeader, "HTTP/1.1", 8)==0) {
					pHeader+=8;
					while(*pHeader&&(*pHeader==' '||*pHeader=='\t'))
						pHeader++;
					status_=::atoi(pHeader);
					continue;
				} else if(q_strncasecmp(pHeader, "Content-Length:", 15)==0) {
					pHeader+=15;
					while(*pHeader&&(*pHeader==' '||*pHeader=='\t')&&*pHeader!='\r'&&*pHeader!='\n')
						pHeader++;
					content_length_=::atol(pHeader);
					continue;
				} else if(q_strncasecmp(pHeader, "Connection:", 11)==0) {
					pHeader+=11;
					while(*pHeader&&(*pHeader==' '||*pHeader=='\t')&&*pHeader!='\r'&&*pHeader!='\n')
						pHeader++;
					pTemp=pHeader;
					while(*pTemp!='\r'&&*pTemp!='\n')
						pTemp++;
					connection_.assign(pHeader, (ptrdiff_t)(pTemp-pHeader));
					pHeader=pTemp;
					continue;
				} else if(q_strncasecmp(pHeader, "Server:", 7)==0) {
					pHeader+=7;
					while(*pHeader&&(*pHeader==' '||*pHeader=='\t')&&*pHeader!='\r'&&*pHeader!='\n')
						pHeader++;
					pTemp=pHeader;
					while(*pTemp!='\r'&&*pTemp!='\n')
						pTemp++;
					server_.assign(pHeader, (ptrdiff_t)(pTemp-pHeader));
					pHeader=pTemp;
					continue;
				} else if(q_strncasecmp(pHeader, "Content-Type:", 13)==0) {
					pHeader+=13;
					while(*pHeader&&(*pHeader==' '||*pHeader=='\t')&&*pHeader!='\r'&&*pHeader!='\n')
						pHeader++;
					pTemp=pHeader;
					while(*pTemp!='\r'&&*pTemp!='\n')
						pTemp++;
					content_type_.assign(pHeader, (ptrdiff_t)(pTemp-pHeader));
					pHeader=pTemp;
					continue;
				} else if(q_strncasecmp(pHeader, "Content-Encoding:", 17)==0) {
					pHeader+=17;
					while(*pHeader&&(*pHeader==' '||*pHeader=='\t')&&*pHeader!='\r'&&*pHeader!='\n')
						pHeader++;
					pTemp=pHeader;
					while(*pTemp!='\r'&&*pTemp!='\n')
						pTemp++;
					content_encoding_.assign(pHeader, (ptrdiff_t)(pTemp-pHeader));
					pHeader=pTemp;
					continue;
				} else if(q_strncasecmp(pHeader, "Transfer-Encoding:", 18)==0) {
					pHeader+=18;
					while(*pHeader&&(*pHeader==' '||*pHeader=='\t')&&*pHeader!='\r'&&*pHeader!='\n')
						pHeader++;
					pTemp=pHeader;
					while(*pTemp!='\r'&&*pTemp!='\n')
						pTemp++;
					transfer_encoding_.assign(pHeader, (ptrdiff_t)(pTemp-pHeader));
					pHeader=pTemp;
					continue;
				}
				++pHeader;
			}

			return 0;
		}

	protected:
		int32_t		status_;			// 服务器返回状态码
		uint64_t	content_length_;		// 服务器返回内容长度

		std::string	server_;			// 服务器信息
		std::string	content_type_;			// 服务器返回数据类型
		std::string	connection_;			// 服务器连接状态
		std::string	content_encoding_;		// 服务器返回数据编码
		std::string	transfer_encoding_;		// 传输编码
};

// HTTP协议请求客户端类
class QHttpClient {
	public:
		// @函数名: HTTP请求客户端构造函数
		inline QHttpClient() :
			client_sock_(-1),
			timeout_(10000)
		{}

		// @函数名: HTTP请求客户端析构函数
		virtual ~QHttpClient()
		{}


		// @函数名: 设置超时时间(默认为10秒)
		void setTimeout(int32_t timeout=10000)
		{
			this->timeout_=timeout;
		}

		// @函数名: URL下载函数
		int32_t request(QHttpRequestHeader& request_header, char* out_buf, int32_t max_buf_size)
		{
			if(out_buf==NULL||max_buf_size<=0)
				return -1;

			int32_t ret=0;
			struct hostent* hp=NULL;
			struct sockaddr_in server_addr;

			// 获取主机IP信息
			if((hp=::gethostbyname(request_header.getHost()))==NULL) {
				Q_INFO("QHttpClient: gethostbyname error!");
				return -2;
			}

			// 客户端开始建立socket描述符
			if((client_sock_=::socket(AF_INET, SOCK_STREAM, 0))<0) {
				Q_INFO("QHttpClient: socket create error!");
				return -3;
			}

			// 客户端初始化服务端信息
			bzero(&server_addr, sizeof(server_addr));
			server_addr.sin_family=AF_INET;
			server_addr.sin_port=htons(request_header.getPort());
			server_addr.sin_addr=*((struct in_addr*)hp->h_addr);

			try {
				// 客户端发起socket连接请求
				if(::connect(client_sock_, (struct sockaddr*)(&server_addr), sizeof(struct sockaddr))<0) {
					Q_INFO("QHttpClient: connect error!");
					return -4;
				}

				// 设置超时时间
				if(q_set_overtime(client_sock_, timeout_)) {
					Q_INFO("QHttpClient: set overtime (%d) error!", timeout_);
					throw -5;
				}

#if defined (DEBUG)
				Q_INFO("request header = \n%s", request_header.to_buffer().getBuffer());
#endif

				// 发送HTTP协议头信息
				if(q_sendbuf(client_sock_, request_header.to_buffer().getBuffer(), request_header.to_buffer().length())) {
					Q_INFO("QHttpClient: send request header error!");
					throw -6;
				}

				// 接收HTTP协议响应报头
				if(recv_reply_header(client_sock_, reply_header_buffer_)<0) {
					Q_INFO("QHttpClient: recv reply header error!");
					throw -7;
				}

#if defined (DEBUG)
				Q_INFO("reply header = \n%s", reply_header_buffer_.getBuffer());
#endif

				// 解析HTTP协议响应报头信息
				ret=reply_header_.parse_reply_header(reply_header_buffer_.getBuffer());
				if(ret<0) {
					Q_INFO("QHttpClient: parse reply header error, ret = (%d)!", ret);
					throw -8;
				}

				switch(reply_header_.getStatus())
				{
					case 200:
						// 判断是否为chunked协议类型, 目前仅处理单个chunk的情况
						if(reply_header_.isChunked()) {
							// 先获取chunk头信息(通常每个chunk六字节, 四字节的十六进制字符串+"\r\n")
							uint64_t chunk_header=0;
							if(q_recvbuf(client_sock_, (char*)&chunk_header, 6)) {
								Q_INFO("QHttpClient: recv chunk header error!");
								throw -9;
							}

							// 判断chunk协议是否正确
							if(*(uint16_t*)((char*)(&chunk_header)+4)!=*(uint16_t*)"\r\n") {
								Q_INFO("QHttpClient: chunked protocol error!");
								throw -10;
							}

							// 获取chunk的实际大小
							int32_t chunk_size=str_hex_2_dec((char*)&chunk_header, 4);
#if defined (DEBUG)
							Q_INFO("chunk_size = (%d)", chunk_size);
#endif

							// 获取Chunked编码的数据内容
							if(recv_reply_content(client_sock_, reply_content_, chunk_size)) {
								Q_INFO("QHttpClient: recv reply content error!");
								throw -11;
							}
						} else {
							if(reply_header_.getContentLength()>0) {
								// 接收响应的正文内容信息
								if(recv_reply_content(client_sock_, reply_content_, reply_header_.getContentLength())) {
									Q_INFO("QHttpClient: recv reply content error!");
									throw -12;
								}
							} else {
								// 接收数据长度出错
								Q_INFO("QHttpClient: content_length = (%d)", reply_header_.getContentLength());
								throw -13;
							}
						}

						// 判断是否为gzip数据
						if(reply_header_.isGzip()) {
							ret=zlib_.gzip_uncompress((uint8_t*)reply_content_.get_data(), reply_content_.get_data_length(), (uint8_t*)out_buf, max_buf_size);
							if(ret<0) {
								Q_INFO("QHttpClient: gzip uncompress error, ret = (%d)!", ret);
								throw -14;
							}
						} else {
							if(reply_content_.get_data_length()>max_buf_size) {
								Q_INFO("QHttpClient: out buffer size (%d) is needed!", reply_content_.get_data_length());
								throw -15;
							}
							memcpy(out_buf, reply_content_.get_data(), reply_content_.get_data_length());
							ret=reply_content_.get_data_length();
						}
						break;
					default:
						// 其他错误码的处理
						throw -reply_header_.getStatus();
						break;
				}

				q_close_socket(client_sock_);
			} catch(const int32_t err) {
				ret=err;
				q_close_socket(client_sock_);
			}

			return ret;
		}

	private:
		// @函数名: 读取HTTP协议响应报头(状态行和消息报头)
		int32_t recv_reply_header(Q_SOCKET_T sock, QStringBuffer& buffer)
		{
			int32_t bytes_read=0;
			int32_t line_breaks=0;
			char ch;

			do {
				bytes_read=::recv(sock, &ch, 1, 0);
				if(bytes_read<0) {
					Q_INFO("QHttpClient: bytes_read = (%d)", bytes_read);
					return -1;
				} else if(bytes_read==0) {
					break;
				} else {
					if(ch=='\r'||ch=='\n')
						line_breaks++;
					else
						line_breaks=0;
					buffer.append(ch);
				}
			} while(line_breaks<4);

			return 0;
		}

		// @函数名: 读取HTTP协议返回的Content信息(未指定长度时, 默认不超过3M空间)
		int32_t recv_reply_content(Q_SOCKET_T sock, QDataStream& stream, int32_t content_bytes)
		{
			int32_t already_bytes=0;
			int32_t bytes_read=0;
			char buf[1<<10]={0};

			do {
				bytes_read=::recv(sock, buf, sizeof(buf), 0);
				if(bytes_read<0) {
					if(errno==EAGAIN||errno==EWOULDBLOCK) {
						continue;
					} else {
						Q_INFO("QHttpClient: socket recv error!", sock);
						return -1;
					}
				} else if(bytes_read==0) {
					break;
				} else {
					stream.set_bytes((void *)buf, bytes_read);
					already_bytes+=bytes_read;
#if defined (DEBUG)
					Q_INFO("...... %d bytes received!", bytes_read);
#endif
				}
			} while(already_bytes<content_bytes);

			return 0;
		}

		// @函数名: 十六进制转十进制
		int32_t str_hex_2_dec(const char* hex, int32_t len)
		{
			int32_t sum=0;
			int32_t t=0;
			for(int32_t i=0; i<len; ++i) {
				if(hex[i]<='9') {
					t=hex[i]-'0';
				} else {
					t=hex[i]-'a'+10;
				}
				sum=sum*16+t;
			}
			return sum;
		}

	protected:
		QStringBuffer		reply_header_buffer_;	// HTTP协议响应消息报头
		QHttpReplyHeader	reply_header_;		// HTTP协议请求头信息长度
		QDataStream		reply_content_;		// HTTP协议响应内容

		Q_SOCKET_T		client_sock_;		// 通信socket文件描述符
		int32_t			timeout_;		// 通信超时时间

		QZlibManager		zlib_;			// gzip压缩解压用
};

Q_END_NAMESPACE

#endif // __QHTTPCLIENT_H_
