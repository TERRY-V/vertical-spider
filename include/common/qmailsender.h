/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qmailsender.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/05/28
**
*********************************************************************************************/

#ifndef __QMAILSENDER_H_
#define __QMAILSENDER_H_

#include <curl/curl.h>
#include "qglobal.h"

Q_BEGIN_NAMESPACE

#define MULTI_PERFORM_HANG_TIMEOUT 60*1000

// C++邮件发送类，不支持QQ邮箱
class QMailSender {
	public:
		// 邮件发送类构造函数, SMTP默认端口号为25
		explicit QMailSender(const std::string& user="", const std::string& passwd="", const std::string& smtpServer="", int32_t port=25, const std::string& mailFrom="")
		{
			this->mUser=user;
			this->mPasswd=passwd;
			this->mSmtpServer=smtpServer;
			this->mPort=port;
			this->mRecipientList.clear();
			this->mMailFrom=mailFrom;
			this->mMailContent.clear();
			this->mMailContentPos=0;
		}

		// 邮件发送类析构函数
		virtual ~QMailSender()
		{}

		// 邮箱用户名
		void setUser(const std::string& user)
		{this->mUser=user;}

		std::string getUser() const
		{return this->mUser;}

		// 邮箱密码
		void setPasswd(const std::string& passwd)
		{this->mPasswd=passwd;}

		std::string getPasswd() const
		{return this->mPasswd;}

		// 设置邮箱SMTP服务器
		void setSmtpServer(const std::string& smtpServer)
		{this->mSmtpServer=smtpServer;}

		std::string getSmtpServer() const
		{return this->mSmtpServer;}
		
		// 设置邮箱SMTP端口号
		void setPort(int32_t port)
		{this->mPort=port;}

		int32_t getPort() const
		{return this->mPort;}

		// 设置邮箱发件人
		void setMailFrom(const std::string& mailFrom)
		{this->mMailFrom=mailFrom;}

		std::string getMailFrom() const
		{return mMailFrom;}
		
		// 添加收件人
		void addRecipient(const std::string& mailTo)
		{mRecipientList.push_back(mailTo);}

		void addRecipient(std::list<std::string> recipientList)
		{copy(recipientList.begin(), recipientList.end(), mRecipientList.begin());}

		// 邮件发送函数
		bool sendMail(const std::string& strSubject, const std::string& strMailBody) 
		{
			mMailContent.clear();
			mMailContentPos=0;
			ConstructHead(strSubject, strMailBody);
			bool bRet=true;
			int32_t still_running=1;
			struct timeval mp_start;
			struct curl_slist* rcpt_list=NULL;
			
			curl_global_init(CURL_GLOBAL_DEFAULT);
			CURL* curl=curl_easy_init();
			if(!curl) {
				Q_DEBUG("QMailSender: init curl failed!");
				return false;
			}
			CURLM* mcurl=curl_multi_init();
			if(!mcurl) {
				Q_DEBUG("QMailSender: init mcurl failed!");
				return false;
			}

			for(std::list<std::string>::iterator it=mRecipientList.begin(); it!=mRecipientList.end(); it++)
				rcpt_list=curl_slist_append(rcpt_list, it->c_str());
			
			if(mSmtpServer.empty()||mPort<=0)
			{
				Q_DEBUG("QMailSender: smtp server couldn't be empty, or port must be large than 0!");
				curl_slist_free_all(rcpt_list);
				curl_multi_cleanup(mcurl);
				curl_easy_cleanup(curl);
				curl_global_cleanup();
				return false;
			}
			
			std::string strUrl="smtp://"+mSmtpServer;
			strUrl+=":";
			char cPort[10];
			memset(cPort, 0, 10);
			sprintf(cPort, "%d", mPort);
			strUrl+=cPort;
			curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
			
			if(mUser!="") curl_easy_setopt(curl, CURLOPT_USERNAME, mUser.c_str());
			if(mPasswd!="") curl_easy_setopt(curl, CURLOPT_PASSWORD, mPasswd.c_str());
			
			curl_easy_setopt(curl, CURLOPT_READFUNCTION, &read_callback);
			
			if(mMailFrom.empty()) {
				Q_DEBUG("QMailSender: mail from address couldn't be empty!");
				curl_slist_free_all(rcpt_list);
				curl_multi_cleanup(mcurl);
				curl_easy_cleanup(curl);
				curl_global_cleanup();
				return false;
			}
			
			curl_easy_setopt(curl, CURLOPT_MAIL_FROM, mMailFrom.c_str());
			curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, rcpt_list);
			curl_easy_setopt(curl, CURLOPT_USE_SSL, (long) CURLUSESSL_ALL);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
			curl_easy_setopt(curl, CURLOPT_READDATA, this);
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
			curl_easy_setopt(curl, CURLOPT_SSLVERSION, 0L);
			curl_easy_setopt(curl, CURLOPT_SSL_SESSIONID_CACHE, 0L);
			curl_multi_add_handle(mcurl, curl);
			
