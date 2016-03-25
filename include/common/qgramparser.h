/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qgramparser.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2015/11/17
**
*********************************************************************************************/
#ifndef __QGRAMPARSER_H_
#define __QGRAMPARSER_H_

#include "qglobal.h"
#include "qfunc.h"
#include "qgramtree.h"
#include "qlogicalparser.h"

#define PARSE_OK  (0)
#define PARSE_ERR (-1)

Q_BEGIN_NAMESPACE

class QGramParser {
	public:
		explicit QGramParser();

		virtual ~QGramParser();

		int32_t init();

		int32_t str2listPOS(const std::string& strPOS, std::list< std::vector<std::string> >& listPOS);

		std::string listPOS2str(const std::list< std::vector<std::string> >& listPOS);

		int32_t parse(const std::string& groupName, std::list< std::vector<std::string> >& listPOS);

	private:
		bool parse_gram(std::list< std::vector<std::string> >& listPOS, \
				std::list< std::vector<std::string> >::iterator& it, \
				const std::string& gram);

		int32_t split_gram(const std::string& gram, std::list<std::string>& leftMeanings, \
				std::list<std::string>& rightMeanings);

		void getVariousMeanings(const std::list< std::vector<std::string> >& listPOS, \
				const std::map<size_t, std::list< std::vector<std::string> >::iterator>& iterMap, \
				const std::string& grammarItem, \
				std::vector<std::string>& meanings);

	protected:
		QGramTree* treeMap;
};

Q_END_NAMESPACE

#endif // __QGRAMPARSER_H_
