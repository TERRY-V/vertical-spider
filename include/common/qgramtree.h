/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qgramtree.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2015/11/17
**
*********************************************************************************************/
#ifndef __QGRAMTREE_H_
#define __QGRAMTREE_H_

#include "qglobal.h"
#include "qmd5.h"
#include "qfile.h"
#include "qfunc.h"

#define GRAM_OK          (0)
#define GRAM_ERR         (-1)

#define GRAM_DEFAULT_FILE ("__trans.gram")

Q_BEGIN_NAMESPACE

class QGramTree {
		typedef std::map< uint64_t, std::vector<std::string> > TreeMap;
		typedef std::map< uint64_t, std::vector<std::string> >::iterator TreeMapIter;

	public:
		explicit QGramTree();
		virtual ~QGramTree();

		int32_t init();

		int32_t query(const std::string& groupName, const std::string& meaning, std::vector<std::string>& grams);

	private:
		int32_t parseFromString(std::vector<std::string>& lines);

		int32_t getIndexMeaning(const std::string& gram, std::string& meaning);

	protected:
		QMD5		md5;
		TreeMap		treeMap;
};

Q_END_NAMESPACE

#endif // __QGRAMTREE_H_
