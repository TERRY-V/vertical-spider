#include "qallocator.h"

Q_BEGIN_NAMESPACE

QAllocator::~QAllocator()
{
	if(pBlockLibrary!=NULL) {
		for(int64_t i=0; i<mMaxBlockNum; ++i)
			q_delete_array<char>(pBlockLibrary[i]);
		q_delete_array<char*>(pBlockLibrary);
	}
}

char* QAllocator::alloc(int64_t allocLen)
{
	// 创建内存块索引
	if(pBlockLibrary==NULL) {
		pBlockLibrary=q_new_array<char*>(mMaxBlockNum);
		if(pBlockLibrary==NULL)
			return NULL;
		memset(pBlockLibrary, 0, mMaxBlockNum*sizeof(void*));
	}

	// 需要开辟新的内存块
	if(allocLen>mMaxBlockLen-mNowBlockLen) {
		// 需要开辟的内存太长
		if(allocLen>mMaxBlockLen)
			return NULL;

		// 分配的内存块达到最大值
		if(++mNowBlockNum>=mMaxBlockNum) {
			--mNowBlockNum;
			return NULL;
		}

		// 开辟新的块内存
		if(pBlockLibrary[mNowBlockNum]==NULL) {
			pBlockLibrary[mNowBlockNum]=q_new_array<char>(mMaxBlockLen);
			if(pBlockLibrary[mNowBlockNum]==NULL) {
				mNowBlockNum--;
				return NULL;
			}
		}

		// 新内存块的长度为0
		mNowBlockLen=0;
	}

	// 原内存块剩余内存可继续使用
	char* pTemp=&pBlockLibrary[mNowBlockNum][mNowBlockLen];
	mNowBlockLen+=allocLen;
	mTotalLen+=allocLen;

	return pTemp;
}

int64_t QAllocator::getAllocLength() const
{
	return mTotalLen;
}

int64_t QAllocator::getBufferLength() const
{
	return mMaxBlockLen*mNowBlockNum+mNowBlockLen;
}

void QAllocator::resetAllocator()
{
	mNowBlockNum=-1;
	mNowBlockLen=mMaxBlockLen;
	mTotalLen=0;
}

int64_t QAllocator::freeAllocator()
{
	this->~QAllocator();
	mNowBlockNum=-1;
	mNowBlockLen=mMaxBlockLen;
	pBlockLibrary=NULL;
	mTotalLen=0;
	return 0;
}

QPoolAllocator::~QPoolAllocator()
{
	if(pBlockLibrary!=NULL) {
		for(int64_t i=0; i<mMaxBlockNum; ++i)
			q_delete_array<char>(pBlockLibrary[i]);
		q_delete_array<char*>(pBlockLibrary);
	}
	pRecycleLibrary=NULL;
}

int32_t QPoolAllocator::init(int32_t lChunkSize, int64_t lMaxBlockNum, int64_t lChunkNumPerBlock)
{
	if(lChunkSize<=0||lMaxBlockNum<=0||lChunkNumPerBlock<=0)
		return -1;

	mChunkSize=lChunkSize+sizeof(void*);

	mMaxBlockNum=lMaxBlockNum;
	mNowBlockNum=-1;

	mMaxChunkNum=lChunkNumPerBlock;
	mNowChunkNum=-1;

	mMaxBlockLen=lChunkNumPerBlock*mChunkSize;
	mNowBlockLen=0;

	pBlockLibrary=NULL;
	pRecycleLibrary=NULL;

	mRecycleNum=0;
	mAllocNum=0;

	pBlockLibrary=q_new_array<char*>(mMaxBlockNum);
	if(pBlockLibrary==NULL)
		return -2;
	memset(pBlockLibrary, 0, mMaxBlockNum*sizeof(char*));

	return 0;
}

char* QPoolAllocator::alloc()
{
	mMutex.lock();
	// 从回收区查看是否有内存可复用
	if(pRecycleLibrary!=NULL) {
		char* p=pRecycleLibrary+sizeof(void*);
		pRecycleLibrary=*(char**)pRecycleLibrary;
		mRecycleNum--;
		mMutex.unlock();
		return p;
	}

	// 是否需要新分配内存
	if(mNowChunkNum==-1 || mNowChunkNum==mMaxChunkNum) {
		// 当前block数已经达到峰值
		if(mNowBlockNum+1>=mMaxBlockNum) {
			mMutex.unlock();
			return NULL;
		}

		// 分配新内存
		if(pBlockLibrary[++mNowBlockNum]==NULL) {
			pBlockLibrary[mNowBlockNum]=q_new_array<char>(mMaxBlockLen);
			if(pBlockLibrary[mNowBlockNum]==NULL) {
				mNowBlockNum--;
				mMutex.unlock();
				return NULL;
			}
		}

		mNowBlockLen=0;
		mNowChunkNum=0;
	}

	// 当前内存已足够
	char* p=&pBlockLibrary[mNowBlockNum][mNowBlockLen]+sizeof(void*);
	mNowBlockLen+=mChunkSize;

	mNowChunkNum++;
	mAllocNum++;

	mMutex.unlock();
	return p;
}

void QPoolAllocator::free(char* buffer)
{
	mMutex.lock();
	buffer-=sizeof(void*);
	*(char**)buffer=pRecycleLibrary;
	pRecycleLibrary=buffer;
	mRecycleNum++;
	mMutex.unlock();
}

void QPoolAllocator::resetAllocator()
{
	mMutex.lock();
	pRecycleLibrary=NULL;
	mNowChunkNum=mMaxChunkNum;
	mNowBlockLen=0;
	mNowBlockNum=-1;
	mRecycleNum=0;
	mAllocNum=0;
	mMutex.unlock();
}

void QPoolAllocator::freeAllocator()
{
	mMutex.lock();
	if(pBlockLibrary!=NULL) {
		for(int64_t i=0; i<mMaxBlockNum; ++i)
			q_delete_array<char>(pBlockLibrary[i]);
		q_delete_array<char*>(pBlockLibrary);
	}
	pRecycleLibrary=NULL;
	mNowChunkNum=-1;
	mNowBlockLen=0;
	mNowBlockNum=-1;
	mRecycleNum=0;
	mAllocNum=0;
	mMutex.unlock();
}

void QPoolAllocator::trace(const char* name)
{
	Q_INFO("QAllocatorRecyle(%s): mAllocNum=(%ld), mRecyleNum=(%ld), mMaxBlockNum=(%ld), mNowBlockNum=(%ld)", \
			name, mAllocNum, mRecycleNum, mMaxBlockNum, mNowBlockNum+1);
}

int64_t QPoolAllocator::getChunkSize() const
{
	return mChunkSize;
}

Q_END_NAMESPACE
