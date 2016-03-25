#include "qnetworkaccessmanager.h"

Q_BEGIN_NAMESPACE

QNetworkAccessManager::QNetworkAccessManager() :
	curl_handle_(0),
	ptr_page_(0),
	max_page_size_(0),
	page_len_(0)
{
	network_manager_=this;
}

QNetworkAccessManager::~QNetworkAccessManager()
{
	::curl_easy_cleanup(curl_handle_);
	::curl_global_cleanup();
}

int32_t QNetworkAccessManager::init()
{
	CURLcode res=::curl_global_init(CURL_GLOBAL_ALL);
	if(res!=CURLE_OK) {
		Q_INFO("QNetworkAccessManager: curl_global_init failed, res = (%d)", res);
		return NET_ERR;
	}

	curl_handle_=::curl_easy_init();
	if(NULL==curl_handle_) {
		Q_INFO("QNetworkAccessManager:: curl_easy_init failed!");
		return NET_ERR;
	}

	return NET_OK;
}

bool QNetworkAccessManager::setReferer(const char* referer)
{
	if(referer==NULL)
		return (CURLE_OK==::curl_easy_setopt(curl_handle_, CURLOPT_AUTOREFERER, 1L));
	else
		return (CURLE_OK==::curl_easy_setopt(curl_handle_, CURLOPT_REFERER, referer));
}

bool QNetworkAccessManager::setUserAgent(const char* user_agent)
{
	if(user_agent==NULL) {
		if(CURLE_OK==::curl_easy_setopt(curl_handle_, CURLOPT_USERAGENT, NET_DEFAULT_USERAGENT))
			return true;
	} else {
		if(CURLE_OK==::curl_easy_setopt(curl_handle_, CURLOPT_USERAGENT, user_agent))
			return true;
	}
	return false;
}

bool QNetworkAccessManager::setProxy(const char* proxy)
{
	return (CURLE_OK==::curl_easy_setopt(curl_handle_, CURLOPT_PROXY, proxy));
}

bool QNetworkAccessManager::setProxy(const char* ip, int16_t port)
{
	char proxy[32]={0};
	if(q_snprintf(proxy, sizeof(proxy), "%s:%d", ip, port)<0)
		return false;
	return (CURLE_OK==::curl_easy_setopt(curl_handle_, CURLOPT_PROXY, proxy));
}

bool QNetworkAccessManager::setCookie(const char* cookie)
{
	return (CURLE_OK==::curl_easy_setopt(curl_handle_, CURLOPT_COOKIE, cookie));
}

bool QNetworkAccessManager::setCookieEnabled()
{
	if(CURLE_OK!=::curl_easy_setopt(curl_handle_, CURLOPT_COOKIEJAR, "cookie.txt"))
		return false;
	if(CURLE_OK!=::curl_easy_setopt(curl_handle_, CURLOPT_COOKIEFILE, "cookie.txt"))
		return false;
	return true;
}

bool QNetworkAccessManager::setRedirectionEnabled(int64_t times)
{
	return (CURLE_OK==::curl_easy_setopt(curl_handle_, CURLOPT_FOLLOWLOCATION, 1L) \
			&&CURLE_OK==::curl_easy_setopt(curl_handle_, CURLOPT_MAXREDIRS, times));
}

bool QNetworkAccessManager::setInterface(const char* interface)
{
	return (CURLE_OK==::curl_easy_setopt(curl_handle_, CURLOPT_INTERFACE, interface));
}

void QNetworkAccessManager::resetOption()
{
	::curl_easy_reset(curl_handle_);
	ptr_page_=0;
	max_page_size_=0;
	page_len_=0;
}

