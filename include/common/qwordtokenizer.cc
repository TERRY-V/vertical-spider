#include "qtextcodec.h"
#include "qwordtokenizer.h"

Q_BEGIN_NAMESPACE

QWordTokenizer::QWordTokenizer() :
	trieTree(0),
	hashSearch(0)
{}

QWordTokenizer::~QWordTokenizer()
{
	q_delete< QTrieTree >(trieTree);
	q_delete< QHashSearch<uint64_t> >(hashSearch);
}

int32_t QWordTokenizer::init()
{
	int32_t ret=0;

	Q_INFO("Start initing the tokenizer...");

	trieTree=q_new<QTrieTree>();
	if(trieTree==NULL) {
		Q_FATAL("Trie tree creating failed...");
		return -1;
	}

	ret=trieTree->init();
	if(ret<0) {
		Q_FATAL("Trie tree initing failed, ret = (%d)!", ret);
		return -2;
	}

	hashSearch=q_new< QHashSearch<uint64_t> >();
	if(hashSearch==NULL) {
		Q_FATAL("Hash search creating failed...");
		return -4;
	}

	ret=hashSearch->init(TOKEN_DEFAULT_BUCKET_SIZE, -1);
	if(ret<0) {
		Q_FATAL("Hash search initing failed, ret = (%d)!", ret);
		return -5;
	}

	Q_INFO("Loading the dict lib file...");

	ret=loadDictionary(TOKEN_DEFAULT_LIB_FILE);
	if(ret<0) {
		Q_FATAL("Dict lib file loading failed, ret = (%d)!", ret);
		return -3;
	}

	Q_INFO("Loading the dict pos file...");

	ret=loadPartOfSpeech(TOKEN_DEFAULT_POS_FILE);
	if(ret<0) {
		Q_FATAL("Dict pos file loading failed, ret = (%d)!", ret);
		return -6;
	}

	return ret;
}

int32_t QWordTokenizer::word_tokenize(const char* pszText, int32_t iLength, char* pszResult, int32_t iResultSize, CodeType codeType, bool bEnablePOS)
{
	Q_CHECK_PTR(pszText);
	Q_CHECK_PTR(pszResult);
	Q_ASSERT(iLength>0, "iLength must be larger than 0!");
	Q_ASSERT(iResultSize>0, "iResultSize must be larger than 0!");

	int32_t iAllocSize=iLength<<5;
	char* pszAlloc=NULL;
	char* pszToken=NULL;
	int32_t iTokenLen=0;

	int32_t ret=0;

	pszAlloc=q_new_array<char>(iAllocSize);
	if(pszAlloc==NULL)
		return -1;

	try {
		if(codeType==CODE_TYPE_GBK) {
			ret=QTextCodec::gbk2unicode(const_cast<char*>(pszText), iLength, pszAlloc, iAllocSize);
			if(ret<0) throw -2;

			pszToken=pszAlloc+ret;
			iTokenLen=iAllocSize-ret;

			ret=word_tokenize(bEnablePOS, pszAlloc+2, ret-2, pszToken, iTokenLen);
			if(ret<0) throw -3;

			ret=QTextCodec::unicode2gbk(pszToken, ret, pszResult, iResultSize);
			if(ret<0) throw -4;
		} else if(codeType==CODE_TYPE_UTF8) {
			ret=QTextCodec::utf82unicode(const_cast<char*>(pszText), iLength, pszAlloc, iAllocSize);
			if(ret<0) throw -5;

			pszToken=pszAlloc+ret;
			iTokenLen=iAllocSize-ret;

			ret=word_tokenize(bEnablePOS, pszAlloc+2, ret-2, pszToken, iTokenLen);
			if(ret<0) throw -6;

			ret=QTextCodec::unicode2utf8(pszToken, ret, pszResult, iResultSize);
			if(ret<0) throw -7;
		} else {
			throw -8;
		}
	} catch(int32_t err) {
		ret=err;
	}

	q_delete_array<char>(pszAlloc);
	return ret;
}

