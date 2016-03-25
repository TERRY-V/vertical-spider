#include "parser.h"

Q_BEGIN_NAMESPACE

Parser::~Parser()
{
	q_delete_array<char>(m_ptr_img_info->data);
	q_delete_array<char>(m_ptr_img_info->info);
	q_delete<imageInfo>(m_ptr_img_info);

	q_delete<ImageWriter>(m_ptr_img_writer);
}

int32_t Parser::init(const char* img_server, int32_t img_port)
{
	if(img_server==NULL)
		return PARSE_ERR;

	int32_t ret=0;

	m_img_server=const_cast<char*>(img_server);
	m_img_port=img_port;

	/* init imageInfo */
	m_ptr_img_info=q_new<imageInfo>();
	if(m_ptr_img_info==NULL)
		return -1;

	m_ptr_img_info->data_size=PARSE_DEFAULT_IMAGE_SIZE;
	m_ptr_img_info->data=q_new_array<char>(m_ptr_img_info->data_size);
	if(m_ptr_img_info->data==NULL)
		return -2;

	m_ptr_img_info->info_size=PARSE_DEFAULT_INFO_SIZE;
	m_ptr_img_info->info=q_new_array<char>(m_ptr_img_info->info_size);
	if(m_ptr_img_info->info==NULL)
		return -3;

	/* init ImageWriter */
	m_ptr_img_writer=q_new<ImageWriter>();
	if(m_ptr_img_writer==NULL)
		return -4;

	ret=m_ptr_img_writer->init(m_img_server, m_img_port, m_timeout);
	if(ret<0)
		return -5;

	return PARSE_OK;
}

int32_t Parser::parseXML(const char* req_xml, int32_t req_xml_len, XMLDocument& doc, xmlInfo& xml_info)
{
	if(req_xml==NULL||req_xml_len<=0)
		return PARSE_ERR;

	doc.Parse(req_xml, req_xml_len);
	if(doc.Error()) {
		Q_INFO("Parse XML error (%s)!", doc.GetErrorStr1());
		return -2;
	}

	XMLElement *rootElement=doc.RootElement();
	if(rootElement==NULL)
		return -3;

	XMLElement *baseElement=rootElement->FirstChildElement("base");
	if(baseElement==NULL)
		return -4;

	/* srcid */
	XMLElement *srcidElement=baseElement->FirstChildElement("srcid");
	if(srcidElement==NULL)
		return -5;
	xml_info.srcid=const_cast<char*>(srcidElement->GetText());
	xml_info.srcid_len=(xml_info.srcid==NULL)?0:strlen(xml_info.srcid);

	/* srclink */
	XMLElement *srclinkElement=baseElement->FirstChildElement("srclink");
	if(srclinkElement==NULL)
		return -6;
	xml_info.srclink=const_cast<char*>(srclinkElement->GetText());
	xml_info.srclink_len=(xml_info.srclink==NULL)?0:strlen(xml_info.srclink);

	/* type */
	XMLElement *typeElement = baseElement->FirstChildElement("type");
	if(typeElement==NULL)
		return -7;
	xml_info.type = atoi(typeElement->GetText());

	/* extra type */
	XMLElement *extratypeElement = baseElement->FirstChildElement("extratype");
	if(extratypeElement==NULL)
		return -8;
	xml_info.extratype = atoi(extratypeElement->GetText());

	/* gather data */
	XMLElement *gather_dataElement=baseElement->FirstChildElement("gather_data");
	if(gather_dataElement==NULL)
		return -9;
	xml_info.gather_data=const_cast<char*>(gather_dataElement->GetText());
	xml_info.gather_data_len=(xml_info.gather_data==NULL)?0:strlen(xml_info.gather_data);

	return PARSE_OK;
}

