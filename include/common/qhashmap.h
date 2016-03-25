/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qhashmap.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2013/11/08
**
*********************************************************************************************/

#ifndef __QHASHMAP_H_
#define __QHASHMAP_H_

#include "qglobal.h"
#include "qfunc.h"

Q_BEGIN_NAMESPACE

// 通用哈希函数类
template<typename T_KEY>
class DefaultHash {
	public:
		DefaultHash(uint32_t numBuckets=251)
		{
			Q_ASSERT(numBuckets>0, "QHashMap: numBuckets must be larger than 0!");
			mNumBuckets=numBuckets;
		}

		uint32_t hash(const T_KEY& key) const
		{
			T_KEY hashKey(key);
			uint32_t hashVal=murmurHash(reinterpret_cast<void*>(&hashKey), sizeof(T_KEY));
#if 0
			uint32_t hashVal(0);
			for(uint32_t i=0; i<sizeof(key); ++i)
				hashVal+=*((uint8_t*)&key+i);
#endif
			return (hashVal%mNumBuckets);
		}

		uint32_t numBuckets() const
		{return mNumBuckets;}

	protected:
		uint32_t mNumBuckets;
};

// 字符串哈希函数的特化
template<>
class DefaultHash<std::string> {
	public:
		DefaultHash(uint32_t numBuckets=251)
		{
			Q_ASSERT(numBuckets>0, "QHashMap: numBuckets must be larger than 0!");
			mNumBuckets=numBuckets;
		}

		uint32_t hash(const std::string& key) const
		{
			uint32_t hashVal=murmurHash(key.c_str(), key.size());
			return (hashVal%mNumBuckets);
		}

		uint32_t numBuckets() const
		{return mNumBuckets;}

	protected:
		uint32_t mNumBuckets;
};

// QHashMap类
template< typename T_KEY, typename T, typename Compare=std::equal_to<T_KEY>, typename Hash=DefaultHash<T_KEY> >
class QHashMap {
	public:
		typedef T_KEY key_type;
		typedef T mapped_type;
		typedef std::pair<T_KEY, T> value_type;

		explicit QHashMap(const Hash& hash=Hash(), const Compare& comp=Compare())
		{
			mSize=0;
			mComp=comp;
			mHash=hash;
			Q_ASSERT(mHash.numBuckets()>0, "QHashMap: size must be larger than 0!");
			mElems=new(std::nothrow) std::vector<ListType>(mHash.numBuckets());
			Q_ASSERT(mElems!=NULL, "QHashMap: mElems is null!");
		}

		virtual ~QHashMap()
		{
			delete mElems;
			mElems=NULL;
			mSize=0;
		}

		bool empty() const
		{return mSize==0;}

		uint32_t size() const
		{return mSize;}

		uint32_t bucket_size() const
		{return mHash.numBuckets();}

		uint32_t bucket(const key_type& x)
		{return mHash.hash(x);}

		bool find(const key_type& x, T& v)
		{
			uint32_t bucket;
			typename std::list< std::pair<T_KEY, T> >::iterator it=findElement(x, bucket);
			if(it==(*mElems)[bucket].end())
				return false;
			v=it->second;
			return true;
		}

		void insert(const value_type& x)
		{
			uint32_t bucket;
			typename std::list< std::pair<T_KEY, T> >::iterator it=findElement(x.first, bucket);
			if(it!=(*mElems)[bucket].end()) {
				return;
			} else {
				mSize++;
				(*mElems)[bucket].insert((*mElems)[bucket].end(), x);
			}
		}

		void erase(const key_type& x)
		{
			uint32_t bucket;
			typename std::list< std::pair<T_KEY, T> >::iterator it=findElement(x, bucket);
			if(it!=(*mElems)[bucket].end()) {
				(*mElems)[bucket].erase(it);
				mSize--;
			}
		}