int32_t QWordTokenizer::word_tokenize(bool bEnablePOS, const char* pszText, int32_t iTextLen, char* pszResult, int32_t iResultSize)
{
	Q_CHECK_PTR(pszText);
	Q_CHECK_PTR(pszResult);
	Q_ASSERT(iTextLen>0, "iTextLen must be larger than 0!");
	Q_ASSERT(iResultSize>0, "iResultSize must be larger than 0!");

	QAdjGraph<dataInfo>* adjGraph=NULL;
	std::vector<int32_t> shortestPath;
	int32_t ret=0;

	Q_NEW(adjGraph, QAdjGraph<dataInfo>, (iTextLen+1));
	if(adjGraph==NULL) {
		Q_FATAL("Adjacent graph creating failed...");
		return -2;
	}

	try {
		// 创建有向词图
		ret=generateWordGraph(pszText, iTextLen, adjGraph);
		if(ret<0) {
			Q_FATAL("Generating word graph failed, ret = (%d)!", ret);
			throw -3;
		}

#if defined (__VERBOSE_MODE)
		adjGraph->printGraph();
#endif

		// 计算最短路径
		shortestPath=adjGraph->Dijkstra(0, iTextLen);
		if(shortestPath.empty()) {
			Q_FATAL("Computing shortest path failed!");
			throw -4;
		}

		// 输出分词结果
		if(bEnablePOS) {
			ret=shortestPath2PosString(pszText, iTextLen, shortestPath, pszResult, iResultSize);
			if(ret<0) {
				Q_FATAL("Token POS string translating failed, ret = (%d)!", ret);
				throw ret;
			}
		} else {
			ret=shortestPath2String(pszText, iTextLen, shortestPath, pszResult, iResultSize);
			if(ret<0) {
				Q_FATAL("Token string translating failed, ret = (%d)!", ret);
				throw ret;
			}
		}
	} catch(const int32_t err) {
		ret=err;
	}

	q_delete< QAdjGraph<dataInfo> >(adjGraph);
	return ret;
}

int32_t QWordTokenizer::generateWordGraph(const char* pszText, int32_t iTextLen, QAdjGraph<dataInfo>* adjGraph)
{
	Q_CHECK_PTR(pszText);
	Q_CHECK_PTR(adjGraph);
	Q_ASSERT(iTextLen>0, "iTextLen must be larger than 0!");

	for(int32_t i=0; i!=iTextLen; i+=TOKEN_DEFAULT_UBYTES)
	{
		// 创建相邻字之间的有向边
		Token<dataInfo> token(dataInfo(const_cast<char*>(pszText)+i, TOKEN_DEFAULT_UBYTES), TOKEN_DEFAULT_WEIGHT);
		adjGraph->insertEdge(i, i+TOKEN_DEFAULT_UBYTES, token);

		// 由词创建有向边
		std::vector<int32_t> posVec=trieTree->getPossibleLength(pszText, iTextLen, i);
		for(int32_t j=0; j<(int32_t)posVec.size(); ++j) {
			token.data=dataInfo(const_cast<char*>(pszText)+i, posVec[j]-i);
			token.cost=TOKEN_DEFAULT_WEIGHT;
			adjGraph->insertEdge(i, posVec[j]+TOKEN_DEFAULT_UBYTES, token);
		}

		// 拉丁字母和数字创建有向边
		uint16_t ucs=*(uint16_t*)(const_cast<char*>(pszText)+i);
		if((ucs>=0x0030&&ucs<=0x0039)||(ucs>=0x0041&&ucs<=0x005a)||(ucs>=0x0061&&ucs<=0x007a))
		{
			int32_t pos=i+TOKEN_DEFAULT_UBYTES;
			while(pos<iTextLen) {
				// Next character is still an alphabet or number
				// '_', '-', '.', '/' and '~' are allowed to be among the strings
				uint16_t ucs_pos=*(uint16_t*)(const_cast<char*>(pszText)+pos);
				if((ucs_pos>=0x0030&&ucs_pos<=0x0039) \
						||(ucs_pos>=0x0041&&ucs_pos<=0x005a) \
						||(ucs_pos>=0x0061&&ucs_pos<=0x007a) \
						||ucs_pos==0x005f||ucs_pos==0x002d||ucs_pos==0x002e \
						||ucs_pos==0x002f||ucs_pos==0x007e||ucs_pos==0x0040 \
						||ucs_pos==0xff0d||ucs_pos==0x33A1 \
						||(ucs_pos==0x002c&&*(uint16_t*)(pszText+pos-TOKEN_DEFAULT_UBYTES)>=0x0030&&*(uint16_t*)(pszText+pos-TOKEN_DEFAULT_UBYTES)<=0x0039)) {
					pos+=TOKEN_DEFAULT_UBYTES;
				} else {
					break;
				}
			}

			token.data=dataInfo(const_cast<char *>(pszText)+i, pos-i);
			token.cost=1.0;
			adjGraph->insertEdge(i, pos, token);
		}

		// Web中的html标签
		if(ucs==0x003c)
		{
			int32_t pos=i+TOKEN_DEFAULT_UBYTES;
			while(pos<iTextLen) {
				uint16_t ucs_pos=*(uint16_t*)(const_cast<char*>(pszText)+pos);
				if((ucs_pos>=0x0041&&ucs_pos<=0x005a)||(ucs_pos>=0x0061&&ucs_pos<=0x007a)||ucs_pos==0x002f) {
					while(pos<iTextLen) {
						if(*(uint16_t*)(pszText+pos)==0x003e) {
							pos+=TOKEN_DEFAULT_UBYTES;
							break;
						}
						pos+=TOKEN_DEFAULT_UBYTES;
					}
				} else {
					break;
				}
			}

			token.data=dataInfo(const_cast<char *>(pszText)+i, pos-i);
			token.cost=1.0;
			adjGraph->insertEdge(i, pos, token);
		}
	}

	return 0;
}

