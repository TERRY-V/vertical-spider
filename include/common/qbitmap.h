/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qbitmap.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2013/10/31
**
*********************************************************************************************/

#ifndef __QBITMAP_H_
#define __QBITMAP_H_

#include "qglobal.h"

Q_BEGIN_NAMESPACE

class QBITS16 {
	public:
		inline QBITS16(uint16_t init=0) :
			val(init)
		{}

		virtual ~QBITS16()
		{}

		inline void turn_on_bit(uint8_t bit_num)
		{val=val|01<<bit_num;}

		inline void turn_off_bit(uint8_t bit_num)
		{val=val&~(01<<bit_num);}

		inline void set_bit(uint8_t bit_num, bool value)
		{
			if(value) {
				val=val|01<<bit_num;
			} else {
				val=val&~(01<<bit_num);
			}
		}

		inline bool bit(uint8_t bit_num) const
		{return (val>>bit_num)&01;}

	private:
		uint16_t val;
};

//////////////////////////////////////////////////////////////////////////////////////////////

class QBitMap {
	public:
		inline QBitMap(const bool set_flag=false)
		{
			if(set_flag) {
				item_count_=slot_count_=INVALID_INDEX;
			} else {
				item_count_=slot_count_=0;
			}
			data_=NULL;
			set_count_=0;
			mount_=false;
		}

		inline QBitMap(const uint32_t item_count, const bool set_flag)
		{
			Q_ASSERT(item_count!=0, "QBitMap: item_count must be larger than 0!");
			slot_count_=(item_count+SLOT_SIZE-1)>>3;
			data_=q_new_array<char>(sizeof(char)*slot_count_);
			Q_ASSERT(data_!=NULL, "QBitMap: alloc error, data_ is null!");
			item_count_=item_count;
			memset(data_, set_flag?0xFF:0x00, slot_count_*sizeof(char));
			set_count_=set_flag?item_count:0;
			mount_=false;
		}

		inline QBitMap(const QBitMap& rhs)
		{
			item_count_=rhs.item_count_;
			set_count_=rhs.set_count_;
			slot_count_=rhs.slot_count_;
			if(NULL!=rhs.get_data()) {
				data_=q_new_array<char>(sizeof(char)*slot_count_);
				Q_ASSERT(data_!=NULL, "QBitMap: alloc error, data_ is null!");
				memcpy(data_, rhs.data_, slot_count_*sizeof(char));
			} else {
				data_=NULL;
			}
			mount_=false;
		}

		virtual ~QBitMap()
		{
			if(!mount_&&NULL!=data_)
				q_delete_array<char>(data_);
		}

		void mount(const uint32_t item_count, const char* bitmap_data, const bool mount_flag=true)
		{
			if(!mount_)
				Q_ASSERT(data_==NULL, "QBitMap: mount error, data_ is not null!");
			data_=const_cast<char*>(bitmap_data);
			mount_=mount_flag;
			item_count_=item_count;
			slot_count_=(item_count+SLOT_SIZE-1)>>3;
		}

		bool alloc(const uint32_t item_count, const bool set_flag)
		{
			Q_JUST_RETURN(data_!=NULL||item_count<=0, false);
			slot_count_=(item_count+SLOT_SIZE-1)>>3;
			data_=q_new_array<char>(sizeof(char)*slot_count_);
			Q_ASSERT(data_!=NULL, "QBitMap: alloc error, data_ is null!");
			item_count_=item_count;
			memset(data_, set_flag?0xFF:0x00, slot_count_*sizeof(char));
			set_count_=set_flag?item_count:0;
			return true;
		}

		bool copy(const uint32_t slot_count, const char* bitmap_data)
		{
			Q_JUST_RETURN(data_==NULL||slot_count_!=slot_count, false);
			memcpy(data_, bitmap_data, slot_count);
			set_count_=0;
			for(uint32_t pos=0; pos<item_count_; ++pos)
				if(bit(pos)) ++set_count_;
			return true;
		}