int32_t QNetworkAccessManager::doHttpHeader(const char* pUrl, int32_t iTimeOut, char* pPage, int32_t iMaxPageSize)
{
	if(pUrl==NULL||pPage==NULL||iMaxPageSize<=0||iTimeOut<=0)
		return NET_ERR;

	ptr_page_=pPage;
	max_page_size_=iMaxPageSize;
	page_len_=0;

	timeout_=iTimeOut;

	CURLcode res;
	res=::curl_easy_setopt(curl_handle_, CURLOPT_URL, pUrl);
	if(CURLE_OK!=res)
		return NET_ERR_SET_URL;

	res=::curl_easy_setopt(curl_handle_, CURLOPT_HTTPGET, 1);
	if(CURLE_OK!=res)
		return NET_ERR_SET_HTTPGET;

	res=::curl_easy_setopt(curl_handle_, CURLOPT_HEADER, 1);
	if(CURLE_OK!=res)
		return NET_ERR_SET_HEADER;

	res=::curl_easy_setopt(curl_handle_, CURLOPT_NOBODY, 1);
	if(CURLE_OK!=res)
		return NET_ERR_SET_NOBODY;

	res=::curl_easy_setopt(curl_handle_, CURLOPT_TIMEOUT_MS, timeout_);
	if(CURLE_OK!=res)
		return NET_ERR_SET_TIMEOUT_MS;

	res=::curl_easy_setopt(curl_handle_, CURLOPT_WRITEFUNCTION, QNetworkAccessManager::processFunc);
	if(CURLE_OK!=res)
		return NET_ERR_SET_WRITEFUNCTION;

	res=::curl_easy_setopt(curl_handle_, CURLOPT_WRITEDATA, network_manager_);
	if(CURLE_OK!=res)
		return NET_ERR_SET_WRITEDATA;

	res=::curl_easy_setopt(curl_handle_, CURLOPT_NOSIGNAL, 1L);
	if(CURLE_OK!=res)
		return NET_ERR_SET_NOSIGNAL;

#if defined (VERBOSE_MODE)
	res=::curl_easy_setopt(curl_handle_, CURLOPT_VERBOSE, 1L);
	if(CURLE_OK!=res)
		return NET_ERR_SET_VERBOSE;
#endif

	qsw.start();
	res=::curl_easy_perform(curl_handle_);
	qsw.stop();
	if(CURLE_OK!=res) {
		Q_INFO("curl_easy_perform() failed: (%s)", ::curl_easy_strerror(res));
		return NET_ERR_PERFORM;
	}

	int64_t code=0;
	res=::curl_easy_getinfo(curl_handle_, CURLINFO_RESPONSE_CODE, &code);
	if(CURLE_OK!=res||code!=200) {
		Q_INFO("curl_easy_getinfo() failed: code = (%ld)", code);
		return -code;
	}

	Q_INFO("Download header consumed (%d) ms......", qsw.elapsed_ms());
	return page_len_;
}

int32_t QNetworkAccessManager::doHttpPost(const char* pUrl, const char* pData, int32_t iTimeOut, char* pPage, int32_t iMaxPageSize)
{
	if(pUrl==NULL||pData==NULL||iTimeOut<=0||pPage==NULL||iMaxPageSize<=0)
		return NET_ERR;

	ptr_page_=pPage;
	max_page_size_=iMaxPageSize;
	page_len_=0;

	timeout_=iTimeOut;

	CURLcode res;
	res=::curl_easy_setopt(curl_handle_, CURLOPT_URL, pUrl);
	if(CURLE_OK!=res)
		return NET_ERR_SET_URL;

	res=::curl_easy_setopt(curl_handle_, CURLOPT_POST, 1);
	if(CURLE_OK!=res)
		return NET_ERR_SET_POST;

	res=::curl_easy_setopt(curl_handle_, CURLOPT_POSTFIELDS, pData);
	if(CURLE_OK!=res)
		return NET_ERR_SET_POSTFIELDS;

	res=::curl_easy_setopt(curl_handle_, CURLOPT_TIMEOUT_MS, timeout_);
	if(CURLE_OK!=res)
		return NET_ERR_SET_TIMEOUT_MS;

	res=::curl_easy_setopt(curl_handle_, CURLOPT_WRITEFUNCTION, QNetworkAccessManager::processFunc);
	if(CURLE_OK!=res)
		return NET_ERR_SET_WRITEFUNCTION;

	res=::curl_easy_setopt(curl_handle_, CURLOPT_WRITEDATA, network_manager_);
	if(CURLE_OK!=res)
		return NET_ERR_SET_WRITEDATA;

	res=::curl_easy_setopt(curl_handle_, CURLOPT_NOSIGNAL, 1L);
	if(CURLE_OK!=res)
		return NET_ERR_SET_NOSIGNAL;

#if defined (VERBOSE_MODE)
	res=::curl_easy_setopt(curl_handle_, CURLOPT_VERBOSE, 1L);
	if(CURLE_OK!=res)
		return NET_ERR_SET_VERBOSE;
#endif

	qsw.start();
	res=::curl_easy_perform(curl_handle_);
	qsw.stop();
	if(CURLE_OK!=res) {
#if defined (VERBOSE_MODE)
		Q_INFO("curl_easy_perform() failed: (%s)", ::curl_easy_strerror(res));
#endif
		return NET_ERR_PERFORM;
	}

	int64_t code=0;
	res=::curl_easy_getinfo(curl_handle_, CURLINFO_RESPONSE_CODE, &code);
	if(CURLE_OK!=res||code!=200) {
#if defined (VERBOSE_MODE)
		Q_INFO("curl_easy_getinfo() failed: code = (%ld)", code);
#endif
		return -code;
	}

#if defined (VERBOSE_MODE)
	Q_INFO("Download consumed (%d) ms......", qsw.elapsed_ms());
#endif
	return page_len_;
}

