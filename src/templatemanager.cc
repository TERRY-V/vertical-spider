#include "templatemanager.h"

Q_BEGIN_NAMESPACE

TemplateManager::~TemplateManager()
{
	utMapIter iter=m_tpl_map.begin();
	while(iter!=m_tpl_map.end())
	{
		free_template(iter->second);
		++iter;
	}
}

int32_t TemplateManager::init()
{
	return TPL_OK;
}

int32_t TemplateManager::compile_template(const char* tpl_xml, int32_t tpl_xml_len, templInfo*& tpl_info)
{
	if(tpl_xml==NULL||tpl_xml_len<=0)
		return -1;

	/* memory leaks once creating the template fails */
	tpl_info=q_new<templInfo>();
	if(tpl_info==NULL)
		return -2;

	tpl_info->ptr_tui=q_new_array<unitInfo>(m_tpl_unit_max);
	if(tpl_info->ptr_tui==NULL)
		return -3;

	for(int32_t i=0; i<m_tpl_unit_max; ++i)
	{
		tpl_info->ptr_tui[i].ptr_tii=q_new_array<itemInfo>(m_tpl_item_max);
		if(tpl_info->ptr_tui[i].ptr_tii==NULL)
			return -4;

		for(int32_t j=0; j<m_tpl_item_max; ++j)
		{
			tpl_info->ptr_tui[i].ptr_tii[j].regex_handle=q_new_array<pcre*>(m_tpl_regex_max);
			if(tpl_info->ptr_tui[i].ptr_tii[j].regex_handle==NULL)
				return -5;

			tpl_info->ptr_tui[i].ptr_tii[j].label_handle=q_new_array<char*>(m_tpl_label_max);
			if(tpl_info->ptr_tui[i].ptr_tii[j].label_handle==NULL)
				return -6;

			for(int32_t k=0; k<m_tpl_label_max; ++k)
			{
				tpl_info->ptr_tui[i].ptr_tii[j].label_handle[k]=q_new_array<char>(BUFSIZ_1K);
				if(tpl_info->ptr_tui[i].ptr_tii[j].label_handle[k]==NULL)
					return -7;
			}
		}
	}

	/* parse and compile template */
	XMLDocument doc;
	doc.Parse(tpl_xml, tpl_xml_len);
	if(doc.Error()) {
		Q_INFO("XML parse failed (%s)!", doc.GetErrorStr1());
		return -11;
	}

	XMLElement* templElement=doc.FirstChildElement("templ");
	if(templElement==NULL)
		return -12;

	XMLElement *baseElement=templElement->FirstChildElement("base");
	if(baseElement==NULL)
		return -13;

	XMLElement *websiteElement=baseElement->FirstChildElement("website");
	if(websiteElement&&websiteElement->GetText()) {
		::strcpy(tpl_info->website, websiteElement->GetText());
	} else {
		return -14;
	}

	XMLElement *xml_labelElement=baseElement->FirstChildElement("xmllabel");
	if(xml_labelElement&&xml_labelElement->GetText()) {
		::strcpy(tpl_info->xml_label, xml_labelElement->GetText());
	} else {
		return -15;
	}

	/* units */
	XMLElement* unitsElement=templElement->FirstChildElement("units");
	if(unitsElement==NULL)
		return -16;

	int32_t thisUnitNum=tpl_info->unit_num;

	XMLElement* unitElement=unitsElement->FirstChildElement("unit");
	while(unitElement!=NULL)
	{
		if(thisUnitNum+1>m_tpl_unit_max)
			return -17;

		XMLElement* unit_nameElement=unitElement->FirstChildElement("unitname");
		if(unit_nameElement&&unit_nameElement->GetText()) {
			strcpy(tpl_info->ptr_tui[thisUnitNum].unit_name, unit_nameElement->GetText());
		} else {
			return -18;
		}

		XMLElement* unitlabelElement=unitElement->FirstChildElement("unitlabel");
		if(unitlabelElement&&unitlabelElement->GetText()) {
			strcpy(tpl_info->ptr_tui[thisUnitNum].unit_label, unitlabelElement->GetText());
		} else {
			return -19;
		}

		/* items */
		XMLElement* itemsElement=unitElement->FirstChildElement("items");
		if(itemsElement==NULL)
			return -20;

		int32_t thisItemNum=tpl_info->ptr_tui[thisUnitNum].item_num;

		XMLElement* itemElement=itemsElement->FirstChildElement("item");
		while(itemElement!=NULL)
		{
			if(thisItemNum+1>m_tpl_item_max)
				return -21;

			XMLElement* itemnameElement=itemElement->FirstChildElement("itemname");
			if(itemnameElement&&itemnameElement->GetText()) {
				strcpy(tpl_info->ptr_tui[thisUnitNum].ptr_tii[thisItemNum].item_name, itemnameElement->GetText());
			} else {
				return -22;
			}

			XMLElement* itemloopElement=itemElement->FirstChildElement("itemloop");
			if(itemloopElement&&itemloopElement->GetText()) {
				tpl_info->ptr_tui[thisUnitNum].ptr_tii[thisItemNum].item_loop=strncmp(itemloopElement->GetText(), "TRUE", 4)==0?1:0;
			} else {
				return -23;
			}

			if(tpl_info->ptr_tui[thisUnitNum].ptr_tii[thisItemNum].item_loop==1)
			{
				XMLElement* itemlabelElement=itemElement->FirstChildElement("itemlabel");
				if(itemlabelElement&&itemlabelElement->GetText()) {
					strcpy(tpl_info->ptr_tui[thisUnitNum].ptr_tii[thisItemNum].item_label, itemlabelElement->GetText());
				} else {
					return -24;
				}
			}

			XMLElement* item_relationElement=itemElement->FirstChildElement("itemrelation");
			if(item_relationElement&&item_relationElement->GetText()) {
				tpl_info->ptr_tui[thisUnitNum].ptr_tii[thisItemNum].item_relation=(strncmp(item_relationElement->GetText(), "AND", 3)==0)?1:0;
			} else {
				return -25;
			}

			/* regexes */
			XMLElement* regexesElement=itemElement->FirstChildElement("regexes");
			if(regexesElement==NULL)
				return -26;

			int32_t thisRegexNum=tpl_info->ptr_tui[thisUnitNum].ptr_tii[thisItemNum].regex_num;

			XMLElement* regexElement=regexesElement->FirstChildElement("regex");
			while(regexElement!=NULL)
			{
				if(thisRegexNum+1>m_tpl_regex_max)
					return -27;

				if(regexElement->GetText()) {
					char* error=NULL;
					int32_t error_offset=0;
					pcre* reg=pcre_compile(regexElement->GetText(), 0, (const char**)&error, &error_offset, NULL);
					if(reg==NULL) {
						Q_INFO("PCRE compilation (%s) failed at offset %d : %s", regexElement->GetText(), error_offset, error);
						return -28;
					}
					tpl_info->ptr_tui[thisUnitNum].ptr_tii[thisItemNum].regex_handle[thisRegexNum]=reg;
				} else {
					Q_INFO("Empty PCRE pattern (%s) failed in compilation", regexElement->GetText());
					return -29;
				}

				regexElement=regexElement->NextSiblingElement();
				tpl_info->ptr_tui[thisUnitNum].ptr_tii[thisItemNum].regex_num=++thisRegexNum;
			}

			/* labels */
			XMLElement* labelsElement=itemElement->FirstChildElement("labels");
			if(labelsElement==NULL)
				return -30;

			int32_t thisLabelNum=tpl_info->ptr_tui[thisUnitNum].ptr_tii[thisItemNum].label_num;

			XMLElement* labelElement=labelsElement->FirstChildElement("label");
			while(labelElement!=NULL)
			{
				if(thisLabelNum+1>m_tpl_label_max)
					return -31;

				if(labelElement->GetText()) {
					char* pszLabel=tpl_info->ptr_tui[thisUnitNum].ptr_tii[thisItemNum].label_handle[thisLabelNum];
					strcpy(pszLabel, labelElement->GetText());
				} else {
					Q_DEBUG("Empty label tag: (%s)", labelElement->GetText());
					return -32;
				}

				labelElement=labelElement->NextSiblingElement();
				tpl_info->ptr_tui[thisUnitNum].ptr_tii[thisItemNum].label_num=++thisLabelNum;
			}

			itemElement=itemElement->NextSiblingElement();
			tpl_info->ptr_tui[thisUnitNum].item_num=++thisItemNum;
		}

		unitElement=unitElement->NextSiblingElement();
		tpl_info->unit_num=++thisUnitNum;
	}

	return TPL_OK;
}