	private:
		typename std::list< std::pair<T_KEY, T> >::iterator findElement(const key_type& x, uint32_t& bucket) const
		{
			bucket=mHash.hash(x);
			for(typename std::list< std::pair<T_KEY, T> >::iterator it=(*mElems)[bucket].begin(); \
					it!=(*mElems)[bucket].end(); ++it) {
				if(mComp(it->first, x)) {
					return it;
				}
			}
			return (*mElems)[bucket].end();
		}

	protected:
		typedef std::list<value_type> ListType;
		std::vector<ListType>* mElems;
		uint32_t mSize;
		Compare mComp;
		Hash mHash;
};

// QHashMap2类
template < typename T_KEY, typename T, typename Compare=std::equal_to<T_KEY>, typename Hash=DefaultHash<T_KEY> >
class QHashMap2 {
	public:
		typedef T_KEY key_type;
		typedef T mapped_type;
		typedef std::pair<T_KEY, T> value_type;

		QHashMap2(const Hash& hash=Hash(), const Compare& comp=Compare())
		{
			mSize=0;
			mComp=comp;
			mHash=hash;
			Q_ASSERT(mHash.numBuckets()>0, "QHashMap2: size must be larger than 0!");
			mEntries=new(std::nothrow) EntryMap*[mHash.numBuckets()];
			Q_ASSERT(mEntries!=NULL, "QHashMap2: bad alloc, mEntries is null!");
		}

		virtual ~QHashMap2()
		{clear();}

		void clear()
		{
			if(!mEntries) return;
			for(uint32_t i=0; i<mHash.numBuckets(); ++i)
				if(mEntries[i]) delete mEntries[i];
			delete [] mEntries;
			mEntries=0;
			mSize=0;
		}

		bool empty() const
		{return mSize==0;}

		uint32_t size() const
		{return mSize;}

		uint32_t bucket_size() const
		{return mHash.numBuckets();}

		uint32_t bucket(const key_type& key)
		{return mHash.hash(key);}

		bool insert(const value_type& x)
		{
			uint32_t bucket=this->bucket(x.first);
			if(!mEntries[bucket])
				mEntries[bucket]=new EntryMap();
			std::pair<typename EntryMap::iterator, bool> res(mEntries[bucket]->insert(x));
			if(!res.second)
				return false;
			mSize++;
			return true;
		}

		void update(const value_type& x)
		{
			uint32_t bucket=this->bucket(x.first);
			if(!mEntries[bucket])
				mEntries[bucket]=new EntryMap();
			std::pair<typename EntryMap::iterator, bool> res(mEntries[bucket]->insert(x));
			if(!res.second)
				res.first->second=x.second;
			else
				mSize++;
		}

		void erase(const key_type& x)
		{
			uint32_t bucket=this->bucket(x.first);
			if(mEntries[bucket])
				mSize-=mEntries[bucket]->erase(x);
		}

		bool find(const key_type& x, T& v)
		{
			uint32_t bucket=this->bucket(x);
			if(!mEntries[bucket])
				return false;
			typename EntryMap::iterator it=mEntries[bucket]->find(x);
			if(it==mEntries[bucket]->end())
				return false;
			v=it->second;
			return true;
		}

	protected:
		typedef std::map<T_KEY, T> EntryMap;
		EntryMap** mEntries;
		uint32_t mSize;
		Compare mComp;
		Hash mHash;
};

// 散列表各桶中同义词子表的链结点定义
template<typename T_KEY, typename T>
struct ChainNode {
	T_KEY mKey;
	T mValue;
	ChainNode<T_KEY, T> *link;

	ChainNode(ChainNode<T_KEY, T>* l=NULL): link(l) {}
	ChainNode(const T_KEY& key, const T& val, ChainNode<T_KEY, T>* l=NULL): mKey(key), mValue(val), link(l) {}
};

// 散列表类定义
template<typename T_KEY, typename T>
class QLinearHash {
	public:
		QLinearHash(int32_t size=101) : 
			mBuckets(size), 
			mNowSize(0)
		{
			Q_ASSERT(size>0, "QLinearHash: bucket size = (%d)", size);
			mDivisor=getDivisor(size);
			pHashTable=new(std::nothrow) ChainNode<T_KEY, T>*[mBuckets];
			Q_ASSERT(pHashTable!=NULL, "QLinearHash: bad alloc, pHashTable is null");
			for(int32_t i=0; i<mBuckets; ++i) pHashTable[i]=NULL;
			mTravIndex=0;
			pTravPos=NULL;
		}