int32_t QNetworkAccessManager::doHttpGet(const char* pUrl, int32_t iTimeOut, char* pPage, int32_t iMaxPageSize)
{
	if(pUrl==NULL||iTimeOut<=0||pPage==NULL||iMaxPageSize<=0)
		return NET_ERR;

	ptr_page_=pPage;
	max_page_size_=iMaxPageSize;
	page_len_=0;

	timeout_=iTimeOut;

	CURLcode res;
	res=::curl_easy_setopt(curl_handle_, CURLOPT_URL, pUrl);
	if(CURLE_OK!=res)
		return NET_ERR_SET_URL;

	res=::curl_easy_setopt(curl_handle_, CURLOPT_HTTPGET, 1);
	if(CURLE_OK!=res)
		return NET_ERR_SET_HTTPGET;

	res=::curl_easy_setopt(curl_handle_, CURLOPT_TIMEOUT_MS, timeout_);
	if(CURLE_OK!=res)
		return NET_ERR_SET_TIMEOUT_MS;

	res=::curl_easy_setopt(curl_handle_, CURLOPT_WRITEFUNCTION, QNetworkAccessManager::processFunc);
	if(CURLE_OK!=res)
		return NET_ERR_SET_WRITEFUNCTION;

	res=::curl_easy_setopt(curl_handle_, CURLOPT_WRITEDATA, network_manager_);
	if(CURLE_OK!=res)
		return NET_ERR_SET_WRITEDATA;

	res=::curl_easy_setopt(curl_handle_, CURLOPT_NOSIGNAL, 1L);
	if(CURLE_OK!=res)
		return NET_ERR_SET_NOSIGNAL;

#if defined (VERBOSE_MODE)
	res=::curl_easy_setopt(curl_handle_, CURLOPT_VERBOSE, 1L);
	if(CURLE_OK!=res)
		return NET_ERR_SET_VERBOSE;
#endif

	qsw.start();
	res=::curl_easy_perform(curl_handle_);
	qsw.stop();
	if(CURLE_OK!=res) {
#if defined (VERBOSE_MODE)
		Q_INFO("curl_easy_perform() failed: (%s)", ::curl_easy_strerror(res));
#endif
		return NET_ERR_PERFORM;
	}

	int64_t code=0;
	res=::curl_easy_getinfo(curl_handle_, CURLINFO_RESPONSE_CODE, &code);
	if(CURLE_OK!=res||code!=200) {
#if defined (VERBOSE_MODE)
		Q_INFO("curl_easy_getinfo() failed: code = (%ld)", code);
#endif
		return -code;
	}

#if defined (VERBOSE_MODE)
	Q_INFO("Download consumed (%d) ms......", qsw.elapsed_ms());
#endif
	return page_len_;
}

