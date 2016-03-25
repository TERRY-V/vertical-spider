#include "qtrietree.h"

Q_BEGIN_NAMESPACE

QTrieTree::QTrieTree() :
	root_(0),
	now_num_(0)
{}

QTrieTree::~QTrieTree()
{
	if(root_) destroy(root_);
}

int32_t QTrieTree::init()
{
	Q_NEW(root_, QTrieNode, TRIE_DEFAULT_UNICODE);
	if(root_==NULL) {
		Q_INFO("QTrieTree: root_ is null, bad allocate!");
		return TRIE_ERR;
	}

	now_num_=0;
	return TRIE_OK;
}

int32_t QTrieTree::insert(const char* pszKey, int32_t iKeyLen)
{
	if(pszKey==NULL||iKeyLen<=0)
		return TRIE_ERR;

	QTrieNode *prev=NULL, *p=NULL, *current=root_;
	bool bNewKey=false;

	for(char* pszKeyTemp=const_cast<char*>(pszKey); pszKeyTemp<const_cast<char*>(pszKey)+iKeyLen; \
			pszKeyTemp+=sizeof(uint16_t)) {
		// 没有任何孩子结点
		if(current->first==NULL) {
			current->first=new(std::nothrow) QTrieNode(*(uint16_t *)pszKeyTemp);
			Q_CHECK_PTR(current->first);
			current->uUsed++;
			current=current->first;
			bNewKey=true;
			continue;
		}
		// 判断当前结点的孩子中是否包含当前字结点
		for(p=current->first; p; prev=p, p=p->next) {
			if(p->uUCS==*(uint16_t *)pszKeyTemp) {
				current=p;
				break;
			}
		}
		// 未发现该孩子
		if(!p) {
			prev->next=new(std::nothrow) QTrieNode(*(uint16_t *)pszKeyTemp);
			Q_CHECK_PTR(prev->next);
			current->uUsed++;
			current=prev->next;
			bNewKey=true;
		}
	}

	if(!bNewKey)
		return 1;

	current->bWord=1;
	now_num_++;
	return TRIE_OK;
}

int32_t QTrieTree::setupTrieIndex()
{
	return setupTrieNodeIndex(root_);
}

int32_t QTrieTree::findByIndex(const char* pszKey, int32_t iKeyLen)
{
	if(pszKey==NULL||iKeyLen<=0)
		return TRIE_ERR;

	QTrieNode *p=NULL, *current=root_;

	for(char* pszKeyTemp=const_cast<char*>(pszKey); pszKeyTemp<const_cast<char*>(pszKey)+iKeyLen; \
			pszKeyTemp+=sizeof(uint16_t)) {
		for(p=current->first; p; p=p->next) {
			if(p->uUCS==*(uint16_t *)pszKeyTemp) {
				current=p;
				break;
			}
		}
		if(!p) break;
	}

	if(p&&p->bWord==1)
		return TRIE_OK;
	return TRIE_ERR;
}

int32_t QTrieTree::findByHash(const char* pszKey, int32_t iKeyLen)
{
	if(pszKey==NULL||iKeyLen<=0)
		return TRIE_ERR;

	// 以哈希的方式进行查找
	QTrieNode *p=NULL, *current=root_;
	int32_t iHashValue=0;

	for(char* pszKeyTemp=const_cast<char*>(pszKey); pszKeyTemp<const_cast<char*>(pszKey)+iKeyLen; \
			pszKeyTemp+=sizeof(uint16_t)) {
		iHashValue=trieHash(current, *(uint16_t *)pszKeyTemp);
		for(p=current->sons[iHashValue]; p; p=current->sons[(++iHashValue)%current->uSize]) {
			if(p->uUCS==*(uint16_t *)pszKeyTemp) {
				current=p;
				break;
			}
		}
		if(!p) break;
	}

	if(p&&p->bWord==1)
		return TRIE_OK;

	return TRIE_ERR;
}

std::vector<int32_t> QTrieTree::getPossibleLength(const char* pszSentence, int32_t iSentenceLen, int32_t offset)
{
	Q_ASSERT(pszSentence!=NULL, "QTrieTree: pszSentence is null!");
	Q_ASSERT(iSentenceLen>0, "QTrieTree: iSentenceLen = (%d)", iSentenceLen);
	Q_ASSERT(offset>=0&&offset<iSentenceLen, "QTrieTree: offset value error, offset = (%d)", offset);

	std::vector<int32_t> posVec;
	QTrieNode *p=NULL, *current=root_;
	int32_t iHashValue=0;

	for(char* pszSentenceTemp=const_cast<char*>(pszSentence)+offset; const_cast<char*>(pszSentenceTemp)<pszSentence+iSentenceLen; \
			pszSentenceTemp+=sizeof(uint16_t)) {
		iHashValue=trieHash(current, *(uint16_t *)pszSentenceTemp);
		for(p=current->sons[iHashValue]; p; p=current->sons[(++iHashValue)%current->uSize]) {
			if(p->uUCS==*(uint16_t *)pszSentenceTemp) {
				current=p;
				if(current->bWord==1)
					posVec.push_back(pszSentenceTemp-pszSentence);
				break;
			}
		}
		if(!p) break;
	}

	return posVec;
}

int32_t QTrieTree::hashFactor(int32_t x)
{
	if(x<TRIE_LEAST_HASH_SIZE)
		return TRIE_LEAST_HASH_SIZE;

	int32_t res=0, i=0;
	for(res=x/2*4+1; ; res+=2) {
		for(i=3; i<sqrt(res); i+=2)
			if(res%i==0) break;

		if(res%i!=0)
			break;
	}
	return res;
}

void QTrieTree::destroy(QTrieNode* current)
{
	if(current) {
		destroy(current->first);
		destroy(current->next);
		q_delete_array<QTrieNode*>(current->sons);
		q_delete<QTrieNode>(current);
	}
}

int32_t QTrieTree::setupTrieNodeIndex(QTrieNode* current)
{
	if(current==NULL)
		return TRIE_ERR;

	// 计算每个节点桶的大小
	current->uSize=hashFactor(current->uUsed);
	current->sons=q_new_array<QTrieNode*>(current->uSize);
	Q_CHECK_PTR(current->sons);
	memset((void*)current->sons, 0, sizeof(QTrieNode*)*current->uSize);

	// 为每个节点构造哈希索引
	for(QTrieNode* p=current->first; p; p=p->next) {
		int32_t iHashValue=trieHash(current, p->uUCS);
		QTrieNode* pos=NULL;
		while((pos=current->sons[iHashValue]))
			pos=current->sons[(++iHashValue)%current->uSize];
		current->sons[iHashValue%current->uSize]=p;
		setupTrieNodeIndex(p);
	}

	return TRIE_OK;
}

Q_END_NAMESPACE
