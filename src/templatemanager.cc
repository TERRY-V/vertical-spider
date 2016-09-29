#include "templatemanager.h"

Q_BEGIN_NAMESPACE

TemplateManager::~TemplateManager()
{
	free_template();
}

int32_t TemplateManager::init(const char* templ_xml, int32_t templ_len)
{
	return compile_template(templ_xml, templ_len);
}

int32_t TemplateManager::compile_template(const char* templ_xml, int32_t templ_len)
{
	if(templ_xml==NULL||templ_len<=0)
		return -1;

	/* parse and compile template */
	XMLDocument doc;
	doc.Parse(templ_xml, templ_len);
	if(doc.Error()) {
		Q_INFO("XML parse failed (%s)!", doc.GetErrorStr1());
		return -2;
	}

	XMLElement * rootElement=doc.FirstChildElement("doc");
	if(rootElement==NULL)
		return -3;

	XMLElement *baseElement=rootElement->FirstChildElement("base");
	if(baseElement==NULL)
		return -4;

	XMLElement *websiteElement=baseElement->FirstChildElement("website");
	if(!websiteElement || !websiteElement->GetText())
		return -5;

	XMLElement *siteurlElement=baseElement->FirstChildElement("siteurl");
	if(!siteurlElement || !siteurlElement->GetText())
		return -6;

	XMLElement *xmllabelElement=baseElement->FirstChildElement("xmllabel");
	if(!xmllabelElement || !xmllabelElement->GetText())
		return -7;

	XMLElement* templsElement=rootElement->FirstChildElement("templs");
	if(templsElement==NULL)
		return -8;

	XMLElement* templElement=templsElement->FirstChildElement("templ");
	while(templElement!=NULL)
	{
		templInfo* templ_info=q_new<templInfo>();
		if(templ_info==NULL)
			return -9;

		templ_info->ptr_tui=q_new_array<unitInfo>(templ_unit_max_);
		if(templ_info->ptr_tui==NULL)
			return -10;

		for(int32_t i=0; i<templ_unit_max_; ++i)
		{
			templ_info->ptr_tui[i].ptr_tii=q_new_array<itemInfo>(templ_item_max_);
			if(templ_info->ptr_tui[i].ptr_tii==NULL)
				return -11;

			for(int32_t j=0; j<templ_item_max_; ++j)
			{
				templ_info->ptr_tui[i].ptr_tii[j].regex_handle=q_new_array<pcre*>(templ_regex_max_);
				if(templ_info->ptr_tui[i].ptr_tii[j].regex_handle==NULL)
					return -12;

				templ_info->ptr_tui[i].ptr_tii[j].label_handle=q_new_array<char*>(templ_label_max_);
				if(templ_info->ptr_tui[i].ptr_tii[j].label_handle==NULL)
					return -13;

				for(int32_t k=0; k<templ_label_max_; ++k)
				{
					templ_info->ptr_tui[i].ptr_tii[j].label_handle[k]=q_new_array<char>(BUFSIZ_1K);
					if(templ_info->ptr_tui[i].ptr_tii[j].label_handle[k]==NULL)
						return -14;
				}
			}
		}

		/* base info */
		::strcpy(templ_info->website, websiteElement->GetText());
		::strcpy(templ_info->site_url, websiteElement->GetText());
		::strcpy(templ_info->xml_label, xmllabelElement->GetText());

		/* url pattern */
		XMLElement* urlpatternElement=templElement->FirstChildElement("urlpattern");
		if(urlpatternElement && urlpatternElement->GetText()) {
			::strcpy(templ_info->url_pattern, urlpatternElement->GetText());
		} else {
			return -15;
		}

		/* wflag */
		XMLElement* wflagElement=templElement->FirstChildElement("wflag");
		if(wflagElement && wflagElement->GetText() && strcmp(wflagElement->GetText(), "TRUE")==0)
			templ_info->wflag=true;
		else
			templ_info->wflag=false;

		/* units */
		XMLElement* unitsElement=templElement->FirstChildElement("units");
		if(unitsElement==NULL)
			return -16;

		int32_t thisUnitNum=templ_info->unit_num;

		XMLElement* unitElement=unitsElement->FirstChildElement("unit");
		while(unitElement!=NULL)
		{
			if(thisUnitNum+1>templ_unit_max_)
				return -17;

			XMLElement* unit_nameElement=unitElement->FirstChildElement("unitname");
			if(unit_nameElement&&unit_nameElement->GetText()) {
				strcpy(templ_info->ptr_tui[thisUnitNum].unit_name, unit_nameElement->GetText());
			} else {
				return -18;
			}

			XMLElement* unitlabelElement=unitElement->FirstChildElement("unitlabel");
			if(unitlabelElement&&unitlabelElement->GetText()) {
				strcpy(templ_info->ptr_tui[thisUnitNum].unit_label, unitlabelElement->GetText());
			} else {
				return -19;
			}

			/* items */
			XMLElement* itemsElement=unitElement->FirstChildElement("items");
			if(itemsElement==NULL)
				return -20;

			int32_t thisItemNum=templ_info->ptr_tui[thisUnitNum].item_num;

			XMLElement* itemElement=itemsElement->FirstChildElement("item");
			while(itemElement!=NULL)
			{
				if(thisItemNum+1>templ_item_max_)
					return -21;

				XMLElement* itemnameElement=itemElement->FirstChildElement("itemname");
				if(itemnameElement&&itemnameElement->GetText()) {
					strcpy(templ_info->ptr_tui[thisUnitNum].ptr_tii[thisItemNum].item_name, itemnameElement->GetText());
				} else {
					return -22;
				}

				XMLElement* itemloopElement=itemElement->FirstChildElement("itemloop");
				if(itemloopElement&&itemloopElement->GetText()) {
					templ_info->ptr_tui[thisUnitNum].ptr_tii[thisItemNum].item_loop=strncmp(itemloopElement->GetText(), "TRUE", 4)==0?1:0;
				} else {
					return -23;
				}

				if(templ_info->ptr_tui[thisUnitNum].ptr_tii[thisItemNum].item_loop==1)
				{
					XMLElement* itemlabelElement=itemElement->FirstChildElement("itemlabel");
					if(itemlabelElement&&itemlabelElement->GetText()) {
						strcpy(templ_info->ptr_tui[thisUnitNum].ptr_tii[thisItemNum].item_label, itemlabelElement->GetText());
					} else {
						return -24;
					}
				}

				XMLElement* item_relationElement=itemElement->FirstChildElement("itemrelation");
				if(item_relationElement&&item_relationElement->GetText()) {
					templ_info->ptr_tui[thisUnitNum].ptr_tii[thisItemNum].item_relation=(strncmp(item_relationElement->GetText(), "AND", 3)==0)?1:0;
				} else {
					return -25;
				}

				/* regexes */
				XMLElement* regexesElement=itemElement->FirstChildElement("regexes");
				if(regexesElement==NULL)
					return -26;

				int32_t thisRegexNum=templ_info->ptr_tui[thisUnitNum].ptr_tii[thisItemNum].regex_num;

				XMLElement* regexElement=regexesElement->FirstChildElement("regex");
				while(regexElement!=NULL)
				{
					if(thisRegexNum+1>templ_regex_max_)
						return -27;

					if(regexElement->GetText()) {
						char* error=NULL;
						int32_t error_offset=0;
						pcre* reg=pcre_compile(regexElement->GetText(), 0, (const char**)&error, &error_offset, NULL);
						if(reg==NULL) {
							Q_INFO("PCRE compilation (%s) failed at offset %d : %s", regexElement->GetText(), error_offset, error);
							return -28;
						}
						templ_info->ptr_tui[thisUnitNum].ptr_tii[thisItemNum].regex_handle[thisRegexNum]=reg;
					} else {
						Q_INFO("Empty PCRE pattern (%s) failed in compilation", regexElement->GetText());
						return -29;
					}

					regexElement=regexElement->NextSiblingElement();
					templ_info->ptr_tui[thisUnitNum].ptr_tii[thisItemNum].regex_num=++thisRegexNum;
				}

				/* labels */
				XMLElement* labelsElement=itemElement->FirstChildElement("labels");
				if(labelsElement==NULL)
					return -30;

				int32_t thisLabelNum=templ_info->ptr_tui[thisUnitNum].ptr_tii[thisItemNum].label_num;

				XMLElement* labelElement=labelsElement->FirstChildElement("label");
				while(labelElement!=NULL)
				{
					if(thisLabelNum+1>templ_label_max_)
						return -31;

					if(labelElement->GetText()) {
						char* pszLabel=templ_info->ptr_tui[thisUnitNum].ptr_tii[thisItemNum].label_handle[thisLabelNum];
						strcpy(pszLabel, labelElement->GetText());
					} else {
						Q_DEBUG("Empty label tag: (%s)", labelElement->GetText());
						return -32;
					}

					labelElement=labelElement->NextSiblingElement();
					templ_info->ptr_tui[thisUnitNum].ptr_tii[thisItemNum].label_num=++thisLabelNum;
				}

				itemElement=itemElement->NextSiblingElement();
				templ_info->ptr_tui[thisUnitNum].item_num=++thisItemNum;
			}

			unitElement=unitElement->NextSiblingElement();
			templ_info->unit_num=++thisUnitNum;
		}

		templ_map_.insert(std::make_pair(std::string(templ_info->url_pattern), templ_info));
		templElement=templElement->NextSiblingElement();
	}

	return TEMPL_OK;
}

