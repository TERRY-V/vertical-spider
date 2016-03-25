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

#define TPL_OK           (0)
#define TPL_ERR          (-1)

#define TPL_LABEL_SIZE   (1<<6)
#define TPL_NAME_SIZE    (1<<7)
#define TPL_WEBSITE_SIZE (1<<7)

#define TPL_UNIT_MAX     (10)
#define TPL_ITEM_MAX     (50)
#define TPL_REGEX_MAX    (10)
#define TPL_LABEL_MAX    (5)

Q_BEGIN_NAMESPACE

using namespace tinyxml2;

/* 模板项 */
struct itemInfo {
	char		item_name[TPL_NAME_SIZE];
	uint8_t		item_loop;
	char		item_label[TPL_LABEL_SIZE];
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
	char		unit_name[TPL_NAME_SIZE];
	char		unit_label[TPL_LABEL_SIZE];
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
	int32_t		tid;
	char		website[TPL_WEBSITE_SIZE];
	char		xml_label[TPL_LABEL_SIZE];
	int32_t		unit_num;
	unitInfo*	ptr_tui;

	templInfo() :
		tid(-1),
		unit_num(0),
		ptr_tui(NULL)
	{
		memset(website, '\0', sizeof(website));
		memset(xml_label, '\0', sizeof(xml_label));
	}
};

/* 模板管理类 */
class TemplateManager {
		typedef std::map<uint32_t, templInfo*> utMap;
		typedef std::map<uint32_t, templInfo*>::iterator utMapIter;
		typedef std::map<uint32_t, templInfo*>::const_iterator utMapCIter;

	public:
		/*
		 * @函数名: 模板管理类构造函数
		 * @参数01: 最大模板单元数目
		 * @参数02: 最大模板项数目
		 * @参数03: 最大正则项数目
		 * @参数04: 最大模板标签数目
		 */
		explicit TemplateManager(int32_t unit_max=TPL_UNIT_MAX, \
				int32_t item_max=TPL_ITEM_MAX, \
				int32_t regex_max=TPL_REGEX_MAX, \
				int32_t label_max=TPL_LABEL_MAX) :
			m_tpl_unit_max(unit_max),
			m_tpl_item_max(item_max),
			m_tpl_regex_max(regex_max),
			m_tpl_label_max(label_max)
		{}

		/*
		 * @函数名: 模板析构函数
		 */
		virtual ~TemplateManager();

		/*
		 * @函数名: 初始化函数
		 * @返回值: 成功返回0, 失败返回<0的错误码
		 */
		int32_t init();

		/*
		 * @函数名: 模板添加函数
		 * @参数01: 模板id
		 * @参数02: 模板XML数据内容
		 * @参数03: 模板XML数据内容的实际长度
		 * @返回值: 成功返回0, 失败返回<0的错误码
		 */
		int32_t add_template(int32_t tid, const char* tpl_xml, int32_t tpl_xml_len);

		/*
		 * @函数名: 模板删除函数
		 * @参数01: 模板id
		 * @返回值: 成功返回0, 失败返回<0的错误码
		 */
		int32_t delete_template(int32_t tid);

		/*
		 * @函数名: 模板更新函数
		 * @参数01: 模板id
		 * @参数02: 模板XML数据内容
		 * @参数03: 模板XML数据内容的实际长度
		 * @返回值: 成功返回0, 失败返回<0的错误码
		 */
		int32_t update_template(int32_t tid, const char* tpl_xml, int32_t tpl_xml_len);

		/*
		 * @函数名: 模板获取函数
		 * @参数01: 模板id
		 * @参数02: 模板XML数据内容
		 * @参数03: 模板XML数据内容的实际长度
		 * @返回值: 成功返回0, 失败返回<0的错误码
		 */
		int32_t get_template(int32_t tid, templInfo*& tpl_info);

	private:
		/*
		 * @函数名: 模板编译函数
		 * @参数01: 模板XML数据内容
		 * @参数02: 模板XML数据内容的实际长度
		 * @参数03: 模板编译信息
		 * @返回值: 成功返回0, 失败返回<0的错误码
		 */
		int32_t compile_template(const char* tpl_xml, int32_t tpl_xml_len, templInfo*& tpl_info);

		/*
		 * @函数名: 模板释放函数
		 * @参数01: 模板信息
		 * @返回值: 成功返回0, 失败返回<0的错误码
		 */
		int32_t free_template(templInfo* tpl_info);

	protected:
		Q_DISABLE_COPY(TemplateManager);

		/* 模板映射表 */
		utMap		m_tpl_map;
		QMutexLock	m_tpl_map_mutex;

		/* 相关阈值 */
		int32_t		m_tpl_unit_max;
		int32_t		m_tpl_item_max;
		int32_t		m_tpl_regex_max;
		int32_t		m_tpl_label_max;
};

Q_END_NAMESPACE

#endif // __TEMPLATEMANAGER_H_
