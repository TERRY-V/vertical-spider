/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qtrietree.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/05/22
**
*********************************************************************************************/
#ifndef __QTRIETREE_H_
#define __QTRIETREE_H_

#include "qglobal.h"

#define TRIE_OK              (0)
#define TRIE_ERR             (-1)

#define TRIE_LEAST_HASH_SIZE (3)
#define TRIE_DEFAULT_UNICODE (0xABCD)

Q_BEGIN_NAMESPACE

struct QTrieNode {
	uint16_t	uUCS;	/* unicode */
	uint16_t	uUsed;	/* sons */
	uint32_t	uSize;	/* bucket size */
	uint8_t		bWord;	/* be a word */

	QTrieNode*	first;	/* first child */
	QTrieNode*	next;	/* next child */
	QTrieNode**	sons;	/* hash sons array */

	QTrieNode(const uint16_t& ucs) :
		uUCS(ucs), 
		uUsed(0), 
		uSize(0),
		bWord(0),
		first(NULL),
		next(NULL),
		sons(NULL)
	{}
};

class QTrieTree {
	public:
		explicit QTrieTree();
		virtual ~QTrieTree();

		int32_t init();

		int32_t insert(const char* pszKey, int32_t iKeyLen);

		int32_t findByIndex(const char* pszKey, int32_t iKeyLen);
		int32_t findByHash(const char* pszKey, int32_t iKeyLen);

		int32_t getDataNum() const
		{return now_num_;}

		/* recursively build index for acceleration */
		int32_t setupTrieIndex();

		std::vector<int32_t> getPossibleLength(const char* pszSentence, int32_t iSentenceLen, int32_t offset);

	private:
		int32_t hashFactor(int32_t x);

		inline int32_t trieHash(QTrieNode* p, uint16_t value)
		{return static_cast<int32_t>(value)%(p->uSize);}

		void destroy(QTrieNode* current);

		int32_t setupTrieNodeIndex(QTrieNode* current);

	protected:
		QTrieNode*	root_;
		int32_t		now_num_;
};

Q_END_NAMESPACE

#endif // __QTRIETREE_H_