int32_t QWordTokenizer::shortestPath2String(const char* pszText, int32_t iTextLen, std::vector<int32_t>& pathVec, char* pszResult, int32_t iResultSize)
{
	Q_CHECK_PTR(pszText);
	Q_CHECK_PTR(pszResult);
	Q_ASSERT(iTextLen>0, "iTextLen must be larger than 0!");
	Q_ASSERT(iResultSize>0, "iResultSize must be larger than 0!");

	char* pszKey=NULL;
	int32_t iKeyLen=0;

	char* pszTemp=pszResult;
	char* pszEnd=pszTemp+iResultSize;

	int32_t pathVecSize=pathVec.size();
	int32_t lastPos=0;

	for(int32_t i=1; i<pathVecSize; ++i)
	{
		pszKey=(char*)pszText+lastPos;
		iKeyLen=pathVec[i]-lastPos;

		if(pszTemp+iKeyLen+TOKEN_DEFAULT_UBYTES>pszEnd)
			return -11;

		memcpy(pszTemp, pszKey, iKeyLen);
		pszTemp+=iKeyLen;

		if(i!=pathVecSize-1) {
			if(pszTemp+TOKEN_DEFAULT_UBYTES>pszEnd)
				return -12;

			*(uint16_t*)pszTemp=TOKEN_DEFAULT_TOKEN_SEP;
			pszTemp+=TOKEN_DEFAULT_UBYTES;
		}

		lastPos=pathVec[i];
	}

	return (ptrdiff_t)(pszTemp-pszResult);
}