		virtual ~QLinearHash()
		{
			clear();
			if(pHashTable!=NULL) {
				delete [] pHashTable;
				pHashTable=NULL;
			}
			mTravIndex=0;
			pTravPos=NULL;
		}

		void clear()
		{
			ChainNode<T_KEY, T>* current=NULL;
			for(int32_t i=0; i!=mBuckets; ++i) {
				current=pHashTable[i];
				while(current) {
					ChainNode<T_KEY, T>* next=current->link;
					delete current;
					mNowSize--;
					current=next;
				}
			}
		}

		bool empty() const
		{return mNowSize==0;}

		int32_t size() const
		{return mNowSize;}

		int32_t bucket_size() const
		{return mBuckets;}
		
		bool search(uint64_t hashKey, const T_KEY& key, T& value)
		{
			int32_t hashVal=hash(hashKey);
			ChainNode<T_KEY, T> *current=pHashTable[hashVal];
			while(current&&current->mKey!=key) current=current->link;
			if(current) {
				value=current->mValue;
				return true;
			}
			return false;
		}

		bool push(uint64_t hashKey, const T_KEY& key, const T& value)
		{
			int32_t hashVal=hash(hashKey);
			ChainNode<T_KEY, T> *current=pHashTable[hashVal];
			while(current&&current->mKey!=key) current=current->link;
			if(!current) {
				ChainNode<T_KEY, T>* newNode=new ChainNode<T_KEY, T>(key, value);
				if(newNode==NULL)
					return false;
				newNode->link=pHashTable[hashVal];
				pHashTable[hashVal]=newNode;
				mNowSize++;
				return true;
			}
			return false;
		}

		bool pop(uint64_t hashKey, const T_KEY& key)
		{
			int32_t hashVal=hash(hashKey);
			ChainNode<T_KEY, T> *prev=NULL, *current=pHashTable[hashVal];
			while(current&&current->mKey!=key) {
				prev=current;
				current=current->link;
			}
			if(current) {
				if(!prev) pHashTable[hashVal]=current->link;
				else prev->link=current->link;
				delete current;
				mNowSize--;
				return true;
			}
			return false;
		}

		int32_t prepareTraversal()
		{
			mTravIndex=0;
			pTravPos=pHashTable[mTravIndex];
			return 0;
		}

		int32_t traverse(T_KEY& key, T& value)
		{
			if(pTravPos==NULL) {
				for(mTravIndex++; mTravIndex<bucket_size(); ++mTravIndex) {
					pTravPos=pHashTable[mTravIndex];
					if(pTravPos==NULL) continue;
					else break;
				}
				if(mTravIndex==mBuckets)
					return -1;
			}
			key=pTravPos->mKey;
			value=pTravPos->mValue;
			pTravPos=pTravPos->link;
			return 0;
		}

	private:
		// @函数名: 取一个不大于m，但最接近或等于m的质数作为除数
		int32_t getDivisor(int32_t m)
		{
			int32_t temp=m;
			while(!isPrime(temp)&&temp>2) --temp;
			return (temp==2)?m:temp;
		}

		// @函数名: 判断一个数是否为质数
		bool isPrime(int32_t num)
		{
			if(num<=1) return false;
			for(int32_t i=2; i*i<=num; ++i) if(num%i==0) return false;
			return true;
		}

		int32_t hash(uint64_t hashKey)
		{return static_cast<int32_t>(hashKey%mDivisor);}

	protected:
		int32_t mDivisor;
		int32_t mBuckets;
		int32_t mNowSize;
		ChainNode<T_KEY, T>** pHashTable;
		int32_t mTravIndex;
		ChainNode<T_KEY, T>* pTravPos;
};

Q_END_NAMESPACE

#endif // __QHASHMAP_H_
