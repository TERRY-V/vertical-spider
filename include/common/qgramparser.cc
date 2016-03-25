#include "qgramparser.h"

Q_BEGIN_NAMESPACE

QGramParser::QGramParser() :
	treeMap(NULL)
{
}

QGramParser::~QGramParser()
{
	q_delete<QGramTree>(treeMap);
}

int32_t QGramParser::init()
{
	treeMap=q_new<QGramTree>();
	if(treeMap==NULL) {
		Q_INFO("Error: treeMap is null!");
		return PARSE_ERR;
	}

	int32_t ret=treeMap->init();
	if(ret<0) {
		Q_INFO("Error: treeMap init error, ret = (%d)!", ret);
		return PARSE_ERR;
	}

	return PARSE_OK;
}

int32_t QGramParser::str2listPOS(const std::string& strPOS, std::list< std::vector<std::string> >& listPOS)
{
	if(!strPOS.length() || strPOS.at(0)!='(' || strPOS.at(strPOS.length()-1)!=')')
		return PARSE_ERR;

	std::vector<std::string> words=q_split(strPOS.substr(1, strPOS.length()-2), "),(");
	for(size_t i=0; i<words.size(); ++i)
	{
		std::vector<std::string> meanings=q_split_any(words.at(i), "> ");
		listPOS.push_back(meanings);
	}

	return PARSE_OK;
}

std::string QGramParser::listPOS2str(const std::list< std::vector<std::string> >& listPOS)
{
	std::string strPOS;
	for(std::list< std::vector<std::string> >::const_iterator it=listPOS.begin(); \
			it!=listPOS.end(); \
			++it)
	{
		strPOS.append("(");
		for(size_t i=0; i<it->size(); ++i) {
			strPOS.append(it->at(i));
			if(i==0) strPOS.append(">");
			else if(i==it->size()-1) break;
			else strPOS.append(" ");
		}
		strPOS.append("),");
	}
	return strPOS;
}

int32_t QGramParser::parse(const std::string& groupName, std::list< std::vector<std::string> >& listPOS)
{
	if(!groupName.length() || !listPOS.size())
		return PARSE_ERR;

	std::vector<std::string> grams;
	bool bValue(false);

	// 计算量偏大, 待后期优化
	std::list< std::vector<std::string> >::iterator it=--listPOS.end();
	while(it!=listPOS.end())
	{
		bValue=false;
		for(size_t i(1); !bValue && i<it->size(); ++i) {
			if(treeMap->query(groupName, it->at(i), grams)==GRAM_ERR)
				continue;
			for(size_t j(0); !bValue && j<grams.size(); ++j) {
				bValue=parse_gram(listPOS, it, grams[j]);
#if 0
				if(bValue) printf("Parse OK: [%s] %s\n", groupName.c_str(), grams[j].c_str());
#endif
			}

		}
		if(!bValue) --it;
	}

	return PARSE_OK;
}