int32_t TemplateManager::free_template()
{
	for(utMapIter iter=templ_map_.begin(); iter!=templ_map_.end(); ++iter)
	{
		templInfo* templ_info=iter->second;
		for(int32_t i=0; i<templ_info->unit_num; ++i)
		{
			for(int32_t j=0; j<templ_info->ptr_tui[i].item_num; ++j)
			{
				for(int32_t k=0; k<templ_info->ptr_tui[i].ptr_tii[j].regex_num; ++k)
					pcre_free(templ_info->ptr_tui[i].ptr_tii[j].regex_handle[k]);

				q_delete_array<pcre*>(templ_info->ptr_tui[i].ptr_tii[j].regex_handle);

				for(int32_t k=0; k<templ_info->ptr_tui[i].ptr_tii[j].label_num; ++k)
					q_delete_array<char>(templ_info->ptr_tui[i].ptr_tii[j].label_handle[k]);

				q_delete_array<char*>(templ_info->ptr_tui[i].ptr_tii[j].label_handle);
			}

			q_delete_array<itemInfo>(templ_info->ptr_tui[i].ptr_tii);
		}

		q_delete_array<unitInfo>(templ_info->ptr_tui);
		q_delete<templInfo>(templ_info);
	}

	templ_map_.clear();
	return TEMPL_OK;
}

int32_t TemplateManager::update_template(const char* templ_xml, int32_t templ_len)
{
	if(templ_xml==NULL||templ_len<=0)
		return TEMPL_ERR;

	templ_mutex_.lock();

	if(free_template() != TEMPL_OK) {
		templ_mutex_.unlock();
		return TEMPL_ERR;
	}

	if(compile_template(templ_xml, templ_len) != TEMPL_OK) {
		templ_mutex_.unlock();
		return TEMPL_ERR;
	}

	templ_mutex_.unlock();
	return TEMPL_OK;
}

int32_t TemplateManager::get_template(const std::string& url, templInfo*& templ_info)
{
	for(utMapIter iter=templ_map_.begin(); iter!=templ_map_.end(); ++iter)
	{
		QRegExp reg(iter->first.c_str());
		if(reg.regex_match(url)) {
			templ_info=iter->second;
			return TEMPL_OK;
		}
	}
	return TEMPL_ERR;
}

Q_END_NAMESPACE

