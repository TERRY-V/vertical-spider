#include "urlprocessor.h"

Q_BEGIN_NAMESPACE

URLProcessor::URLProcessor()
{}

URLProcessor::~URLProcessor()
{}

int32_t URLProcessor::init(const char* templ_xml, int32_t templ_len)
{
	if(templ_xml==NULL||templ_len<=0)
		return -1;

	if(templManager.init(templ_xml, templ_len))
		return -2;

	return PROC_OK;
}

int32_t URLProcessor::process(int32_t method, const char* referer, const char* url, const char* playload, char* out_buf, int32_t buf_len, int32_t* wflag)
{
	if(url==NULL||out_buf==NULL||buf_len<=0)
		return -1;

	std::string html;
	templInfo* tpl_info = NULL;
	int32_t ret = 0;

	Q_INFO("Begining to process url (%s)...", url);

	ret = downloadByCurl(method, referer, url, playload, html);
	if(ret < 0) {
		Q_INFO("Oops, url (%s) downloads failed, ret = (%d)", url, ret);
		return ret;
	}

	ret = templManager.get_template(url, tpl_info);
	if(ret < 0) {
		Q_INFO("Oops, url (%s) finds no suitable template, ret = (%d)", url, ret);
		return -10;
	}

	ret = HtmlParser::process(md5.MD5Bits64((unsigned char*)url, strlen(url)), url, html.c_str(), html.length(), tpl_info, out_buf, buf_len);
	if(ret < 0) {
		Q_INFO("Oops, url (%s) fails in parsing html, ret = (%d)", url, ret);
		return -11;
	}

	*wflag = tpl_info->wflag;

	Q_INFO("Processing url (%s) successful!", url);
	return ret;
}

int32_t URLProcessor::downloadByCurl(int32_t method, const char* referer, const char* url, const char* playload, std::string& html)
{
	QNetworkAccessManager networkManager;
	char reply[1<<20] = {0};
	int32_t ret = 0;

	ret = networkManager.init();
	if(ret < 0) {
		Q_INFO("Oops, network manager init error, ret = (%d)", ret);
		return -2;
	}

	networkManager.setUserAgent();
	if(referer) networkManager.setReferer(referer);

	if(method == 1) {
		ret = networkManager.doHttpGet(url, 10000, reply, sizeof(reply));
		if(ret < 0) {
			Q_INFO("Oops, url (%s) downloads error, ret = (%d)", url, ret);
			return -3;
		}
	} else if(method == 2 && playload) {
		ret = networkManager.doHttpPost(url, playload, 10000, reply, sizeof(reply));
		if(ret < 0) {
			Q_INFO("Oops, url (%s) downloads error, ret = (%d)", url, ret);
			return -4;
		}
	} else {
		Q_INFO("Oops, url (%s) method error, method = (%d)", url, method);
		return -5;
	}

	html.append(reply, ret);
	return ret;
}

Q_END_NAMESPACE

