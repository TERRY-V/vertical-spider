/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qwordtokenizer.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2012/11/08
**
*********************************************************************************************/

#ifndef __QWORDTOKENIZER_H_
#define __QWORDTOKENIZER_H_

#include "qglobal.h"
#include "qadjgraph.h"
#include "qhashsearch.h"
#include "qmd5.h"
#include "qtrietree.h"

#define TOKEN_OK                (0)
#define TOKEN_ERR               (-1)

#define TOKEN_MAGIC_NUMBER      (0x123456789)
#define TOKEN_LIB_VERSION       (*(uint64_t*)"14.08.08")
#define TOKEN_AUTHOR            (0x67975c0f725bfeff)
#define TOKEN_END_MARK          (*(uint64_t*)"@#@#@#@#")

#define TOKEN_DEFAULT_BUFFER_SIZE (1<<10)
#define TOKEN_DEFAULT_BUCKET_SIZE (1<<20)

#define TOKEN_DEFAULT_LIB_FILE  ("__common.lib")
#define TOKEN_DEFAULT_POS_FILE  ("__common.pos")

#define TOKEN_DEFAULT_UBYTES    (2)
#define TOKEN_DEFAULT_WEIGHT    (1.0)

#define TOKEN_DEFAULT_TOKEN_SEP (0x002F)
#define TOKEN_DEFAULT_POS_SEP   (0x002c)

Q_BEGIN_NAMESPACE

enum CodeType {
	CODE_TYPE_UNKNOWN,	// unknown type
	CODE_TYPE_ASCII,	// ASCII
	CODE_TYPE_GBK,		// GB2312, GBK, GB10380
	CODE_TYPE_UTF8,		// UTF-8
	CODE_TYPE_BIG5		// BIG5
};

#pragma pack(1)

struct dictInfo {
	uint64_t magicNumber;
	uint64_t libVersion;
	uint64_t author;
	uint32_t totalNum;
};

struct recordHeader {
	uint64_t id;
	uint32_t length;
};

#pragma pack()

struct dataInfo {
	char* data;
	int32_t length;

	dataInfo(char* dataPtr=NULL, int32_t len=0) :
		data(dataPtr),
		length(len)
	{}
};

class QWordTokenizer {
	public:
		QWordTokenizer();

		virtual ~QWordTokenizer();

		int32_t init();

		int32_t word_tokenize(const char* pszText, int iLength, char* pszResult, int iResultSize, CodeType codeType=CODE_TYPE_UTF8, bool bEnablePOS=true);

	private:
		int32_t loadDictionary(const char* fileName);

		int32_t loadPartOfSpeech(const char* fileName);

		int32_t word_tokenize(bool bEnablePOS, const char* pszText, int32_t iTextLen, char* pszResult, int32_t iResultSize);

		int32_t generateWordGraph(const char* pszText, int32_t iTextLen, QAdjGraph<dataInfo>* adjGraph);

		int32_t shortestPath2String(const char* pszText, int32_t iTextLen, std::vector<int32_t>& pathVec, char* pszResult, int32_t iResultSize);

		int32_t shortestPath2PosString(const char* pszText, int32_t iTextLen, std::vector<int32_t>& pathVec, char* pszResult, int32_t iResultSize);

		bool isNumber(const char* pszKey, int32_t iKeyLen) const;

		bool isLetterOrNumber(const char* pszKey, int32_t iKeyLen) const;

	protected:
		QTrieTree*		trieTree;
		QHashSearch<uint64_t>*	hashSearch;
};

Q_END_NAMESPACE

#endif // __QWORDTOKENIZER_H_
