#include "htmlparser.h"

Q_BEGIN_NAMESPACE

int32_t HtmlParser::process(uint64_t urlid, const char* url, const char* html, int32_t html_len, templInfo* tpl_info, char* out_buf, int32_t out_buf_size)
{
	if(html==NULL||html_len<=0||out_buf==NULL||out_buf_size<=0)
		return PARSE_ERR;

	char *ptr_temp=out_buf;
	char *ptr_end=out_buf+out_buf_size;
	int32_t ret=0;

	ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	if(ptr_temp+ret>=ptr_end)
		return -21;
	ptr_temp+=ret;

	ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<%s>\n", tpl_info->xml_label);
	if(ptr_temp+ret>=ptr_end)
		return -22;
	ptr_temp+=ret;

	ret=snprintf(ptr_temp, ptr_end-ptr_temp, \
			"<basic>\n"
			"<website><![CDATA[%s]]></website>\n"
			"<srcid><![CDATA[%lu]]></srcid>\n"
			"<srclink><![CDATA[%s]]></srclink>\n"
			"</basic>\n", \
			tpl_info->website, \
			urlid, \
			url);
	if(ptr_temp+ret>=ptr_end)
		return -23;
	ptr_temp+=ret;

	for(int32_t i=0; i!=tpl_info->unit_num; ++i)
	{
		ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<%s>\n", tpl_info->ptr_tui[i].unit_label);
		if(ptr_temp+ret>=ptr_end)
			return -31;
		ptr_temp+=ret;

		for(int32_t j=0; j!=tpl_info->ptr_tui[i].item_num; ++j)
		{
			ret=parseItemWithRegex(html, html_len, &(tpl_info->ptr_tui[i].ptr_tii[j]), ptr_temp, ptr_end-ptr_temp);
			if(ret<0) {
				Q_DEBUG("REGEX parse error, item_name = (%s), ret = (%d)!", \
						tpl_info->ptr_tui[i].ptr_tii[j].item_name, \
						ret);
				return ret;
			}
			if(ptr_temp+ret>=ptr_end)
				return -32;
			ptr_temp+=ret;
		}

		ret=snprintf(ptr_temp, ptr_end-ptr_temp, "</%s>\n", tpl_info->ptr_tui[i].unit_label);
		if(ptr_temp+ret>=ptr_end)
			return -33;
		ptr_temp+=ret;
	}

	ret=snprintf(ptr_temp, ptr_end-ptr_temp, "</%s>\n", tpl_info->xml_label);
	if(ptr_temp+ret>=ptr_end)
		return -34;
	ptr_temp+=ret;

	return ptrdiff_t(ptr_temp-out_buf);
}

int32_t HtmlParser::parseItemWithRegex(const char* html, int32_t html_len, itemInfo* item_info, char* ptr_buf, int32_t buf_size)
{
	if(html==NULL||item_info==NULL||ptr_buf==NULL||buf_size<=0)
		return -51;

	char* ptr_temp=ptr_buf;
	char* ptr_end=ptr_temp+buf_size;
	int32_t ret=0;

	int32_t pos[1024];
	int32_t pos_size=sizeof(pos)/sizeof(int32_t);
	int32_t rc=0;

	if(item_info->item_loop==1)
	{
		int32_t rc_total=0;
		int32_t offset=0;
		int32_t unit_size=0;
		int32_t* pPos=pos;

		do {
			rc=pcre_exec(item_info->regex_handle[0], NULL, html, html_len, offset, 0, pPos, pos_size);
			if(rc>1) {
				offset=pPos[2*rc-1];
				pPos+=rc*2;
				pos_size-=rc*2;
				rc_total+=rc;
				unit_size=rc;
			}
		} while(rc>1);

		if(rc_total>0) {
			for(int32_t i=0; i!=rc_total/unit_size; ++i) {
				ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<%s>\n", item_info->item_label);
				if(ptr_temp+ret>=ptr_end)
					return -52;
				ptr_temp+=ret;

				for(int32_t j=1; j!=unit_size; ++j)
				{
					ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<%s><![CDATA[%.*s]]></%s>\n", item_info->label_handle[j-1], \
							pos[unit_size*i*2+2*j+1]-pos[unit_size*i*2+2*j], \
							html+pos[unit_size*2*i+2*j], \
							item_info->label_handle[j-1]);
					if(ret<0||ptr_temp+ret>=ptr_end)
						return -53;
					ptr_temp+=ret;

					if(strncmp(item_info->label_handle[j-1], PARSE_IMGLINK_LABEL, PARSE_IMGLINK_LABEL_LEN)==0)
					{
						try {
							QNetworkAccessManager network_manager;
							if(network_manager.init())
								throw -54;

							network_manager.setUserAgent();
							network_manager.setCookieEnabled();
							network_manager.setRedirectionEnabled();

							std::string new_url(html+pos[unit_size*2*i+2*j], pos[unit_size*i*2+2*j+1]-pos[unit_size*i*2+2*j]);
							imageInfo img_info;
							int32_t retry_num=0;

							for(;;) {
								img_info.data_length=network_manager.doHttpGet(new_url.c_str(), \
										PARSE_DOWNLOAD_TIMEOUT, \
										img_info.data, \
										img_info.data_size);
								if(img_info.data_length>0) {
									Q_INFO("Downloading image url (%s) successful!", new_url.c_str());
									break;
								} if(++retry_num>=PARSE_DEFAULT_RETRY_NUM) {
									Q_INFO("Downloading image url (%s) failed, ret = (%d)...", new_url.c_str());
									throw -55;
								} else {
									Q_INFO("Downloading image url (%s) failed, now to retry...", new_url.c_str());
								}
							}

							ret=saveImage(new_url.c_str(), \
									img_info.data, \
									img_info.data_length, \
									img_info);
							if(ret<0) {
								Q_INFO("Save image (%s) error, errno = (%d)", new_url.c_str(), ret);
								throw -56;
							}

							ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<imgid><![CDATA[%s]]></imgid>\n", img_info.imgid.c_str());
							if(ptr_temp+ret>=ptr_end)
								throw -57;
							ptr_temp+=ret;

							ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<imgpath><![CDATA[%s]]></imgpath>\n", img_info.imgpath.c_str());
							if(ptr_temp+ret>=ptr_end)
								throw -58;
							ptr_temp+=ret;

							ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<imgsize><![CDATA[%s]]></imgsize>\n", img_info.imgsize.c_str());
							if(ptr_temp+ret>=ptr_end)
								throw -59;
							ptr_temp+=ret;
						} catch(const int32_t err) {
							return err;
						}
					}
				}

				ret=snprintf(ptr_temp, ptr_end-ptr_temp, "</%s>\n", item_info->item_label);
				if(ptr_temp+ret>=ptr_end)
					return -60;
				ptr_temp+=ret;
			}
		}
	} else {
		for(int32_t i=0; i!=item_info->regex_num; ++i) {
			rc=pcre_exec(item_info->regex_handle[i], NULL, html, html_len, 0, 0, pos, pos_size);
			if(rc<0) {
				if(rc==PCRE_ERROR_NOMATCH)
					continue;
				else
					return -61;
			} else {
				break;
			}
		}
		if(rc<=0||(rc-1!=item_info->label_num)) {
			for(int32_t j=0; j!=item_info->label_num; ++j) {
				ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<%s><![CDATA[]]></%s>\n", item_info->label_handle[j], item_info->label_handle[j]);
				if(ptr_temp+ret>=ptr_end)
					return -62;
				ptr_temp+=ret;
			}
		} else {
			for(int32_t j=0; j!=item_info->label_num; ++j)
			{
				ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<%s><![CDATA[%.*s]]></%s>\n", item_info->label_handle[j], \
						pos[2*(j+1)+1]-pos[2*(j+1)], \
						html+pos[2*(j+1)], \
						item_info->label_handle[j]);
				if(ret<0||ptr_temp+ret>=ptr_end)
					return -63;
				ptr_temp+=ret;
			}
		}
	}

	return ptrdiff_t(ptr_temp-ptr_buf);
}

