/********************************************************************************************
**
** Copyright (C) 2010-2015 Terry Niu (Beijing, China)
** Filename:	templatemanager.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2015/09/22
**
*********************************************************************************************/

#ifndef __TEMPLATEMANAGER_H_
#define __TEMPLATEMANAGER_H_

#include <qglobal.h>
#include <qfunc.h>
#include <qregexp.h>
#include <tinyxml2.h>

#define TEMPL_OK           (0)
#define TEMPL_ERR          (-1)

#define TEMPL_LABEL_SIZE   (1<<6)
#define TEMPL_NAME_SIZE    (1<<7)
#define TEMPL_WEBSITE_SIZE (1<<7)
#define TEMPL_URL_SIZE     (1<<8)
#define TEMPL_PATTERN_SIZE (1<<8)

#define TEMPL_UNIT_MAX     (10)
#define TEMPL_ITEM_MAX     (50)
#define TEMPL_REGEX_MAX    (10)
#define TEMPL_LABEL_MAX    (5)

Q_BEGIN_NAMESPACE

using namespace tinyxml2;

/* 模板项 */
struct itemInfo {
	char		item_name[TEMPL_NAME_SIZE];
	uint8_t		item_loop;
	char		item_label[TEMPL_LABEL_SIZE];
	uint8_t		item_relation;
	int32_t		regex_num;
	pcre**		regex_handle;
	int32_t		label_num;
	char**		label_handle;

	itemInfo() :
		item_loop(0),
		item_relation(1),
		regex_num(0),
		regex_handle(NULL),
		label_num(0),
		label_handle(NULL)
	{
		memset(item_name, '\0', sizeof(item_name));
		memset(item_label, '\0', sizeof(item_label));
	}
};

/* 模板单元 */
struct unitInfo {
	char		unit_name[TEMPL_NAME_SIZE];
	char		unit_label[TEMPL_LABEL_SIZE];
	int32_t		item_num;
	itemInfo*	ptr_tii;

	unitInfo() :
		item_num(0),
		ptr_tii(NULL)
	{
		memset(unit_name, '\0', sizeof(unit_name));
		memset(unit_label, '\0', sizeof(unit_label));
	}
};

/* 模板信息 */
struct templInfo {
	char		website[TEMPL_WEBSITE_SIZE];
	char		site_url[TEMPL_URL_SIZE];
	char		xml_label[TEMPL_LABEL_SIZE];

	char		url_pattern[TEMPL_PATTERN_SIZE];
	bool		wflag;
	int32_t		unit_num;
	unitInfo*	ptr_tui;

	templInfo() :
		unit_num(0),
		ptr_tui(NULL)
	{
		memset(website, '\0', sizeof(website));
		memset(site_url, '\0', sizeof(site_url));
		memset(xml_label, '\0', sizeof(xml_label));
		memset(url_pattern, '\0', sizeof(url_pattern));
	}
};

/* 模板管理类 */
class TemplateManager {
		typedef std::map<std::string, templInfo*> utMap;
		typedef std::map<std::string, templInfo*>::iterator utMapIter;
		typedef std::map<std::string, templInfo*>::const_iterator utMapCIter;

	public:
		/*
		 * @函数名: 模板管理类构造函数
		 * @参数01: 最大模板单元数目
		 * @参数02: 最大模板项数目
		 * @参数03: 最大正则项数目
		 * @参数04: 最大模板标签数目
		 */
		explicit TemplateManager(int32_t unit_max=TEMPL_UNIT_MAX, \
				int32_t item_max=TEMPL_ITEM_MAX, \
				int32_t regex_max=TEMPL_REGEX_MAX, \
				int32_t label_max=TEMPL_LABEL_MAX) :
			templ_unit_max_(unit_max),
			templ_item_max_(item_max),
			templ_regex_max_(regex_max),
			templ_label_max_(label_max)
		{}

		/*
		 * @函数名: 模板析构函数
		 */
		virtual ~TemplateManager();

		/*
		 * @函数名: 初始化函数
		 * @返回值: 成功返回0, 失败返回<0的错误码
		 */
		int32_t init(const char* templ_xml, int32_t templ_len);

		/*
		 * @函数名: 模板更新函数
		 * @参数01: 模板XML数据内容
		 * @参数02: 模板XML数据内容的实际长度
		 * @返回值: 成功返回0, 失败返回<0的错误码
		 */
		int32_t update_template(const char* templ_xml, int32_t templ_len);

		/*
		 * @函数名: 模板获取函数
		 * @参数01: 待匹配URL
		 * @参数02: 模板XML数据内容
		 * @返回值: 成功返回0, 失败返回<0的错误码
		 */
		int32_t get_template(const std::string& url, templInfo*& tpl_info);

	private:
		/*
		 * @函数名: 模板编译函数
		 * @参数01: 模板XML数据内容
		 * @参数02: 模板XML数据内容的实际长度
		 * @返回值: 成功返回0, 失败返回<0的错误码
		 */
		int32_t compile_template(const char* tpl_xml, int32_t tpl_xml_len);

		/*
		 * @函数名: 模板释放函数
		 * @返回值: 成功返回0, 失败返回<0的错误码
		 */
		int32_t free_template();

	protected:
		Q_DISABLE_COPY(TemplateManager);

		/* 模板映射表 */
		utMap		templ_map_;
		QMutexLock	templ_mutex_;

		/* 相关阈值 */
		int32_t		templ_unit_max_;
		int32_t		templ_item_max_;
		int32_t		templ_regex_max_;
		int32_t		templ_label_max_;
};

Q_END_NAMESPACE

#endif // __TEMPLATEMANAGER_H_