		bool bit(const uint32_t pos) const
		{
			Q_ASSERT(pos<item_count_, "QBitMap::test error, pos >= item_count_!");
			uint32_t quot=pos/SLOT_SIZE;
			uint32_t rem=pos%SLOT_SIZE;
			return (data_[quot]&BITMAP_MASK[rem])!=0;
		}

		void turn_on_bit(const uint32_t pos)
		{
			Q_ASSERT(pos<item_count_, "QBitMap::test error, pos >= item_count_!");
			uint32_t quot=pos/SLOT_SIZE;
			uint32_t rem=pos%SLOT_SIZE;
			if(!(data_[quot]&BITMAP_MASK[rem])) {
				data_[quot]|=BITMAP_MASK[rem];
				++set_count_;
			}
		}

		void turn_off_bit(const uint32_t pos)
		{
			Q_ASSERT(pos<item_count_, "QBitMap::test error, pos >= item_count_!");
			uint32_t quot=pos/SLOT_SIZE;
			uint32_t rem=pos%SLOT_SIZE;
			if(data_[quot]&BITMAP_MASK[rem]) {
				data_[quot]&=~BITMAP_MASK[rem];
				--set_count_;
			}
		}

		inline void clear()
		{
			memset(data_, 0, slot_count_*sizeof(char));
			set_count_=0;
		}

		inline char* get_data() const
		{return data_;}

		inline uint32_t get_set_count() const
		{return set_count_;}

		inline uint32_t get_slot_count() const
		{return slot_count_;}

		inline uint32_t get_item_count() const
		{return item_count_;}

	private:
		static const uint32_t SLOT_SIZE=8*sizeof(char);
		static const unsigned char BITMAP_MASK[SLOT_SIZE];
		static const uint32_t INVALID_INDEX=0xFFFFFFFF;

	protected:
		uint32_t item_count_;
		uint32_t slot_count_;
		mutable uint32_t set_count_;
		char* data_;
		bool mount_;
};

////////////////////////////////////////////////////////////////////////////////////////

class QBitMap2 {
	public:
		inline QBitMap2(uint32_t size=1<<20) :
			set_size_(size)
		{
			Q_ASSERT(set_size_>0, "QBitMap2: set_size_ must be larger than 0!");
			vec_size_=(set_size_+15)>>4;
			pBitVector=q_new_array<uint16_t>(vec_size_);
			Q_ASSERT(pBitVector!=NULL, "QBitMap2: alloc error, pBitVector is null!");
			for(uint32_t i=0; i<vec_size_; ++i) pBitVector[i]=0;
		}

		inline ~QBitMap2()
		{q_delete_array<uint16_t>(pBitVector);}

		void clear()
		{
			for(uint32_t i=0; i<vec_size_; ++i)
				pBitVector[i]=0;
		}

		bool bit(const uint32_t x)
		{
			uint32_t ad=x/16, id=x%16;
			uint16_t elem=pBitVector[ad];
			return ((elem>>(15-id))%2);
		}

		void set_bit(const uint32_t x, uint16_t v)
		{
			uint32_t ad=x/16, id=x%16;
			uint16_t elem=pBitVector[ad];
			uint16_t temp=elem>>(15-id);
			elem=elem<<(id+1);
			if(temp%2==0&&v==1) temp+=1;
			else if(temp%2==-1&&v==0) temp-=1;
			pBitVector[ad]=(temp<<(15-id))|(elem>>(id+1));
		}

		bool turn_on_bit(const uint32_t x)
		{
			Q_ASSERT(x>=0&&x<set_size_, "QBitMap2: out of range error!");
			if(bit(x)==0) {
				set_bit(x, 1);
				return true;
			}
			return false;
		}

		bool turn_off_bit(const uint32_t x)
		{
			Q_ASSERT(x>=0&&x<set_size_, "QBitMap2: out of range error!");
			if(bit(x)==1) {
				set_bit(x, 0);
				return true;
			}
			return false;
		}

	private:
		uint32_t set_size_;
		uint32_t vec_size_;
		uint16_t* pBitVector;
};

Q_END_NAMESPACE

#endif // __QBITMAP_H_