int32_t QWordTokenizer::shortestPath2PosString(const char* pszText, int32_t iTextLen, std::vector<int32_t>& pathVec, char* pszResult, int32_t iResultSize)
{
	Q_CHECK_PTR(pszText);
	Q_CHECK_PTR(pszResult);
	Q_ASSERT(iTextLen>0, "iTextLen must be larger than 0!");
	Q_ASSERT(iResultSize>0, "iResultSize must be larger than 0!");

	char* pszKey=NULL;
	int32_t iKeyLen=0;
	char* pszValue=NULL;
	int32_t iValueLen=0;

	char* pszTemp=pszResult;
	char* pszEnd=pszTemp+iResultSize;

	int32_t pathVecSize=pathVec.size();
	int32_t lastPos=0;

	char pszBuffer[1<<5]={0};
	uint64_t tokenKey=0;

	for(int32_t i=1; i<pathVecSize; ++i)
	{
		pszKey=(char*)pszText+lastPos;
		iKeyLen=pathVec[i]-lastPos;

		QMD5 md5;
		tokenKey=md5.MD5Bits64(reinterpret_cast<unsigned char*>(pszKey), iKeyLen);

		pszValue=pszBuffer;
		iValueLen=0;

		if(hashSearch->searchKey_VL(tokenKey, (void**)(&pszValue), &iValueLen)==0) {
			// 查询成功修改pszValue的指针地址
		}

		// 处理单个字符的词
		else if(iKeyLen==TOKEN_DEFAULT_UBYTES) {
			uint16_t ucs=*(uint16_t*)pszKey;
			if((ucs>=0x0020&&ucs<=0x002f)||(ucs>=0x003a&&ucs<=0x0040) \
					||(ucs>=0x005b&&ucs<=0x0060)||(ucs>=0x007b&&ucs<=0x007e) \
					||(ucs>=0x2000&&ucs<=0x206f)||(ucs>=0x3000&&ucs<=0x303f) \
					||(ucs>=0xff00&&ucs<=0xffef)) {

				// 通用标点语义符号O标识
				*(uint16_t*)pszValue=0x004f;
				iValueLen+=TOKEN_DEFAULT_UBYTES;

				// 中英文冒号和空格O2
				if(ucs==0x003a||ucs==0xff1a||ucs==0x0020||ucs==0x3000) {
					*(uint16_t*)(pszValue+iValueLen)=0x0020;
					*(uint32_t*)(pszValue+iValueLen+TOKEN_DEFAULT_UBYTES)=0x0032004f;
					iValueLen+=3*TOKEN_DEFAULT_UBYTES;
				}

				// 中文顿号O3
				if(ucs==0x3001) {
					*(uint16_t*)(pszValue+iValueLen)=0x0020;
					*(uint32_t*)(pszValue+iValueLen+TOKEN_DEFAULT_UBYTES)=0x0033004f;
					iValueLen+=3*TOKEN_DEFAULT_UBYTES;
				}

				// 中英文括号O5
				if(ucs==0x0028||ucs==0x0029||ucs==0xff08||ucs==0xff09) {
					*(uint16_t*)(pszValue+iValueLen)=0x0020;
					*(uint32_t*)(pszValue+iValueLen+TOKEN_DEFAULT_UBYTES)=0x0035004f;
					iValueLen+=3*TOKEN_DEFAULT_UBYTES;
				}
			}

			// 判断是否为数字
			else if(isNumber(pszKey, iKeyLen)) {
				*(uint16_t*)pszValue=0x0053;
				iValueLen+=TOKEN_DEFAULT_UBYTES;
			}

			// 识别失败标识
			else {
				*(uint64_t*)pszValue=0x004c004c0055004e;
				iValueLen+=4*TOKEN_DEFAULT_UBYTES;
			}
		}

		// 判断是否为数字
		else if(isNumber(pszKey, iKeyLen)) {
			*(uint16_t *)pszValue=0x0053;
			iValueLen+=TOKEN_DEFAULT_UBYTES;
		}

		// 判断是字母与数字
		else if(isLetterOrNumber(pszKey, iKeyLen)) {
			*(uint64_t *)pszValue=0x004e004e004c004c;
			iValueLen+=4*TOKEN_DEFAULT_UBYTES;
		}

		// 识别失败的标识
		else {
			*(uint64_t *)pszValue=0x004c004c0055004e;
			iValueLen+=4*TOKEN_DEFAULT_UBYTES;
		}

		if(pszTemp+iKeyLen+iValueLen+3*TOKEN_DEFAULT_UBYTES>pszEnd)
			return -15;

		*(uint16_t*)pszTemp=0x0028;
		memcpy(pszTemp+TOKEN_DEFAULT_UBYTES, pszKey, iKeyLen);
		*(uint16_t*)(pszTemp+TOKEN_DEFAULT_UBYTES+iKeyLen)=0x003e;
		memcpy(pszTemp+iKeyLen+2*TOKEN_DEFAULT_UBYTES, pszValue, iValueLen);
		*(uint16_t *)(pszTemp+iKeyLen+2*TOKEN_DEFAULT_UBYTES+iValueLen)=0x0029;

		pszTemp+=iKeyLen+iValueLen+3*TOKEN_DEFAULT_UBYTES;

		if(i!=pathVecSize-1) {
			if(pszTemp+TOKEN_DEFAULT_UBYTES>pszEnd)
				return -16;

			*(uint16_t*)pszTemp=TOKEN_DEFAULT_POS_SEP;
			pszTemp+=TOKEN_DEFAULT_UBYTES;
		}

		lastPos=pathVec[i];
	}

	return (ptrdiff_t)(pszTemp-pszResult);
}