bool QGramParser::parse_gram(std::list< std::vector<std::string> >& listPOS, std::list< std::vector<std::string> >::iterator& it, const std::string& gram)
{
	std::list<std::string> leftMeanings;
	std::list<std::string> rightMeanings;

	std::list<std::string>::iterator asteriskIter, itemIter;
	size_t asteriskPos(0);
	size_t itemPos(0);

	std::list< std::vector<std::string> >::iterator posIter;
	std::map<size_t, std::list< std::vector<std::string> >::iterator > iterMap;

	QLogicalParser lp;

	// Split the grammar into two groups
	if(split_gram(gram, leftMeanings, rightMeanings)==PARSE_ERR)
		return false;

	// The following steps calculates the position of asterisk
	for(asteriskIter=leftMeanings.begin(); asteriskIter!=leftMeanings.end(); ++asteriskIter)
	{
		if(asteriskIter->at(0)=='<') continue;
		++asteriskPos;
		if(asteriskIter->at(0)=='*') break;
	}

	// The following steps match grammar items after the asterisk
	posIter=it;
	itemIter=asteriskIter;
	itemPos=asteriskPos;

	while(itemIter!=leftMeanings.end())
	{
		if(posIter==listPOS.end()) {
			for(; itemIter!=leftMeanings.end(); ++itemIter)
			{
				if(itemIter->at(0)=='(') {
					iterMap.insert(std::make_pair(itemPos, listPOS.end()));
				} else if(itemIter->at(0)=='<') {
					if(!lp.parse(itemIter->substr(1, itemIter->length()-2), std::string(), std::vector<std::string>()))
						return false;
				} else {
					return false;
				}
				itemPos++;
			}
			break;
		}

		if(itemIter->at(0)=='*') {
			if(!lp.parse(itemIter->substr(1), posIter->at(0), std::vector<std::string>(posIter->begin()+1, posIter->end())))
				return false;
			iterMap.insert(std::make_pair(itemPos, posIter));
			itemPos++;
			itemIter++;
			posIter++;
		} else if(itemIter->at(0)=='(') {
			if(lp.parse(itemIter->substr(1, itemIter->length()-2), posIter->at(0), std::vector<std::string>(posIter->begin()+1, posIter->end()))) {
				iterMap.insert(std::make_pair(itemPos, posIter));
				posIter++;
			} else {
				iterMap.insert(std::make_pair(itemPos, listPOS.end()));
			}
			itemPos++;
			itemIter++;
		} else if(itemIter->at(0)=='<') {
			if(!lp.parse(itemIter->substr(1, itemIter->length()-2), posIter->at(0), std::vector<std::string>(posIter->begin()+1, posIter->end())))
				return false;
			itemIter++;
			posIter++;
		} else {
			if(!lp.parse(*itemIter, posIter->at(0), std::vector<std::string>(posIter->begin()+1, posIter->end())))
				return false;
			iterMap.insert(std::make_pair(itemPos, posIter));
			itemPos++;
			itemIter++;
			posIter++;
		}
	}

	// The following steps match grammar items before the asterisk.
	itemIter=asteriskIter;
	itemIter--;
	itemPos=asteriskPos-1;
	posIter=it;
	posIter--;

	while(itemIter!=leftMeanings.end())
	{
		if(posIter==listPOS.end()) {
			for(; itemIter!=leftMeanings.end(); --itemIter) {
				if(itemIter->at(0)=='(') {
					iterMap.insert(std::make_pair(itemPos, listPOS.end()));
				} else if(itemIter->at(0)=='<') {
					if(!lp.parse(itemIter->substr(1, itemIter->length()-2), std::string(), std::vector<std::string>()))
						return false;
				} else {
					return false;
				}
				itemPos--;
			}
			break;
		}

		if(itemIter->at(0)=='(') {
			if(lp.parse(itemIter->substr(1, itemIter->length()-2), posIter->at(0), std::vector<std::string>(posIter->begin()+1, posIter->end()))) {
				iterMap.insert(std::make_pair(itemPos, posIter));
				posIter--;
			} else {
				iterMap.insert(std::make_pair(itemPos, listPOS.end()));
			}
			itemPos--;
			itemIter--;
		} else if(itemIter->at(0)=='<') {
			if(!lp.parse(itemIter->substr(1, itemIter->length()-2), posIter->at(0), std::vector<std::string>(posIter->begin()+1, posIter->end())))
				return false;
			itemIter--;
			posIter--;
		} else {
			if(!lp.parse(*itemIter, posIter->at(0), std::vector<std::string>(posIter->begin()+1, posIter->end())))
				return false;
			iterMap.insert(std::make_pair(itemPos, posIter));
			itemPos--;
			itemIter--;
			posIter--;
		}
	}

	// Well, congratulations, the grammar was successfully matched.
	// The following steps mainly process the sentence in accordance with the right part of the gramamr.
	std::list< std::vector<std::string> > listPOS_new;
	itemIter=rightMeanings.begin();

	if(itemIter->at(0)=='@')
	{
		std::string keyword;
		std::vector<std::string> meanings;
		for(std::map<size_t, std::list< std::vector<std::string> >::iterator >::iterator iter=iterMap.begin(); \
				iter!=iterMap.end(); \
				++iter) {
			if(iter->second!=listPOS.end())
				keyword+=iter->second->at(0);
		}
		getVariousMeanings(listPOS, iterMap, *itemIter, meanings);
		meanings.insert(meanings.begin(), keyword);
		listPOS_new.push_back(meanings);
	} else {
		for(; itemIter!=rightMeanings.end(); ++itemIter) {
			std::vector<std::string> meanings;
			getVariousMeanings(listPOS, iterMap, *itemIter, meanings);
			listPOS_new.push_back(meanings);
		}
	}

	// The following steps mainly operate the original list
	std::map<size_t, std::list< std::vector<std::string> >::iterator >::iterator it1=iterMap.begin();
	while(it1!=iterMap.end()&&it1->second==listPOS.end()) ++it1;

	std::map<size_t, std::list< std::vector<std::string> >::iterator >::reverse_iterator it2=iterMap.rbegin();
	while(it2!=iterMap.rend()&&it2->second==listPOS.end()) ++it2;

	posIter=it1->second;
	it=--posIter;

	if(it1->second==it2->second) listPOS.erase(it1->second);
	else listPOS.erase(it1->second, ++(it2->second));

	listPOS.insert(++posIter, listPOS_new.begin(), listPOS_new.end());
	return true;
}

int32_t QGramParser::split_gram(const std::string& gram, std::list<std::string>& leftMeanings, std::list<std::string>& rightMeanings)
{
	std::vector<std::string> lrParts=q_split(gram, '=');
	if(lrParts.size()!=2)
		return PARSE_ERR;

	q_split(lrParts.at(0), '/', leftMeanings);
	if(leftMeanings.empty())
		return PARSE_ERR;

	q_split(lrParts.at(1), ' ', rightMeanings);
	if(rightMeanings.empty())
		return PARSE_ERR;

	return PARSE_OK;
}

void QGramParser::getVariousMeanings(const std::list< std::vector<std::string> >& listPOS, \
		const std::map<size_t, std::list< std::vector<std::string> >::iterator>& iterMap, \
		const std::string& gramItem, \
		std::vector<std::string>& meanings)
{
	std::vector<std::string> items;
	q_split_any(gramItem, "[]", items);
	assert(items.size()>0);

	int32_t itemPos(0);
	std::list< std::vector<std::string> >::iterator t;

	if(items[0].length()>1 && items[0].at(0)=='N' && isdigit(items[0].at(1)))
	{
		itemPos=atoi(items[0].substr(1).c_str());
		t=iterMap.at(itemPos);
		if(t!=listPOS.end()) meanings.insert(meanings.end(), t->begin(), t->end());
	}

	if(items.size()>1)
	{
		std::vector<std::string> childItems;
		q_split(items[1], '+', childItems);
		meanings.insert(meanings.end(), childItems.begin(), childItems.end());
	}
}

Q_END_NAMESPACE