int32_t QNetworkAccessManager::doHttpDownload(const char* pUrl, const char* pFileName, int32_t iTimeOut)
{
	if(pUrl==NULL||iTimeOut<=0||pFileName==NULL)
		return NET_ERR;

	FILE *fp=fopen(pFileName, "wb++");
	if(fp==NULL)
		return NET_ERR_FILE_OPEN;

	CURLcode res;
	res=::curl_easy_setopt(curl_handle_, CURLOPT_URL, pUrl);
	if(CURLE_OK!=res)
		return NET_ERR_SET_URL;

	res=::curl_easy_setopt(curl_handle_, CURLOPT_TIMEOUT_MS, iTimeOut);
	if(CURLE_OK!=res)
		return NET_ERR_SET_TIMEOUT_MS;

	res=::curl_easy_setopt(curl_handle_, CURLOPT_WRITEFUNCTION, QNetworkAccessManager::processDownloadFunc);
	if(CURLE_OK!=res)
		return NET_ERR_SET_WRITEFUNCTION;

	res=::curl_easy_setopt(curl_handle_, CURLOPT_WRITEDATA, (void*)fp);
	if(CURLE_OK!=res)
		return NET_ERR_SET_WRITEDATA;

	res=::curl_easy_setopt(curl_handle_, CURLOPT_NOSIGNAL, 1L);
	if(CURLE_OK!=res)
		return NET_ERR_SET_NOSIGNAL;

#if defined (VERBOSE_MODE)
	res=::curl_easy_setopt(curl_handle_, CURLOPT_VERBOSE, 1L);
	if(CURLE_OK!=res)
		return NET_ERR_SET_VERBOSE;
#endif

	qsw.start();
	res=::curl_easy_perform(curl_handle_);
	qsw.stop();
	if(CURLE_OK!=res) {
#if defined (VERBOSE_MODE)
		Q_INFO("curl_easy_perform() failed: (%s)", ::curl_easy_strerror(res));
#endif
		return NET_ERR_PERFORM;
	}

	int64_t code=0;
	res=::curl_easy_getinfo(curl_handle_, CURLINFO_RESPONSE_CODE, &code);
	if(CURLE_OK!=res||code!=200) {
#if defined (VERBOSE_MODE)
		Q_INFO("curl_easy_getinfo() failed: code = (%ld)", code);
#endif
		return -code;
	}

	fclose(fp);
	fp=NULL;

#if defined (VERBOSE_MODE)
	Q_INFO("Download consumed (%d) ms......", qsw.elapsed_ms());
#endif
	return NET_OK;
}

int32_t QNetworkAccessManager::contentCodec()
{
	char* content_type=NULL;
	int32_t codec=0;

	CURLcode res=::curl_easy_getinfo(curl_handle_, CURLINFO_CONTENT_TYPE, &content_type);
	if(CURLE_OK==res&&content_type)
		codec=codecFromContentType(content_type);

	return codec;
}

size_t QNetworkAccessManager::processFunc(void* ptr, size_t size, size_t nmemb, void* userdata)
{
	QNetworkAccessManager *ptr_this=static_cast<QNetworkAccessManager*>(userdata);
	ptr_this->qsw.stop();
	if(ptr_this->qsw.elapsed_ms()>=ptr_this->timeout_) {
#if defined (VERBOSE_MODE)
		Q_INFO("...... Page downloads timeout");
#endif
		return NET_ERR_TIMEOUT;
	}
	int32_t iSize=size*nmemb;
	if(ptr_this->page_len_+iSize>=ptr_this->max_page_size_) {
#if defined (VERBOSE_MODE)
		Q_INFO("...... Page is too large: page_len_(%d) + iSize(%d) >= max_page_size_(%d)", \
				ptr_this->page_len_, \
				iSize, \
				ptr_this->max_page_size_);
#endif
		return NET_ERR_PAGE_TOO_LARGE;
	} else {
		memcpy(ptr_this->ptr_page_+ptr_this->page_len_, ptr, iSize);
		ptr_this->page_len_+=iSize;
#if defined (VERBOSE_MODE)
		Q_INFO("...... %d bytes downloaded successful!", iSize);
#endif
	}
	return iSize;
}

size_t QNetworkAccessManager::processDownloadFunc(void* ptr, size_t size, size_t nmemb, void* userdata)
{
	FILE* pFile=(FILE*)userdata;
	size_t iSize=fwrite(ptr, size, nmemb, pFile);
#if defined (VERBOSE_MODE)
	Q_INFO("...... %ld bytes downloaded successful!", size*nmemb);
#endif
	fflush(pFile);
	return iSize;
}

int32_t QNetworkAccessManager::codecFromContentType(const char* content_type)
{
	int32_t codec=0;
	char* ptr=strcasestr((char*)content_type, (char*)"charset=");
	if(ptr==NULL)
		return codec;

	ptr+=8;
	if(strcasecmp(ptr, "gbk")==0||strcasecmp(ptr, "gb2312")==0) {
		codec=1;
	} else if(strcasecmp(ptr, "utf-8")==0) {
		codec=2;
	} else {
		codec=0;
	}

	return codec;
}

Q_END_NAMESPACE
