#include "qsimhasher.h"

Q_BEGIN_NAMESPACE

QSimHasher::QSimHasher()
{
	tokenizer=q_new<QWordTokenizer>();
	Q_ASSERT(tokenizer!=NULL, "alloc error, tokenizer is null!");
}

QSimHasher::~QSimHasher()
{
	q_delete<QWordTokenizer>(tokenizer);
}

int32_t QSimHasher::init()
{
	int32_t ret=tokenizer->init();
	if(ret<0) {
		Q_INFO("QWordTokenizer: init failed, ret = (%d)!", ret);
		return SIM_ERR;
	}

	return SIM_OK;
}

int32_t QSimHasher::extract(const std::string& text, int32_t topN, std::vector< std::pair<std::string, double> >& keywords)
{
	char tokens[4<<10]={0};
	int32_t ret=0;

	ret=tokenizer->word_tokenize(text.c_str(), text.length(), tokens, sizeof(tokens));
	if(ret<0) {
		Q_INFO("QWordTokenizer: word_tokenize error, ret = (%d)!", ret);
		return SIM_ERR;
	}

	std::list< std::vector<std::string> > listPOS;
	ret=str2listPOS(std::string(tokens, ret), listPOS);
	if(ret<0) {
		Q_INFO("QWordTokenizer: str2listPOS error, ret = (%d)!", ret);
		return SIM_ERR;
	}

	std::map<std::string, double> wordMap;
	for(std::list< std::vector<std::string> >::iterator iter=listPOS.begin(); \
			iter!=listPOS.end(); \
			++iter) {
		if(find(iter->begin(), iter->end(), "STOP_W")!=iter->end())
			continue;
		if(find(iter->begin(), iter->end(), "O")!=iter->end())
			continue;
		wordMap[iter->at(0)]+=1.0;
	}

	keywords.clear();
	std::copy(wordMap.begin(), wordMap.end(), std::inserter(keywords, keywords.begin()));

	topN=static_cast<int32_t>(keywords.size())<topN?keywords.size():topN;
	std::partial_sort(keywords.begin(), keywords.begin()+topN, keywords.end(), Compare);
	keywords.resize(topN);

	return SIM_OK;
}

void QSimHasher::printKeyWords(const std::vector< std::pair<std::string, double> >& keywords)
{
	std::cout<<"Keywords: "<<std::endl;
	for(std::vector< std::pair<std::string, double> >::const_iterator it=keywords.begin(); \
			it!=keywords.end(); \
			++it) {
		std::cout<<'('<<it->first<<':'<<it->second<<')';
	}
	std::cout<<std::endl;
}

int32_t QSimHasher::calculateSimHash(const std::vector< std::pair<std::string, double> >& wordWeights, uint64_t& simVal)
{
	if(wordWeights.empty())
		return SIM_ERR;

	std::vector< std::pair<uint64_t, double> > hashValues;
	std::vector<double> weights(64, 0.0);
	const uint64_t u64(1);

	hashValues.resize(wordWeights.size());
	for(size_t i=0; i<hashValues.size(); ++i)
	{
		hashValues[i].first=md5.MD5Bits64((unsigned char*)(wordWeights[i].first.c_str()), wordWeights[i].first.length());
		hashValues[i].second=wordWeights[i].second;
	}

	for(size_t i=0; i<hashValues.size(); ++i)
	{
		for(size_t j=0; j<64; ++j)
			weights[j] += (((u64 << j) & hashValues[i].first) ? 1: -1) * hashValues[i].second;
	}

	simVal=0;
	for(size_t j=0; j<64; ++j)
	{
		if(weights[j]>0.0)
			simVal|=(u64<<j);
	}

	return SIM_OK;
}

bool QSimHasher::isEqual(uint64_t lhs, uint64_t rhs, unsigned short n)
{
	unsigned short cnt = 0;
	lhs ^= rhs;
	while(lhs && cnt <= n)
	{
		lhs &= lhs - 1;
		cnt++;
	}
	if(cnt <= n)
	{
		return true;
	}
	return false;
}

void QSimHasher::toBinaryString(uint64_t req, std::string& res)
{
	res.resize(64);
	for(signed i = 63; i >= 0; i--)
	{
		req & 1 ? res[i] = '1' : res[i] = '0';
		req >>= 1;
	}
}

uint64_t QSimHasher::binaryStringToUint64(const std::string& bin)
{
	uint64_t res = 0;
	for(size_t i = 0; i < bin.size(); i++)
	{
		res <<= 1;
		if(bin[i] == '1')
		{
			res += 1;
		}
	}
	return res;
}

int32_t QSimHasher::str2listPOS(const std::string& strPOS, std::list< std::vector<std::string> >& listPOS)
{
	if(!strPOS.length() || strPOS.at(0)!='(' || strPOS.at(strPOS.length()-1)!=')')
		return SIM_ERR;

	std::vector<std::string> words=q_split(strPOS.substr(1, strPOS.length()-2), "),(");
	for(size_t i=0; i<words.size(); ++i)
	{
		std::vector<std::string> meanings=q_split_any(words.at(i), "> ");
		listPOS.push_back(meanings);
	}

	return SIM_OK;
}

std::string QSimHasher::listPOS2str(const std::list< std::vector<std::string> >& listPOS)
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

bool QSimHasher::Compare(const std::pair<std::string, double>& lhs, const std::pair<std::string, double>& rhs)
{
	return lhs.second>rhs.second;
}

Q_END_NAMESPACE
