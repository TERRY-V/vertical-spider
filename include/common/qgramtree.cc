#include "qgramtree.h"

Q_BEGIN_NAMESPACE

QGramTree::QGramTree()
{
}

QGramTree::~QGramTree()
{
}

int32_t QGramTree::init()
{
	std::string contentOfAll;
	try {
		contentOfAll=QFile::readAll(GRAM_DEFAULT_FILE);
	} catch(std::runtime_error& e) {
		Q_INFO("Loading (%s) failed, (%s)!", GRAM_DEFAULT_FILE, e.what());
		return GRAM_ERR;
	}

	std::vector<std::string> lines=q_line_tokenize(contentOfAll);
	if(parseFromString(lines)<0) {
		Q_INFO("Loading gram tree failed!");
		return GRAM_ERR;
	}

	Q_INFO("Loading gram tree successful...");
	return GRAM_OK;
}

int32_t QGramTree::parseFromString(std::vector<std::string>& lines)
{
	std::string groupName;
	std::string strKey;
	uint64_t uint64Key(0);

	for(size_t i=0; i<lines.size(); ++i)
	{
		if(lines[i].empty()||q_starts_with(lines[i], '~')||q_starts_with(lines[i], "//"))
			continue;

		if(q_starts_with(lines[i], '%')) {
			groupName=lines[i].substr(1);
			if(groupName.empty()) {
				Q_INFO("Invalid gram groupName: (%s)!", lines[i].c_str());
				return GRAM_ERR;
			}
		} else {
			std::string meaning;
			if(getIndexMeaning(lines[i], meaning))
				return GRAM_ERR;

			strKey=groupName+'_'+meaning;
			uint64Key=md5.MD5Bits64(reinterpret_cast<unsigned char*>((char*)strKey.c_str()), strKey.length());

			TreeMapIter iter=treeMap.find(uint64Key);
			if(iter==treeMap.end()) {
				std::vector<std::string> meaningVec;
				meaningVec.push_back(lines[i]);
				treeMap.insert(std::make_pair< uint64_t, std::vector<std::string> >(uint64Key, meaningVec));
			} else {
				iter->second.push_back(lines[i]);
			}
		}
	}

	return GRAM_OK;
}

int32_t QGramTree::query(const std::string& groupName, const std::string& meaning, std::vector<std::string>& grams)
{
	if(groupName.empty()||meaning.empty())
		return GRAM_ERR;

	std::string strKey=groupName+"_"+meaning;
	uint64_t uint64Key=md5.MD5Bits64(reinterpret_cast<unsigned char*>((char*)strKey.c_str()), strKey.length());

	TreeMapIter iter=treeMap.find(uint64Key);
	if(iter==treeMap.end())
		return GRAM_ERR;

	grams.clear();
	grams.insert(grams.end(), iter->second.begin(), iter->second.end());

	return GRAM_OK;
}

int32_t QGramTree::getIndexMeaning(const std::string& gram, std::string& meaning)
{
	size_t pos=gram.find('=');
	if(pos==gram.npos) {
		Q_INFO("Please check your gram: (%s), lack of '='", gram.c_str());
		return GRAM_ERR;
	}

	std::vector<std::string> meanings=q_split(gram.substr(0, pos), '/');
	for(size_t i=0; i<meanings.size(); ++i)
	{
		if(meanings[i].length()&&meanings[i].at(0)=='*')
		{
			std::vector<std::string> indexMeanings=q_split_any(meanings[i].substr(1), "&|!");
			if(!indexMeanings.size()) {
				Q_INFO("Please check your gram: (%s), lack of '*' items", gram.c_str());
				return GRAM_ERR;
			}

			meaning=indexMeanings.at(0);
			return GRAM_OK;
		}
	}

	Q_INFO("Please check your gram: (%s), lack of '*'", gram.c_str());
	return GRAM_ERR;
}

Q_END_NAMESPACE