int32_t TemplateManager::free_template(templInfo* tpl_info)
{
	if(tpl_info==NULL)
		return -1;

	for(int32_t i=0; i<tpl_info->unit_num; ++i)
	{
		for(int32_t j=0; j<tpl_info->ptr_tui[i].item_num; ++j)
		{
			for(int32_t k=0; k<tpl_info->ptr_tui[i].ptr_tii[j].regex_num; ++k)
			{
				pcre_free(tpl_info->ptr_tui[i].ptr_tii[j].regex_handle[k]);
				tpl_info->ptr_tui[i].ptr_tii[j].regex_handle[k]=NULL;
			}

			q_delete_array<pcre*>(tpl_info->ptr_tui[i].ptr_tii[j].regex_handle);
			tpl_info->ptr_tui[i].ptr_tii[j].regex_handle=NULL;

			for(int32_t k=0; k<tpl_info->ptr_tui[i].ptr_tii[j].label_num; ++k)
			{
				q_delete_array<char>(tpl_info->ptr_tui[i].ptr_tii[j].label_handle[k]);
				tpl_info->ptr_tui[i].ptr_tii[j].label_handle[k]=NULL;
			}

			q_delete_array<char*>(tpl_info->ptr_tui[i].ptr_tii[j].label_handle);
			tpl_info->ptr_tui[i].ptr_tii[j].label_handle=NULL;
		}

		q_delete_array<itemInfo>(tpl_info->ptr_tui[i].ptr_tii);
		tpl_info->ptr_tui[i].ptr_tii=NULL;
	}

	q_delete_array<unitInfo>(tpl_info->ptr_tui);
	tpl_info->ptr_tui=NULL;

	q_delete<templInfo>(tpl_info);
	tpl_info=NULL;

	return TPL_OK;
}