			mp_start = tvnow();
			
			curl_multi_perform(mcurl, &still_running);
			
			while(still_running) {
				struct timeval timeout;
				int32_t rc;
				
				fd_set fdread;
				fd_set fdwrite;
				fd_set fdexcep;
				int32_t maxfd = -1;

				long curl_timeo = -1;

				FD_ZERO(&fdread);
				FD_ZERO(&fdwrite);
				FD_ZERO(&fdexcep);

				timeout.tv_sec = 1;
				timeout.tv_usec = 0;

				curl_multi_timeout(mcurl, &curl_timeo);
				if (curl_timeo>=0) {
					timeout.tv_sec=curl_timeo/1000;
					if(timeout.tv_sec>1)
						timeout.tv_sec = 1;
					else
						timeout.tv_usec=(curl_timeo%1000)*1000;
				}

				curl_multi_fdset(mcurl, &fdread, &fdwrite, &fdexcep, &maxfd);
				rc=select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);

				if(tvdiff(tvnow(), mp_start)>MULTI_PERFORM_HANG_TIMEOUT) {
					fprintf(stderr, "QMailSender: ABORTING TEST, it seems that it would have run forever.\n");
					bRet = false;
					break;
				}

				switch(rc) {
					case -1:
						Q_DEBUG("QMailSender: select error!");
						bRet = false;
						break;
					case 0:
						Q_DEBUG("QMailSender: time out, retry again!");
						curl_multi_perform(mcurl, &still_running);
						break;
					default:
						curl_multi_perform(mcurl, &still_running);
						break;
				}
			}
			
			curl_multi_remove_handle(mcurl, curl);
			curl_slist_free_all(rcpt_list);
			curl_multi_cleanup(mcurl);
			curl_easy_cleanup(curl);
			curl_global_cleanup();
			return bRet;
		}

	private:
		static size_t read_callback(void* ptr, size_t size, size_t nmemb, void* userp)
		{
			QMailSender* pSm=(QMailSender*)userp;
			if(size*nmemb<1) return 0;
			if((size_t)pSm->mMailContentPos<pSm->mMailContent.size()) {
				size_t len=pSm->mMailContent[pSm->mMailContentPos].length();
				memcpy(ptr, pSm->mMailContent[pSm->mMailContentPos].c_str(), pSm->mMailContent[pSm->mMailContentPos].length());
				pSm->mMailContentPos++;
				return len;
			}
			return 0;
		}
		
		struct timeval tvnow()
		{
			struct timeval now;
			now.tv_sec = (long)time(NULL);
			now.tv_usec = 0;
			return now;
		}

		long tvdiff(timeval newer, timeval older)
		{
			return (newer.tv_sec-older.tv_sec)*1000+(newer.tv_usec-older.tv_usec)/1000;
		}

		bool ConstructHead(const std::string& strSubject, const std::string& strContent)
		{
			mMailContent.push_back("MIME-Versioin: 1.0\n");
			std::string strTemp="To: ";
			for(std::list<std::string>::iterator it=mRecipientList.begin(); it!=mRecipientList.end();) {
				strTemp+=*it;
				it++;
				if(it!=mRecipientList.end()) strTemp+=",";
		    	}
		    	strTemp+="\n";
		    	mMailContent.push_back(strTemp);
			if(strSubject!="") {
				strTemp="Subject: ";
				strTemp+=strSubject;
				strTemp+="\n";
				mMailContent.push_back(strTemp);
			}
			mMailContent.push_back("Content-Transfer-Encoding: 8bit\n");
			mMailContent.push_back("Content-Type: text/html; \n Charset=\"UTF-8\"\n\n");
			if(strContent!="") mMailContent.push_back(strContent);
			return true;
		}

	protected:
		// 邮箱用户名
		std::string mUser;
		// 邮箱密码
		std::string mPasswd;
		// 邮箱SMTP服务器
		std::string mSmtpServer;
		// 邮箱SMTP服务器端口
		int32_t mPort;
		// 接收者邮件
		std::list<std::string> mRecipientList;
		// 发送者邮箱
		std::string mMailFrom;
		// 发送的内容队列，包括头和内容项
		std::vector<std::string> mMailContent;
		// 用于发送数据时记录发送到第几个content
		int32_t mMailContentPos;
};

Q_END_NAMESPACE

#endif // __QMAILSENDER_H_