int32_t HtmlParser::saveImage(const char* url, const char* ptr_data, int32_t data_len, imageInfo& img_info)
{
	QTcpClient client;
	networkReply reply;
	int ret=0;

	client.setHost("192.168.1.91", 8190);
	client.setTimeout(10000);

	client.setProtocolType(1);
	client.setSourceType(1);
	client.setCommandType(1);
	client.setOperateType(1);

	ret=client.sendRequest(ptr_data, data_len);
	if(ret<0)
		return -1;

	ret=client.getReply(&reply);
	if(ret<0)
		return ret;

	if(reply.status)
		return reply.status;

	std::string reply_str(reply.data, reply.length);

	img_info.imgid=q_substr(reply_str, "<imgid><![CDATA[", "]]></imgid>");
	if(img_info.imgid.empty())
		return -2;

	img_info.imgpath=q_substr(reply_str, "<imgpath><![CDATA[", "]]></imgpath>");
	if(img_info.imgpath.empty())
		return -3;

	img_info.imgsize=q_substr(reply_str, "<imgsize><![CDATA[", "]]></imgsize>");
	if(img_info.imgsize.empty())
		return -4;

	return PARSE_OK;
}

imageType HtmlParser::getImageType(const char* url)
{
	int32_t url_len=strlen(url);

	if(q_ends_with(url, url_len, ".jpg", 4)||q_ends_with(url, url_len, ".jpeg", 5)) {
		return TYPE_JPG;
	} else if(q_ends_with(url, url_len, ".png", 4)) {
		return TYPE_PNG;
	} else if(q_ends_with(url, url_len, ".gif", 4)) {
		return TYPE_GIF;
	} else if(q_ends_with(url, url_len, ".bmp", 4)) {
		return TYPE_BMP;
	} else if(q_ends_with(url, url_len, ".tif", 4)) {
		return TYPE_TIF;
	} else {
		return TYPE_JPG;
	}
}

const char* HtmlParser::getImageTypeName(imageType image_type)
{
	switch(image_type)
	{
		case TYPE_JPG:
			return "jpg";
		case TYPE_PNG:
			return "png";
		case TYPE_GIF:
			return "gif";
		case TYPE_BMP:
			return "bmp";
		case TYPE_TIF:
			return "tif";
		default:
			return "jpg";
	}
}

std::string HtmlParser::rewriteImageLink(std::string str)
{
	std::string new_str(str);

	std::string sub = "360buyimg.com";
	std::size_t pos = new_str.find(sub);
	if(pos != new_str.npos) {
		new_str.replace(pos+14,2,"n0");
		return new_str;
	}

	return str;
}

Q_END_NAMESPACE