int32_t Parser::process(const char* req_xml, int32_t req_xml_len, templInfo* tpl_info, char* out_buf, int32_t out_buf_size)
{
	if(req_xml==NULL||req_xml_len<=0||out_buf==NULL||out_buf_size<=0)
		return PARSE_ERR;

	char *ptr_temp=out_buf;
	char *ptr_end=out_buf+out_buf_size;

	XMLDocument doc;
	xmlInfo xml_info;
	int32_t ret=0;

	/* parse xml */
	ret=parseXML(req_xml, req_xml_len, doc, xml_info);
	if(ret<0)
		return ret;

	ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	if(ptr_temp+ret>=ptr_end)
		return -21;
	ptr_temp+=ret;

	ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<%s>\n", tpl_info->xml_label);
	if(ptr_temp+ret>=ptr_end)
		return -22;
	ptr_temp+=ret;

	ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<base>\n");
	if(ptr_temp+ret>=ptr_end)
		return -23;
	ptr_temp+=ret;

	ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<website><![CDATA[%s]]></website>\n", tpl_info->website);
	if(ptr_temp+ret>=ptr_end)
		return -24;
	ptr_temp+=ret;

	ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<srcid><![CDATA[%.*s]]></srcid>\n", xml_info.srcid_len, xml_info.srcid);
	if(ptr_temp+ret>=ptr_end)
		return -25;
	ptr_temp+=ret;

	ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<srclink><![CDATA[%.*s]]></srclink>\n", xml_info.srclink_len, xml_info.srclink);
	if(ptr_temp+ret>=ptr_end)
		return -26;
	ptr_temp+=ret;

	ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<type><![CDATA[%d]]></type>\n", xml_info.type);
	if(ptr_temp+ret>=ptr_end)
		return -27;
	ptr_temp+=ret;

	ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<extratype><![CDATA[%d]]></extratype>\n", xml_info.extratype);
	if(ptr_temp+ret>=ptr_end)
		return -28;
	ptr_temp+=ret;

	ret=snprintf(ptr_temp, ptr_end-ptr_temp, "</base>\n");
	if(ptr_temp+ret>=ptr_end)
		return -29;
	ptr_temp+=ret;

	for(int32_t i=0; i!=tpl_info->unit_num; ++i)
	{
		ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<%s>\n", tpl_info->ptr_tui[i].unit_label);
		if(ptr_temp+ret>=ptr_end)
			return -31;
		ptr_temp+=ret;

		for(int32_t j=0; j!=tpl_info->ptr_tui[i].item_num; ++j)
		{
			ret=parseItemWithRegex(&(tpl_info->ptr_tui[i].ptr_tii[j]), &xml_info, ptr_temp, ptr_end-ptr_temp);
			if(ret<0) {
				Q_DEBUG("REGEX parse error, item_name = (%s), ret = (%d)", \
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

int32_t Parser::parseItemWithRegex(itemInfo* item_info, xmlInfo* xml_info, char* ptr_buf, int32_t buf_size)
{
	if(item_info==NULL||xml_info==NULL||ptr_buf==NULL||buf_size<=0)
		return -51;

	char* ptr_temp=ptr_buf;
	char* ptr_end=ptr_temp+buf_size;
	int32_t ret=0;

	int32_t pos[1024];
	int32_t pos_size=sizeof(pos)/sizeof(int32_t);
	int32_t rc=0;

	char denoised_buf[10<<10]={0};

	if(item_info->item_loop==1)
	{
		int32_t rc_total=0;
		int32_t offset=0;
		int32_t unit_size=0;
		int32_t* pPos=pos;

		do {
			rc=pcre_exec(item_info->regex_handle[0], NULL, xml_info->gather_data, xml_info->gather_data_len, offset, 0, pPos, pos_size);
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
#if defined (__DENOISE_ENABLED)
					ret=q_denoise(xml_info->gather_data+pos[unit_size*2*i+2*j], pos[unit_size*i*2+2*j+1]-pos[unit_size*i*2+2*j], \
							denoised_buf, \
							sizeof(denoised_buf));
					if(ret<0) {
						ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<%s><![CDATA[%.*s]]></%s>\n", item_info->label_handle[j-1], \
								pos[unit_size*i*2+2*j+1]-pos[unit_size*i*2+2*j], \
								xml_info->gather_data+pos[unit_size*2*i+2*j], \
								item_info->label_handle[j-1]);
					} else {
						if(strncmp(item_info->label_handle[j-1], PARSE_IMGLINK_LABEL, PARSE_IMGLINK_LABEL_LEN)==0) {
							ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<%s><![CDATA[%.*s]]></%s>\n", item_info->label_handle[j-1], \
									ret, \
									denoised_buf, \
									item_info->label_handle[j-1]);
						} else if(strncmp(item_info->label_handle[j-1], PARSE_URL_LABEL, PARSE_URL_LABEL_LEN)==0) {
							ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<%s><![CDATA[%.*s]]></%s>\n", item_info->label_handle[j-1], \
									ret, \
									denoised_buf, \
									item_info->label_handle[j-1]);
						} else {
							ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<%s><![CDATA[%.*s]]></%s>\n", item_info->label_handle[j-1], \
									ret, \
									denoised_buf, \
									item_info->label_handle[j-1]);
						}
					}
#else
					ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<%s><![CDATA[%.*s]]></%s>\n", item_info->label_handle[j-1], \
							pos[unit_size*i*2+2*j+1]-pos[unit_size*i*2+2*j], \
							xml_info->gather_data+pos[unit_size*2*i+2*j], \
							item_info->label_handle[j-1]);
#endif

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

#if defined (__DENOISE_ENABLED)
							std::string new_url(xml_info->gather_data+pos[unit_size*2*i+2*j], pos[unit_size*i*2+2*j+1]-pos[unit_size*i*2+2*j]);
#else
							std::string new_url(denoised_buf);
#endif
							int32_t retry_num=0;
							for(;;)
							{
								m_ptr_img_info->data_length=network_manager.doHttpGet(new_url.c_str(), \
										m_timeout, \
										m_ptr_img_info->data, \
										m_ptr_img_info->data_size);
								if(m_ptr_img_info->data_length>0) {
									Q_INFO("Downloading image url (%s) successful...", new_url.c_str());
									break;
								} if(++retry_num>=m_retry_max) {
									Q_INFO("Downloading image url (%s) failed, ret = (%d)...", new_url.c_str());
									throw -55;
								} else {
									Q_INFO("Downloading image url (%s) failed, now to retry...", new_url.c_str());
								}
							}

							/* save image */
							m_ptr_img_info->info_length=m_ptr_img_writer->write((int32_t)getImageType(new_url.c_str()), \
									m_ptr_img_info->data, \
									m_ptr_img_info->data_length, \
									m_ptr_img_info->info, \
									m_ptr_img_info->info_size);
							if(m_ptr_img_info->info_length<0) {
								Q_INFO("Write image error, errno = (%d)", m_ptr_img_info->info_length);
								throw -56;
							}

							/* image info */
							ret=getImageInfo(std::string(m_ptr_img_info->info, m_ptr_img_info->info_length), \
									m_ptr_img_info->imgid, \
									m_ptr_img_info->imgpath);
							if(ret<0) {
								Q_INFO("Fetch imgid or imgpath error (%.*s)!", m_ptr_img_info->info_length, m_ptr_img_info->info);
								throw -57;
							}

							ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<imgpath><![CDATA[%s]]></imgpath>\n", m_ptr_img_info->imgpath.c_str());
							if(ptr_temp+ret>=ptr_end)
								throw -58;
							ptr_temp+=ret;

							ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<imgid><![CDATA[%s]]></imgid>\n", m_ptr_img_info->imgid.c_str());
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
			rc=pcre_exec(item_info->regex_handle[i], NULL, xml_info->gather_data, xml_info->gather_data_len, 0, 0, pos, pos_size);
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
#if defined (__DENOISE_ENABLED)
				ret=q_denoise(xml_info->gather_data+pos[2*(j+1)], pos[2*(j+1)+1]-pos[2*(j+1)], \
						denoised_buf, \
						sizeof(denoised_buf));
				if(ret<0) {
					ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<%s><![CDATA[%.*s]]></%s>\n", item_info->label_handle[j], \
							pos[2*(j+1)+1]-pos[2*(j+1)], \
							xml_info->gather_data+pos[2*(j+1)], \
							item_info->label_handle[j]);
				} else {
					ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<%s><![CDATA[%.*s]]></%s>\n", item_info->label_handle[j], \
							ret, \
							denoised_buf, \
							item_info->label_handle[j]);
				}
#else
				ret=snprintf(ptr_temp, ptr_end-ptr_temp, "<%s><![CDATA[%.*s]]></%s>\n", item_info->label_handle[j], \
						pos[2*(j+1)+1]-pos[2*(j+1)], \
						xml_info->gather_data+pos[2*(j+1)], \
						item_info->label_handle[j]);
#endif

				if(ret<0||ptr_temp+ret>=ptr_end)
					return -63;
				ptr_temp+=ret;
			}
		}
	}

	return ptrdiff_t(ptr_temp-ptr_buf);
}

imageType Parser::getImageType(const char* url)
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

const char* Parser::getImageTypeName(imageType image_type)
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

int32_t Parser::getImageInfo(const std::string& info, std::string& iid, std::string& ipath)
{
	iid=q_substr(info, "<imgid><![CDATA[", "]]></imgid>");
	if(iid.empty())
		return -1;

	ipath=q_substr(info, "<imgpath><![CDATA[", "]]></imgpath>");
	if(ipath.empty())
		return -2;

	return PARSE_OK;
}

std::string Parser::rewriteImageLink(std::string str)
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

