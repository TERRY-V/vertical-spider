/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qnetworkaccessmanager.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/03/27
**
*********************************************************************************************/

#ifndef __QNETWORKACCESSMANAGER_H_
#define __QNETWORKACCESSMANAGER_H_

#include <curl/curl.h>
#include <curl/easy.h>
#include "qglobal.h"

Q_BEGIN_NAMESPACE

#define NET_OK			(0)
#define NET_ERR			(-1)

#define NET_ERR_SET_URL		(-2)
#define NET_ERR_SET_HTTPGET	(-3)
#define NET_ERR_SET_POST	(-4)
#define NET_ERR_SET_POSTFIELDS	(-5)
#define NET_ERR_SET_HEADER	(-6)
#define NET_ERR_SET_NOBODY	(-7)
#define NET_ERR_SET_TIMEOUT_MS	(-8)
#define NET_ERR_SET_WRITEFUNCTION (-9)
#define NET_ERR_SET_WRITEDATA	(-10)
#define NET_ERR_SET_NOSIGNAL	(-11)
#define NET_ERR_SET_VERBOSE	(-12)
#define NET_ERR_PERFORM		(-13)
#define NET_ERR_FILE_OPEN	(-14)
#define NET_ERR_TIMEOUT		(-15)
#define NET_ERR_PAGE_TOO_LARGE	(-16)

#define NET_DEFAULT_REDIRECTIONS (-1L)
#define NET_DEFAULT_TIMEOUT	(60*1000)

#define NET_DEFAULT_USERAGENT	("Mozilla/5.0 (compatible; MISE 9.0; Windows NT 6.1); Trident/5.0")

class QNetworkAccessManager {
	public:
		QNetworkAccessManager();

		virtual ~QNetworkAccessManager();

		/* init */
		int32_t init();

		/* referer */
		bool setReferer(const char* referer=NULL);

		/* user-agent */
		bool setUserAgent(const char* user_agent=NULL);

		/* proxy */
		bool setProxy(const char* proxy);

		bool setProxy(const char* ip, int16_t port);

		/* cookie */
		bool setCookie(const char* cookie);

		bool setCookieEnabled();

		/* redirection */
		bool setRedirectionEnabled(int64_t times=NET_DEFAULT_REDIRECTIONS);

		/* source interface for outgoing traffic */
		bool setInterface(const char* interface);

		/* reset */
		void resetOption();

		/* Header method */
		int32_t doHttpHeader(const char* pUrl, int32_t iTimeout, char* pPage, int32_t iMaxPageSize);

		/* Get method */
		int32_t doHttpGet(const char* pUrl, int32_t iTimeOut, char* pPage, int32_t iMaxPageSize);

		/* Post method */
		int32_t doHttpPost(const char* pUrl, const char* pData, int32_t iTimeOut, char* pPage, int32_t iMaxPageSize);

		/* Download */
		int32_t doHttpDownload(const char* pUrl, const char* pFileName, int32_t iTimeOut=NET_DEFAULT_TIMEOUT);

		/* Encoding */
		int32_t contentCodec();

	private:
		static size_t processFunc(void* ptr, size_t size, size_t nmemb, void* userdata);

		static size_t processDownloadFunc(void* ptr, size_t size, size_t nmemb, void* userdata);

		inline int32_t codecFromContentType(const char* content_type);

	protected:
		Q_DISABLE_COPY(QNetworkAccessManager);

		QNetworkAccessManager* network_manager_;
		CURL*		curl_handle_;
		char*		ptr_page_;
		int32_t		max_page_size_;
		int32_t		page_len_;
		QStopwatch	qsw;
		int32_t		timeout_;
};

Q_END_NAMESPACE

#endif // __QNETWORKACCESSMANAGER_H_