int32_t TemplateManager::add_template(int32_t tid, const char* tpl_xml, int32_t tpl_xml_len)
{
	if(tid<0||tpl_xml==NULL||tpl_xml_len<=0)
		return -1;

	templInfo* tpl_info=NULL;
	int32_t ret=0;

	ret=compile_template(tpl_xml, tpl_xml_len, tpl_info);
	if(ret<0||tpl_info==NULL)
		return -2;

	m_tpl_map_mutex.lock();

	utMapIter iter=m_tpl_map.find(tid);
	if(iter==m_tpl_map.end()) {
		m_tpl_map.insert(std::make_pair(tid, tpl_info));
	} else {
		if(free_template(iter->second)<0) {
			m_tpl_map_mutex.unlock();
			return -3;
		}
		iter->second=tpl_info;
	}

	m_tpl_map_mutex.unlock();
	return TPL_OK;
}

int32_t TemplateManager::delete_template(int32_t tid)
{
	if(tid<0)
		return TPL_ERR;

	m_tpl_map_mutex.lock();

	utMapIter iter=m_tpl_map.find(tid);
	if(iter!=m_tpl_map.end()) {
		if(free_template(iter->second)<0) {
			m_tpl_map_mutex.unlock();
			return TPL_ERR;
		}
		m_tpl_map.erase(iter);
	}

	m_tpl_map_mutex.unlock();
	return TPL_OK;
}

int32_t TemplateManager::update_template(int32_t tid, const char* tpl_xml, int32_t tpl_xml_len)
{
	if(tid<0||tpl_xml==NULL||tpl_xml_len<=0)
		return -1;

	templInfo* tpl_info=NULL;
	int32_t ret=0;

	ret=compile_template(tpl_xml, tpl_xml_len, tpl_info);
	if(ret<0||tpl_info==NULL)
		return -2;

	m_tpl_map_mutex.lock();

	utMapIter iter=m_tpl_map.find(tid);
	if(iter==m_tpl_map.end()) {
		m_tpl_map.insert(std::make_pair(tid, tpl_info));
	} else {
		if(free_template(iter->second)<0) {
			m_tpl_map_mutex.unlock();
			return -3;
		}
		iter->second=tpl_info;
	}

	m_tpl_map_mutex.unlock();
	return TPL_OK;
}

int32_t TemplateManager::get_template(int32_t tid, templInfo*& tpl_info)
{
	m_tpl_map_mutex.lock();

	utMapIter iter=m_tpl_map.find(tid);
	if(iter!=m_tpl_map.end()) {
		tpl_info=iter->second;
	} else {
		m_tpl_map_mutex.unlock();
		return TPL_ERR;
	}

	m_tpl_map_mutex.unlock();
	return TPL_OK;
}

Q_END_NAMESPACE