int32_t QWordTokenizer::loadDictionary(const char* fileName)
{
	dictInfo dict_info;
	recordHeader record_header;
	uint64_t end_mark=0;

	char buffer[TOKEN_DEFAULT_BUFFER_SIZE]={0};
	int32_t ret=0;

	FILE* fpR=::fopen(fileName, "rb");
	if(fpR==NULL)
		return -1;

	try {
		if(fread(&dict_info, sizeof(dict_info), 1, fpR)!=1)
			throw -2;

		if(dict_info.magicNumber!=TOKEN_MAGIC_NUMBER)
			throw -3;

		if(dict_info.libVersion<TOKEN_LIB_VERSION)
			throw -4;

		if(dict_info.author!=TOKEN_AUTHOR)
			throw -5;

		for(int32_t i=1; i<=static_cast<int32_t>(dict_info.totalNum); ++i)
		{
			if(fread(&record_header, sizeof(record_header), 1, fpR)!=1)
				throw -6;

			if(record_header.length<0||record_header.length>TOKEN_DEFAULT_BUFFER_SIZE)
				throw -7;

			if(fread(buffer, record_header.length, 1, fpR)!=1)
				throw -8;

			if(trieTree->insert(buffer, record_header.length)<0)
				throw -9;

			if((fread(&end_mark, sizeof(end_mark), 1, fpR)!=1)&&(end_mark!=TOKEN_END_MARK))
				throw -10;
		}

		if(trieTree->setupTrieIndex()<0)
			throw -11;

		Q_INFO("Loading (%d) words in total...", dict_info.totalNum);
	} catch(const int32_t err) {
		ret=err;
	}

	::fclose(fpR);
	return ret;
}

int32_t QWordTokenizer::loadPartOfSpeech(const char* fileName)
{
	dictInfo dict_info;
	recordHeader record_header;
	uint64_t end_mark=0;

	char buffer[TOKEN_DEFAULT_BUFFER_SIZE]={0};
	int32_t ret=0;

	void* p=NULL;
	int32_t len=0;

	FILE* fpR=::fopen(fileName, "rb");
	if(fpR==NULL)
		return -1;

	try {
		if(fread(&dict_info, sizeof(dict_info), 1, fpR)!=1)
			throw -2;

		if(dict_info.magicNumber!=TOKEN_MAGIC_NUMBER)
			throw -3;

		if(dict_info.libVersion<TOKEN_LIB_VERSION)
			throw -4;

		if(dict_info.author!=TOKEN_AUTHOR)
			throw -5;

		for(int32_t i=1; i<=static_cast<int32_t>(dict_info.totalNum); ++i)
		{
			if(fread(&record_header, sizeof(record_header), 1, fpR)!=1)
				throw -6;

			if(record_header.length<0||record_header.length>TOKEN_DEFAULT_BUFFER_SIZE)
				throw -7;

			if(fread(buffer, record_header.length, 1, fpR)!=1)
				throw -8;

			if(hashSearch->addKey_VL(record_header.id, buffer, record_header.length, &p, &len)<0)
				throw -9;

			if((fread(&end_mark, sizeof(end_mark), 1, fpR)!=1)&&(end_mark!=TOKEN_END_MARK))
				throw -10;
		}

		Q_INFO("Loading POS successful...");
	} catch(const int32_t err) {
		ret=err;
	}

	fclose(fpR);
	return ret;
}

bool QWordTokenizer::isNumber(const char* pszKey, int32_t iKeyLen) const
{
	if(pszKey==NULL||iKeyLen<=0)
		return false;

	uint16_t ucs=0;
	for(int32_t pos(0); pos!=iKeyLen; pos+=TOKEN_DEFAULT_UBYTES) {
		ucs=*(uint16_t *)(pszKey+pos);
		if(!((ucs>=0x0030&&ucs<=0x0039)||ucs==0x002e||ucs==0x002c))
			return false;
	}
	return true;
}

bool QWordTokenizer::isLetterOrNumber(const char* pszKey, int32_t iKeyLen) const
{
	if(pszKey==NULL||iKeyLen<=0)
		return false;

	uint16_t ucs=0;
	for(int32_t pos(0); pos!=iKeyLen; pos+=TOKEN_DEFAULT_UBYTES) {
		ucs=*(uint16_t*)(pszKey+pos);
		if(!((ucs>=0x0030&&ucs<=0x0039)||(ucs>=0x0041&&ucs<=0x005a)||(ucs>=0x0061&&ucs<=0x007a) \
					||ucs==0x005f||ucs==0x002d||ucs==0x0028||ucs==0x0029 \
					||ucs==0x33A1||ucs==0xff0d||ucs==0x007e||ucs==0x002f))
			return false;
	}
	return true;
}

Q_END_NAMESPACE
