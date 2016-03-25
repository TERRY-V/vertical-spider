/***************************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qalgorithm.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/05/10
**
**************************************************************************************************/

#ifndef __QALGORITHM_H_
#define __QALGORITHM_H_

#include "qglobal.h"
#include "qqueue.h"

Q_BEGIN_NAMESPACE

///////////////////////////////////////////////////////////////////////////////////////////////////
// 查找相关算法
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T_KEY>
static int32_t Compare_Key_Default(T_KEY tKey1, T_KEY tKey2)
{
	if(tKey1>tKey2)
		return 1;
	if(tKey1<tKey2)
		return -1;

	return 0;
}

template <typename T_KEY>
static int32_t Q_Sequential_Search_Default(int32_t lBegin, int32_t lEnd, T_KEY* tpLibrary, T_KEY tKey)
{
	if(lBegin>lEnd)
		return -1;

	for(int32_t lPos=lBegin; lPos<=lEnd; ++lPos) {
		if(tpLibrary[lPos]==tKey)
			return lPos;
	}

	return -1;
}

template <typename T_KEY>
static int32_t Q_Sequential_Search_Custom(int32_t lBegin, int32_t lEnd, T_KEY* tpLibrary, T_KEY tKey, int32_t (*Compare_Key)(T_KEY, T_KEY)=Compare_Key_Default)
{
	if(lBegin>lEnd)
		return -1;

	for(int32_t lPos=lBegin; lPos<=lEnd; ++lPos) {
		if(Compare_Key(tpLibrary[lPos], tKey)==0)
			return lPos;
	}

	return -1;
}

template <typename T_KEY>
static int32_t Q_Binary_Search_Default(int32_t lBegin, int32_t lEnd, T_KEY* tpLibrary, T_KEY tKey)
{
	if(lBegin>lEnd)
		return -1;

	T_KEY tMidVal;
	int32_t lMidVal;

	while(lBegin<=lEnd) {
		lMidVal=(lBegin+lEnd)>>1;
		tMidVal=tpLibrary[lMidVal];
		if(tMidVal<tKey)
			lBegin=lMidVal+1;
		else if(tMidVal>tKey)
			lEnd=lMidVal-1;
		else
			return lMidVal;
	}

	return -1;
}

template <typename T_KEY>
static int32_t Q_Binary_Search_Recursion(int32_t lBegin, int32_t lEnd, T_KEY* tpLibrary, T_KEY tKey)
{
	if(lBegin>lEnd)
		return -1;

	T_KEY tMidVal;
	int32_t lMidVal=0;

	if(lBegin<=lEnd) {
		lMidVal=(lBegin+lEnd)>>1;
		tMidVal=tpLibrary[lMidVal];
		if(tMidVal<tKey)
			lMidVal=Q_Binary_Search_Recursion(lMidVal+1, lEnd, tpLibrary, tKey);
		else if(tMidVal>tKey)
			lMidVal=Q_Binary_Search_Recursion(lBegin, lMidVal-1, tpLibrary, tKey);
	}

	return lMidVal;
}

template <typename T_KEY>
static int32_t Q_Binary_Search_Custom(int32_t lBegin, int32_t lEnd, T_KEY* tpLibrary, T_KEY tKey, int32_t (*Compare_Key)(T_KEY, T_KEY)=Compare_Key_Default)
{
	if(lBegin>lEnd)
		return -1;

	T_KEY tMidVal;
	int32_t lMidVal;

	while(lBegin<=lEnd) {
		lMidVal=(lBegin+lEnd)>>1;
		tMidVal=tpLibrary[lMidVal];
		if(Compare_Key(tMidVal, tKey)<0)
			lBegin=lMidVal+1;
		else if(Compare_Key(tMidVal, tKey)>0)
			lEnd=lMidVal-1;
		else
			return lMidVal;
	}

	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 改进的一维数组冒泡排序算法(稳定)
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T_KEY>
static int32_t Q_Bubble_Sort_Improved(T_KEY* v, int32_t lBegin, int32_t lEnd)
{
	int32_t exchange;
	for(int32_t i=lBegin+1; i<=lEnd; ++i) {
		exchange=0;

		for(int32_t j=lEnd; j>=i; --j) {
			if(v[j-1]>v[j]) {
				T_KEY temp=v[j-1];
				v[j-1]=v[j];
				v[j]=temp;
				exchange=1;
			}
		}

		if(exchange==0)
			break;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 一维数组折半插入排序算法(稳定)
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T_KEY>
static int32_t Q_Insert_Sort(T_KEY* v, int32_t lBegin, int32_t lEnd)
{
	T_KEY temp;
	int32_t i, low, high, middle, k;

	for(i=lBegin+1; i<=lEnd; ++i) {
		temp=v[i];
		low=lBegin;
		high=i-1;

		while(low<=high) {
			middle=(low+high)/2;
			if(temp<v[middle])
				high=middle-1;
			else
				low=middle+1;
		}

		for(k=i-1; k>=low; --k)
			v[k+1]=v[k];

		v[low]=temp;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 一维数组希尔排序(不稳定)
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T_KEY>
static int32_t Q_Shell_Sort(T_KEY* v, int32_t lBegin, int32_t lEnd)
{
	int32_t i, j, gap=lEnd-lBegin+1;
	T_KEY temp;

	do {
		gap=gap/3+1;
		for(i=lBegin+gap; i<=lEnd; ++i) {
			if(v[i]<v[i-gap]) {
				temp=v[i];
				j=i-gap;
				do {
					v[j+gap]=v[j];
					j-=gap;
				} while(j>=lBegin&&temp<v[j]);
				v[j+gap]=temp;
			}
		}
	} while(gap>1);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 一维数组快速排序的改进算法(不稳定)
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T_KEY>
static const T_KEY& median3(T_KEY* v, int32_t lBegin, int32_t lEnd)
{
	int32_t mid=(lBegin+lEnd)/2;
	int32_t k=lBegin;

	if(v[mid]<v[k]) k=mid;
	if(v[lEnd]<v[k]) k=lEnd;

	if(k!=lBegin) {
		T_KEY temp=v[k];
		v[k]=v[lBegin];
		v[lBegin]=temp;
	}

	if(mid!=lEnd&&v[mid]<v[lEnd]) {
		T_KEY temp=v[mid];
		v[mid]=v[lEnd];
		v[lEnd]=temp;
	}

	return v[lEnd];
}

template<typename T_KEY>
static int32_t partition(T_KEY* v, int32_t lBegin, int32_t lEnd)
{
	int32_t i=lBegin, j=lEnd-1;

	if(lBegin<lEnd) {
		T_KEY pivot=median3(v, lBegin, lEnd);

		Q_FOREVER {
			while(i<j&&v[i]<pivot) i++;
			while(i<j&&pivot<v[j]) j--;
			if(i<j) {
				T_KEY temp=v[i];
				v[i]=v[j];
				v[j]=temp;
			} else {
				break;
			}
		}

		if(v[i]>pivot) {
			v[lEnd]=v[i];
			v[i]=pivot;
		}
	}

	return i;
}

template <typename T_KEY>
static int32_t Q_Quick_Sort(T_KEY* v, int32_t lBegin, int32_t lEnd)
{
	if(lEnd-lBegin<=20) {
		Q_Insert_Sort(v, lBegin, lEnd);
	} else {
		int32_t pivotpos=partition(v, lBegin, lEnd);
		Q_Quick_Sort(v, lBegin, pivotpos-1);
		Q_Quick_Sort(v, pivotpos+1, lEnd);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 一维数组三路划分的快速排序算法(特适用于重复值问题)
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T_KEY>
static int32_t Q_Quick_Sort_3(T_KEY* v, int32_t lBegin, int32_t lEnd)
{
	if(lEnd<=lBegin)
		return 0;

	int32_t i, j, k, p, q;

	i=lBegin-1;
	j=lEnd;
	p=lBegin-1;
	q=lEnd;
	T_KEY pivot=v[lEnd];

	while(1) {
		while(v[++i]<pivot) if(i==j) break;
		while(pivot<v[--j]) if(j==i) break;

		if(i>=j)
			break;

		T_KEY temp=v[i];
		v[i]=v[j];
		v[j]=temp;

		if(v[i]==pivot) {
			p++;
			T_KEY temp=v[p];
			v[p]=v[i];
			v[i]=temp;
		}

		if(pivot==v[j]) {
			q--;
			T_KEY temp=v[q];
			v[q]=v[j];
			v[j]=temp;
		}
	}

	T_KEY temp=v[i];
	v[i]=v[lEnd];
	v[lEnd]=temp;
	j=i-1;
	i++;

	for(k=1; k<=p; k++, j--) {
		T_KEY temp=v[k];
		v[k]=v[j];
		v[j]=temp;
	}

	for(k=lEnd-1; k>=q; k--, i++) {
		T_KEY temp=v[k];
		v[k]=v[i];
		v[i]=temp;
	}

	Q_Quick_Sort_3(v, lBegin, j);
	Q_Quick_Sort_3(v, i, lEnd);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 一维数组选择排序算法(不稳定)，比较操作多，移动操作少
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T_KEY>
static int32_t Q_Slect_Sort(T_KEY* v, int32_t lBegin, int32_t lEnd)
{
	for(int32_t i=lBegin; i<lEnd; ++i) {
		int32_t k=i;

		for(int32_t j=i+1; j<=lEnd; ++j) {
			if(v[j]<v[k]) k=j;
		}

		if(k!=i) {
			T_KEY temp=v[i];
			v[i]=v[k];
			v[k]=temp;
		}
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 一维数组归并排序算法(稳定)
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T_KEY>
static void improvedMerge(T_KEY* v1, T_KEY* v2, int32_t lBegin, int32_t mid, int32_t lEnd)
{
	int32_t s1=lBegin, s2=lEnd, t=lBegin, k;

	for(k=lBegin; k<=mid; ++k)
		v2[k]=v1[k];

	for(k=mid+1; k<=lEnd; k++)
		v2[lEnd+mid+1-k]=v1[k];

	while(t<=lEnd) {
		if(v2[s1]<=v2[s2]) v1[t++]=v2[s1++];
		else v1[t++]=v2[s2--];
	}
}

template <typename T_KEY>
static void doSort(T_KEY* v1, T_KEY* v2, int32_t lBegin, int32_t lEnd)
{
	if(lBegin>=lEnd)
		return;

	if(lEnd-lBegin+1<20)
		return;

	int32_t mid=(lBegin+lEnd)/2;
	doSort(v1, v2, lBegin, mid);
	doSort(v1, v2, mid+1, lEnd);
	improvedMerge(v1, v2, lBegin, mid, lEnd);
}

template <typename T_KEY>
static int32_t Q_Merge_Sort(T_KEY* v1, T_KEY* v2, int32_t lBegin, int32_t lEnd)
{
	doSort(v1, v2, lBegin, lEnd);
	insertSort(v1, lBegin, lEnd);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 一维数组堆排序(不稳定)
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T_KEY>
static void heapAdjust(T_KEY* v, int32_t size, int32_t i)
{
	int32_t l=2*i+1;
	int32_t r=2*i+2;
	int32_t largest=i;

	if((l<size)&&(v[l]>v[i]))
		largest=l;
	if((r<size)&&(v[r]>v[largest]))
		largest=r;

	if(largest!=i) {
		T_KEY temp=v[i];
		v[i]=v[largest];
		v[largest]=temp;
		heapAdjust(v, size, largest);
	}
}

template <typename T_KEY>
static int32_t Q_Heap_Sort(T_KEY* v, int32_t size)
{
	for(int32_t i=size/2-1; i>=0; i--)
		heapAdjust(v, size, i);

	for(int32_t i=size-1; i>0; i--) {
		T_KEY temp=v[0];
		v[0]=v[i];
		v[i]=temp;
		size--;
		heapAdjust(v, size, 0);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 多关键码快排相关
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T_KEY_1>
static int32_t Compare_1K(T_KEY_1 key_1_1, T_KEY_1 key_1_2)
{
	if(key_1_1>key_1_2)
		return 1;
	if(key_1_1<key_1_2)
		return -1;

	return 0;
}

template <typename T_KEY_1>
static int32_t Compare_1K_Ascending(T_KEY_1 key_1_1, T_KEY_1 key_1_2)
{
	if(key_1_1>key_1_2)
		return 1;
	if(key_1_1<key_1_2)
		return -1;

	return 0;
}

template <typename T_KEY_1>
static int32_t Compare_1K_Descending(T_KEY_1 key_1_1, T_KEY_1 key_1_2)
{
	if(key_1_1>key_1_2)
		return -1;
	if(key_1_1<key_1_2)
		return 1;

	return 0;
}

template <typename T_KEY_1>
static int32_t Compare_1K_Ascending_S(T_KEY_1 key_1_1, T_KEY_1 key_1_2, int32_t pos_1, int32_t pos_2)
{
	if(key_1_1>key_1_2)
		return 1;
	if(key_1_1<key_1_2)
		return -1;
	if(pos_1>pos_2)
		return 1;
	if(pos_1<pos_2)
		return -1;

	return 0;
}

template <typename T_KEY_1>
static int32_t Compare_1K_Descending_S(T_KEY_1 key_1_1, T_KEY_1 key_1_2, int32_t pos_1, int32_t pos_2)
{
	if(key_1_1>key_1_2)
		return -1;
	if(key_1_1<key_1_2)
		return 1;
	if(pos_1>pos_2)
		return 1;
	if(pos_1<pos_2)
		return -1;

	return 0;
}

template <typename T_KEY_1, typename T_KEY_2>
static int32_t Compare_2K(T_KEY_1 key_1_1, T_KEY_1 key_1_2, T_KEY_2 key_2_1, T_KEY_2 key_2_2)
{
	if(key_1_1>key_1_2)
		return 1;
	if(key_1_1<key_1_2)
		return -1;

	if(key_2_1>key_2_2)
		return -1;
	if(key_2_1<key_2_2)
		return -1;

	return 0;
}

template <typename T_KEY_1, typename T_KEY_2, typename T_KEY_3>
static int32_t Compare_3K(T_KEY_1 key_1_1, T_KEY_1 key_1_2, T_KEY_2 key_2_1, T_KEY_2 key_2_2, T_KEY_3 key_3_1, T_KEY_3 key_3_2)
{
	if(key_1_1>key_1_2)
		return 1;
	if(key_1_1<key_1_2)
		return -1;

	if(key_2_1>key_2_2)
		return -1;
	if(key_2_1<key_2_2)
		return -1;

	if(key_3_1>key_3_2)
		return 1;
	if(key_3_1<key_3_2)
		return -1;

	return 0;
}

template <typename T_KEY_1>
static int32_t Q_Recursion_1K(int32_t lBegin, int32_t lEnd, T_KEY_1* tKey_1)
{
	if(lBegin>=lEnd)
		return 0;

	T_KEY_1 tTmp_1;

	if(lBegin+1==lEnd) {
		if(tKey_1[lBegin]>tKey_1[lEnd]) {
			tTmp_1=tKey_1[lBegin];
			tKey_1[lBegin]=tKey_1[lEnd];
			tKey_1[lEnd]=tTmp_1;
		}
		return 0;
	}

	int32_t lMid=(lBegin+lEnd)>>1;
	int32_t m=lBegin, n=lEnd;

	T_KEY_1 tMid_1=tKey_1[lMid];

	while(lBegin<lEnd) {
		while(lBegin<lEnd&&tKey_1[lBegin]<tMid_1) lBegin++;
		while(lBegin<lEnd&&tKey_1[lEnd]>tMid_1) lEnd--;
		if(lBegin<lEnd) {
			tTmp_1=tKey_1[lBegin];
			tKey_1[lBegin]=tKey_1[lEnd];
			tKey_1[lEnd]=tTmp_1;

			if(++lBegin<lEnd)
				lEnd--;
		}
	}

	if(tKey_1[lBegin]<tMid_1)
		lBegin++;

	if(lBegin-1>m)
		Q_Recursion_1K(m, lBegin-1, tKey_1);
	if(lEnd<n)
		Q_Recursion_1K(lEnd, n, tKey_1);

	return 0;
}

template <typename T_KEY_1>
static int32_t Q_Unrecursion_1K(int32_t lBegin, int32_t lEnd, T_KEY_1* tKey_1)
{
	int32_t stack_buf[1<<10];
	int32_t stack_pos=0;

	stack_buf[stack_pos++]=lBegin;
	stack_buf[stack_pos++]=lEnd;

	Q_FOREVER {
		if(stack_pos==0)
			break;

		lEnd=stack_buf[--stack_pos];
		lBegin=stack_buf[--stack_pos];

		if(lBegin>=lEnd)
			continue;

		T_KEY_1 tTmp_1;

		if(lBegin+1==lEnd) {
			if(tKey_1[lBegin]>tKey_1[lEnd]) {
				tTmp_1=tKey_1[lBegin];
				tKey_1[lBegin]=tKey_1[lEnd];
				tKey_1[lEnd]=tTmp_1;
			}
			continue;
		}

		int32_t lMid=(lBegin+lEnd)>>1;
		int32_t m=lBegin, n=lEnd;

		T_KEY_1 tMid_1=tKey_1[lMid];

		while(lBegin<lEnd) {
			while(lBegin<lEnd&&tKey_1[lBegin]<tMid_1) lBegin++;
			while(lBegin<lEnd&&tKey_1[lEnd]>tMid_1) lEnd--;
			if(lBegin<lEnd) {
				tTmp_1=tKey_1[lBegin];
				tKey_1[lBegin]=tKey_1[lEnd];
				tKey_1[lEnd]=tTmp_1;

				if(++lBegin<lEnd)
					lEnd--;
			}
		}

		if(tKey_1[lBegin]<tMid_1)
			lBegin++;

		if(lEnd<n) {
			stack_buf[stack_pos++]=lEnd;
			stack_buf[stack_pos++]=n;
		}
		if(lBegin-1>m) {
			stack_buf[stack_pos++]=m;
			stack_buf[stack_pos++]=lBegin-1;
		}
	}

	return 0;
}

template <typename T_KEY_1>
static int32_t Q_Recursion_1K_Custom(int32_t lBegin, int32_t lEnd, T_KEY_1* tKey_1, int32_t (*Compare_Key)(T_KEY_1, T_KEY_1)=Compare_1K)
{
	if(lBegin>=lEnd)
		return 0;

	T_KEY_1 tTmp_1;

	if(lBegin+1==lEnd) {
		if(Compare_Key(tKey_1[lBegin], tKey_1[lEnd])>0) {
			tTmp_1=tKey_1[lBegin];
			tKey_1[lBegin]=tKey_1[lEnd];
			tKey_1[lEnd]=tTmp_1;
		}
		return 0;
	}

	int32_t lMid=(lBegin+lEnd)>>1;
	int32_t m=lBegin, n=lEnd;

	T_KEY_1 tMid_1=tKey_1[lMid];

	while(lBegin<lEnd) {
		while(lBegin<lEnd&&Compare_Key(tKey_1[lBegin], tMid_1)<0) lBegin++;
		while(lBegin<lEnd&&Compare_Key(tKey_1[lEnd], tMid_1)>0) lEnd--;
		if(lBegin<lEnd) {
			tTmp_1=tKey_1[lBegin];
			tKey_1[lBegin]=tKey_1[lEnd];
			tKey_1[lEnd]=tTmp_1;

			if(++lBegin<lEnd)
				lEnd--;
		}
	}

	if(Compare_Key(tKey_1[lBegin], tMid_1)<0)
		lBegin++;

	if(lBegin-1>m)
		Q_Recursion_1K_Custom(m, lBegin-1, tKey_1, Compare_Key);
	if(lEnd<n)
		Q_Recursion_1K_Custom(lEnd, n, tKey_1, Compare_Key);

	return 0;
}

template <typename T_KEY_1>
static int32_t Q_Unrecursion_1K_Custom(int32_t lBegin, int32_t lEnd, T_KEY_1* tKey_1, int32_t (*Compare_Key)(T_KEY_1, T_KEY_1)=Compare_1K)
{
	int32_t stack_buf[1<<10];
	int32_t stack_pos=0;

	stack_buf[stack_pos++]=lBegin;
	stack_buf[stack_pos++]=lEnd;

	Q_FOREVER {
		if(stack_pos==0)
			break;

		lEnd=stack_buf[--stack_pos];
		lBegin=stack_buf[--stack_pos];

		if(lBegin>=lEnd)
			continue;

		T_KEY_1 tTmp_1;

		if(lBegin+1==lEnd) {
			if(Compare_Key(tKey_1[lBegin], tKey_1[lEnd])>0) {
				tTmp_1=tKey_1[lBegin];
				tKey_1[lBegin]=tKey_1[lEnd];
				tKey_1[lEnd]=tTmp_1;
			}
			continue;
		}

		int32_t lMid=(lBegin+lEnd)>>1;
		int32_t m=lBegin, n=lEnd;

		T_KEY_1 tMid_1=tKey_1[lMid];

		while(lBegin<lEnd) {
			while(lBegin<lEnd&&Compare_Key(tKey_1[lBegin], tMid_1)<0) lBegin++;
			while(lBegin<lEnd&&Compare_Key(tKey_1[lEnd], tMid_1)>0) lEnd--;
			if(lBegin<lEnd) {
				tTmp_1=tKey_1[lBegin];
				tKey_1[lBegin]=tKey_1[lEnd];
				tKey_1[lEnd]=tTmp_1;

				if(++lBegin<lEnd)
					lEnd--;
			}
		}

		if(Compare_Key(tKey_1[lBegin], tMid_1)<0)
			lBegin++;

		if(lEnd<n) {
			stack_buf[stack_pos++]=lEnd;
			stack_buf[stack_pos++]=n;
		}
		if(lBegin-1>m) {
			stack_buf[stack_pos++]=m;
			stack_buf[stack_pos++]=lBegin-1;
		}
	}

	return 0;
}

template <typename T_KEY_1>
static int32_t Q_Recursion_1K_Custom_S(int32_t lBegin, int32_t lEnd, T_KEY_1* tKey_1, int32_t (*Compare_Key)(T_KEY_1, T_KEY_1, int32_t, int32_t)=Compare_1K_Ascending_S)
{
	if(lBegin>=lEnd)
		return 0;

	T_KEY_1 tTmp_1;

	if(lBegin+1==lEnd) {
		if(Compare_Key(tKey_1[lBegin], tKey_1[lEnd], lBegin, lEnd)>0) {
			tTmp_1=tKey_1[lBegin];
			tKey_1[lBegin]=tKey_1[lEnd];
			tKey_1[lEnd]=tTmp_1;
		}
		return 0;
	}

	int32_t lMid=(lBegin+lEnd)>>1;
	int32_t m=lBegin, n=lEnd;

	T_KEY_1 tMid_1=tKey_1[lMid];

	while(lBegin<lEnd) {
		while(lBegin<lEnd&&Compare_Key(tKey_1[lBegin], tMid_1, lBegin, lMid)<0) lBegin++;
		while(lBegin<lEnd&&Compare_Key(tKey_1[lEnd], tMid_1, lEnd, lMid)>0) lEnd--;
		if(lBegin<lEnd) {
			tTmp_1=tKey_1[lBegin];
			tKey_1[lBegin]=tKey_1[lEnd];
			tKey_1[lEnd]=tTmp_1;

			if(++lBegin<lEnd)
				lEnd--;
		}
	}

	if(Compare_Key(tKey_1[lBegin], tMid_1, lBegin, lMid)<0)
		lBegin++;

	if(lBegin-1>m)
		Q_Recursion_1K_Custom(m, lBegin-1, tKey_1, Compare_Key);
	if(lEnd<n)
		Q_Recursion_1K_Custom(lEnd, n, tKey_1, Compare_Key);

	return 0;
}

template <typename T_KEY_1>
static int32_t Q_Unrecursion_1K_Custom_S(int32_t lBegin, int32_t lEnd, T_KEY_1* tKey_1, int32_t (*Compare_Key)(T_KEY_1, T_KEY_1, int32_t, int32_t)=Compare_1K_Ascending_S)
{
	int32_t stack_buf[1<<10];
	int32_t stack_pos=0;

	stack_buf[stack_pos++]=lBegin;
	stack_buf[stack_pos++]=lEnd;

	Q_FOREVER {
		if(stack_pos==0)
			break;

		lEnd=stack_buf[--stack_pos];
		lBegin=stack_buf[--stack_pos];

		if(lBegin>=lEnd)
			continue;

		T_KEY_1 tTmp_1;

		if(lBegin+1==lEnd) {
			if(Compare_Key(tKey_1[lBegin], tKey_1[lEnd], lBegin, lEnd)>0) {
				tTmp_1=tKey_1[lBegin];
				tKey_1[lBegin]=tKey_1[lEnd];
				tKey_1[lEnd]=tTmp_1;
			}
			continue;
		}

		int32_t lMid=(lBegin+lEnd)>>1;
		int32_t m=lBegin, n=lEnd;

		T_KEY_1 tMid_1=tKey_1[lMid];

		while(lBegin<lEnd) {
			while(lBegin<lEnd&&Compare_Key(tKey_1[lBegin], tMid_1, lBegin, lMid)<0) lBegin++;
			while(lBegin<lEnd&&Compare_Key(tKey_1[lEnd], tMid_1, lEnd, lMid)>0) lEnd--;
			if(lBegin<lEnd) {
				tTmp_1=tKey_1[lBegin];
				tKey_1[lBegin]=tKey_1[lEnd];
				tKey_1[lEnd]=tTmp_1;

				if(++lBegin<lEnd)
					lEnd--;
			}
		}

		if(Compare_Key(tKey_1[lBegin], tMid_1, lBegin, lMid)<0)
			lBegin++;

		if(lEnd<n) {
			stack_buf[stack_pos++]=lEnd;
			stack_buf[stack_pos++]=n;
		}
		if(lBegin-1>m) {
			stack_buf[stack_pos++]=m;
			stack_buf[stack_pos++]=lBegin-1;
		}
	}

	return 0;
}

template <typename T_KEY_1, typename T_KEY_2>
static int32_t Q_Recursion_1K_1P(int32_t lBegin, int32_t lEnd, T_KEY_1* tKey_1, T_KEY_2* tKey_2)
{
	if(lBegin>=lEnd)
		return 0;

	T_KEY_1 tTmp_1;
	T_KEY_2 tTmp_2;

	if(lBegin+1==lEnd) {
		if(tKey_1[lBegin]>tKey_1[lEnd]) {
			tTmp_1=tKey_1[lBegin];
			tKey_1[lBegin]=tKey_1[lEnd];
			tKey_1[lEnd]=tTmp_1;

			tTmp_2=tKey_2[lBegin];
			tKey_2[lBegin]=tKey_2[lEnd];
			tKey_2[lEnd]=tTmp_2;
		}
		return 0;
	}

	int32_t lMid=(lBegin+lEnd)>>1;
	int32_t m=lBegin, n=lEnd;

	T_KEY_1 tMid_1=tKey_1[lMid];

	while(lBegin<lEnd) {
		while(lBegin<lEnd&&tKey_1[lBegin]<tMid_1) lBegin++;
		while(lBegin<lEnd&&tKey_1[lEnd]>tMid_1) lEnd--;
		if(lBegin<lEnd) {
			tTmp_1=tKey_1[lBegin];
			tKey_1[lBegin]=tKey_1[lEnd];
			tKey_1[lEnd]=tTmp_1;

			tTmp_2=tKey_2[lBegin];
			tKey_2[lBegin]=tKey_2[lEnd];
			tKey_2[lEnd]=tTmp_2;

			if(++lBegin<lEnd)
				lEnd--;
		}
	}

	if(tKey_1[lBegin]<tMid_1)
		lBegin++;

	if(lBegin-1>m)
		Q_Recursion_1K_1P(m, lBegin-1, tKey_1, tKey_2);
	if(lEnd<n)
		Q_Recursion_1K_1P(lEnd, n, tKey_1, tKey_2);

	return 0;
}

template <typename T_KEY_1, typename T_KEY_2>
static int32_t Q_Unrecursion_1K_1P(int32_t lBegin, int32_t lEnd, T_KEY_1* tKey_1, T_KEY_2* tKey_2)
{
	int32_t stack_buf[1<<10];
	int32_t stack_pos=0;

	stack_buf[stack_pos++]=lBegin;
	stack_buf[stack_pos++]=lEnd;

	Q_FOREVER {
		if(stack_pos==0)
			break;

		lEnd=stack_buf[--stack_pos];
		lBegin=stack_buf[--stack_pos];

		if(lBegin>=lEnd)
			continue;

		T_KEY_1 tTmp_1;
		T_KEY_2 tTmp_2;

		if(lBegin+1==lEnd) {
			if(tKey_1[lBegin]>tKey_1[lEnd]) {
				tTmp_1=tKey_1[lBegin];
				tKey_1[lBegin]=tKey_1[lEnd];
				tKey_1[lEnd]=tTmp_1;

				tTmp_2=tKey_2[lBegin];
				tKey_2[lBegin]=tKey_2[lEnd];
				tKey_2[lEnd]=tTmp_2;
			}
			continue;
		}

		int32_t lMid=(lBegin+lEnd)>>1;
		int32_t m=lBegin, n=lEnd;

		T_KEY_1 tMid_1=tKey_1[lMid];

		while(lBegin<lEnd) {
			while(lBegin<lEnd&&tKey_1[lBegin]<tMid_1) lBegin++;
			while(lBegin<lEnd&&tKey_1[lEnd]>tMid_1) lEnd--;
			if(lBegin<lEnd) {
				tTmp_1=tKey_1[lBegin];
				tKey_1[lBegin]=tKey_1[lEnd];
				tKey_1[lEnd]=tTmp_1;

				tTmp_2=tKey_2[lBegin];
				tKey_2[lBegin]=tKey_2[lEnd];
				tKey_2[lEnd]=tTmp_2;

				if(++lBegin<lEnd)
					lEnd--;
			}
		}

		if(tKey_1[lBegin]<tMid_1)
			lBegin++;

		if(lEnd<n) {
			stack_buf[stack_pos++]=lEnd;
			stack_buf[stack_pos++]=n;
		}
		if(lBegin-1>m) {
			stack_buf[stack_pos++]=m;
			stack_buf[stack_pos++]=lBegin-1;
		}
	}

	return 0;
}

template <typename T_KEY_1, typename T_KEY_2, typename T_KEY_3>
static int32_t Q_Recursion_1K_2P(int32_t lBegin, int32_t lEnd, T_KEY_1* tKey_1, T_KEY_2* tKey_2, T_KEY_3* tKey_3)
{
	if(lBegin>=lEnd)
		return 0;

	T_KEY_1 tTmp_1;
	T_KEY_2 tTmp_2;
	T_KEY_3 tTmp_3;

	if(lBegin+1==lEnd) {
		if(tKey_1[lBegin]>tKey_1[lEnd]) {
			tTmp_1=tKey_1[lBegin];
			tKey_1[lBegin]=tKey_1[lEnd];
			tKey_1[lEnd]=tTmp_1;

			tTmp_2=tKey_2[lBegin];
			tKey_2[lBegin]=tKey_2[lEnd];
			tKey_2[lEnd]=tTmp_2;

			tTmp_3=tKey_3[lBegin];
			tKey_3[lBegin]=tKey_3[lEnd];
			tKey_3[lEnd]=tTmp_3;
		}
		return 0;
	}

	int32_t lMid=(lBegin+lEnd)>>1;
	int32_t m=lBegin, n=lEnd;

	T_KEY_1 tMid_1=tKey_1[lMid];

	while(lBegin<lEnd) {
		while(lBegin<lEnd&&tKey_1[lBegin]<tMid_1) lBegin++;
		while(lBegin<lEnd&&tKey_1[lEnd]>tMid_1) lEnd--;
		if(lBegin<lEnd) {
			tTmp_1=tKey_1[lBegin];
			tKey_1[lBegin]=tKey_1[lEnd];
			tKey_1[lEnd]=tTmp_1;

			tTmp_2=tKey_2[lBegin];
			tKey_2[lBegin]=tKey_2[lEnd];
			tKey_2[lEnd]=tTmp_2;

			tTmp_3=tKey_3[lBegin];
			tKey_3[lBegin]=tKey_3[lEnd];
			tKey_3[lEnd]=tTmp_3;

			if(++lBegin<lEnd)
				lEnd--;
		}
	}

	if(tKey_1[lBegin]<tMid_1)
		lBegin++;

	if(lBegin-1>m)
		Q_Recursion_1K_2P(m, lBegin-1, tKey_1, tKey_2, tKey_3);
	if(lEnd<n)
		Q_Recursion_1K_2P(lEnd, n, tKey_1, tKey_2, tKey_3);

	return 0;
}

template <typename T_KEY_1, typename T_KEY_2, typename T_KEY_3>
static int32_t Q_Unrecursion_1K_2P(int32_t lBegin, int32_t lEnd, T_KEY_1* tKey_1, T_KEY_2* tKey_2, T_KEY_3* tKey_3)
{
	int32_t stack_buf[1<<10];
	int32_t stack_pos=0;

	stack_buf[stack_pos++]=lBegin;
	stack_buf[stack_pos++]=lEnd;

	Q_FOREVER {
		if(stack_pos==0)
			break;

		lEnd=stack_buf[--stack_pos];
		lBegin=stack_buf[--stack_pos];

		if(lBegin>=lEnd)
			continue;

		T_KEY_1 tTmp_1;
		T_KEY_2 tTmp_2;
		T_KEY_3 tTmp_3;

		if(lBegin+1==lEnd) {
			if(tKey_1[lBegin]>tKey_1[lEnd]) {
				tTmp_1=tKey_1[lBegin];
				tKey_1[lBegin]=tKey_1[lEnd];
				tKey_1[lEnd]=tTmp_1;

				tTmp_2=tKey_2[lBegin];
				tKey_2[lBegin]=tKey_2[lEnd];
				tKey_2[lEnd]=tTmp_2;

				tTmp_3=tKey_3[lBegin];
				tKey_3[lBegin]=tKey_3[lEnd];
				tKey_3[lEnd]=tTmp_3;
			}
			continue;
		}

		int32_t lMid=(lBegin+lEnd)>>1;
		int32_t m=lBegin, n=lEnd;

		T_KEY_1 tMid_1=tKey_1[lMid];

		while(lBegin<lEnd) {
			while(lBegin<lEnd&&tKey_1[lBegin]<tMid_1) lBegin++;
			while(lBegin<lEnd&&tKey_1[lEnd]>tMid_1) lEnd--;
			if(lBegin<lEnd) {
				tTmp_1=tKey_1[lBegin];
				tKey_1[lBegin]=tKey_1[lEnd];
				tKey_1[lEnd]=tTmp_1;

				tTmp_2=tKey_2[lBegin];
				tKey_2[lBegin]=tKey_2[lEnd];
				tKey_2[lEnd]=tTmp_2;

				tTmp_3=tKey_3[lBegin];
				tKey_3[lBegin]=tKey_3[lEnd];
				tKey_3[lEnd]=tTmp_3;

				if(++lBegin<lEnd)
					lEnd--;
			}
		}

		if(tKey_1[lBegin]<tMid_1)
			lBegin++;

		if(lEnd<n) {
			stack_buf[stack_pos++]=lEnd;
			stack_buf[stack_pos++]=n;
		}
		if(lBegin-1>m) {
			stack_buf[stack_pos++]=m;
			stack_buf[stack_pos++]=lBegin-1;
		}
	}

	return 0;
}

template <typename T_KEY_1, typename T_KEY_2, typename T_KEY_3, typename T_KEY_4>
static int32_t Q_Recursion_1K_3P(int32_t lBegin, int32_t lEnd, T_KEY_1* tKey_1, T_KEY_2* tKey_2, T_KEY_3* tKey_3, T_KEY_4* tKey_4)
{
	if(lBegin>=lEnd)
		return 0;

	T_KEY_1 tTmp_1;
	T_KEY_2 tTmp_2;
	T_KEY_3 tTmp_3;
	T_KEY_4 tTmp_4;

	if(lBegin+1==lEnd) {
		if(tKey_1[lBegin]>tKey_1[lEnd]) {
			tTmp_1=tKey_1[lBegin];
			tKey_1[lBegin]=tKey_1[lEnd];
			tKey_1[lEnd]=tTmp_1;

			tTmp_2=tKey_2[lBegin];
			tKey_2[lBegin]=tKey_2[lEnd];
			tKey_2[lEnd]=tTmp_2;

			tTmp_3=tKey_3[lBegin];
			tKey_3[lBegin]=tKey_3[lEnd];
			tKey_3[lEnd]=tTmp_3;

			tTmp_4=tKey_4[lBegin];
			tKey_4[lBegin]=tKey_4[lEnd];
			tKey_4[lEnd]=tTmp_4;
		}
		return 0;
	}

	int32_t lMid=(lBegin+lEnd)>>1;
	int32_t m=lBegin, n=lEnd;

	T_KEY_1 tMid_1=tKey_1[lMid];

	while(lBegin<lEnd) {
		while(lBegin<lEnd&&tKey_1[lBegin]<tMid_1) lBegin++;
		while(lBegin<lEnd&&tKey_1[lEnd]>tMid_1) lEnd--;
		if(lBegin<lEnd) {
			tTmp_1=tKey_1[lBegin];
			tKey_1[lBegin]=tKey_1[lEnd];
			tKey_1[lEnd]=tTmp_1;

			tTmp_2=tKey_2[lBegin];
			tKey_2[lBegin]=tKey_2[lEnd];
			tKey_2[lEnd]=tTmp_2;

			tTmp_3=tKey_3[lBegin];
			tKey_3[lBegin]=tKey_3[lEnd];
			tKey_3[lEnd]=tTmp_3;

			tTmp_4=tKey_4[lBegin];
			tKey_4[lBegin]=tKey_4[lEnd];
			tKey_4[lEnd]=tTmp_4;

			if(++lBegin<lEnd)
				lEnd--;
		}
	}

	if(tKey_1[lBegin]<tMid_1)
		lBegin++;

	if(lBegin-1>m)
		Q_Recursion_1K_3P(m, lBegin-1, tKey_1, tKey_2, tKey_3, tKey_4);
	if(lEnd<n)
		Q_Recursion_1K_3P(lEnd, n, tKey_1, tKey_2, tKey_3, tKey_4);

	return 0;
}

template <typename T_KEY_1, typename T_KEY_2, typename T_KEY_3, typename T_KEY_4>
static int32_t Q_Unrecursion_1K_3P(int32_t lBegin, int32_t lEnd, T_KEY_1* tKey_1, T_KEY_2* tKey_2, T_KEY_3* tKey_3, T_KEY_4* tKey_4)
{
	int32_t stack_buf[1<<10];
	int32_t stack_pos=0;

	stack_buf[stack_pos++]=lBegin;
	stack_buf[stack_pos++]=lEnd;

	Q_FOREVER {
		if(stack_pos==0)
			break;

		lEnd=stack_buf[--stack_pos];
		lBegin=stack_buf[--stack_pos];

		if(lBegin>=lEnd)
			continue;

		T_KEY_1 tTmp_1;
		T_KEY_2 tTmp_2;
		T_KEY_3 tTmp_3;
		T_KEY_4 tTmp_4;

		if(lBegin+1==lEnd) {
			if(tKey_1[lBegin]>tKey_1[lEnd]) {
				tTmp_1=tKey_1[lBegin];
				tKey_1[lBegin]=tKey_1[lEnd];
				tKey_1[lEnd]=tTmp_1;

				tTmp_2=tKey_2[lBegin];
				tKey_2[lBegin]=tKey_2[lEnd];
				tKey_2[lEnd]=tTmp_2;

				tTmp_3=tKey_3[lBegin];
				tKey_3[lBegin]=tKey_3[lEnd];
				tKey_3[lEnd]=tTmp_3;

				tTmp_4=tKey_4[lBegin];
				tKey_4[lBegin]=tKey_4[lEnd];
				tKey_4[lEnd]=tTmp_4;
			}
			continue;
		}

		int32_t lMid=(lBegin+lEnd)>>1;
		int32_t m=lBegin, n=lEnd;

		T_KEY_1 tMid_1=tKey_1[lMid];

		while(lBegin<lEnd) {
			while(lBegin<lEnd&&tKey_1[lBegin]<tMid_1) lBegin++;
			while(lBegin<lEnd&&tKey_1[lEnd]>tMid_1) lEnd--;
			if(lBegin<lEnd) {
				tTmp_1=tKey_1[lBegin];
				tKey_1[lBegin]=tKey_1[lEnd];
				tKey_1[lEnd]=tTmp_1;

				tTmp_2=tKey_2[lBegin];
				tKey_2[lBegin]=tKey_2[lEnd];
				tKey_2[lEnd]=tTmp_2;

				tTmp_3=tKey_3[lBegin];
				tKey_3[lBegin]=tKey_3[lEnd];
				tKey_3[lEnd]=tTmp_3;

				tTmp_4=tKey_4[lBegin];
				tKey_4[lBegin]=tKey_4[lEnd];
				tKey_4[lEnd]=tTmp_4;

				if(++lBegin<lEnd)
					lEnd--;
			}
		}

		if(tKey_1[lBegin]<tMid_1)
			lBegin++;

		if(lEnd<n) {
			stack_buf[stack_pos++]=lEnd;
			stack_buf[stack_pos++]=n;
		}
		if(lBegin-1>m) {
			stack_buf[stack_pos++]=m;
			stack_buf[stack_pos++]=lBegin-1;
		}
	}

	return 0;
}

template <typename T_KEY_1, typename T_KEY_2>
static int32_t Q_Recursion_2K(int32_t lBegin, int32_t lEnd, T_KEY_1* tKey_1, T_KEY_2* tKey_2)
{
	if(lBegin>=lEnd)
		return 0;

	T_KEY_1 tTmp_1;
	T_KEY_2 tTmp_2;

	if(lBegin+1==lEnd) {
		if(Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lBegin], tKey_1[lEnd], tKey_2[lBegin], tKey_2[lEnd])>0) {
			tTmp_1=tKey_1[lBegin];
			tKey_1[lBegin]=tKey_1[lEnd];
			tKey_1[lEnd]=tTmp_1;

			tTmp_2=tKey_2[lBegin];
			tKey_2[lBegin]=tKey_2[lEnd];
			tKey_2[lEnd]=tTmp_2;
		}
		return 0;
	}

	int32_t lMid=(lBegin+lEnd)>>1;
	int32_t m=lBegin, n=lEnd;

	T_KEY_1 tMid_1=tKey_1[lMid];
	T_KEY_2 tMid_2=tKey_2[lMid];

	while(lBegin<lEnd) {
		while(lBegin<lEnd&&Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2)<0) lBegin++;
		while(lBegin<lEnd&&Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lEnd], tMid_1, tKey_2[lEnd], tMid_2)>0) lEnd--;
		if(lBegin<lEnd) {
			tTmp_1=tKey_1[lBegin];
			tKey_1[lBegin]=tKey_1[lEnd];
			tKey_1[lEnd]=tTmp_1;

			tTmp_2=tKey_2[lBegin];
			tKey_2[lBegin]=tKey_2[lEnd];
			tKey_2[lEnd]=tTmp_2;

			if(++lBegin<lEnd)
				lEnd--;
		}
	}

	if(Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2)<0)
		lBegin++;

	if(lBegin-1>m)
		Q_Recursion_2K(m, lBegin-1, tKey_1, tKey_2);
	if(lEnd<n)
		Q_Recursion_2K(lEnd, n, tKey_1, tKey_2);

	return 0;
}

template <typename T_KEY_1, typename T_KEY_2>
static int32_t Q_Unrecursion_2K(int32_t lBegin, int32_t lEnd, T_KEY_1* tKey_1, T_KEY_2* tKey_2)
{
	int32_t stack_buf[1<<10];
	int32_t stack_pos=0;

	stack_buf[stack_pos++]=lBegin;
	stack_buf[stack_pos++]=lEnd;

	Q_FOREVER {
		if(stack_pos==0)
			break;

		lEnd=stack_buf[--stack_pos];
		lBegin=stack_buf[--stack_pos];

		if(lBegin>=lEnd)
			continue;

		T_KEY_1 tTmp_1;
		T_KEY_2 tTmp_2;

		if(lBegin+1==lEnd) {
			if(Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lBegin], tKey_1[lEnd], tKey_2[lBegin], tKey_2[lEnd])>0) {
				tTmp_1=tKey_1[lBegin];
				tKey_1[lBegin]=tKey_1[lEnd];
				tKey_1[lEnd]=tTmp_1;

				tTmp_2=tKey_2[lBegin];
				tKey_2[lBegin]=tKey_2[lEnd];
				tKey_2[lEnd]=tTmp_2;
			}
			continue;
		}

		int32_t lMid=(lBegin+lEnd)>>1;
		int32_t m=lBegin, n=lEnd;

		T_KEY_1 tMid_1=tKey_1[lMid];
		T_KEY_2 tMid_2=tKey_2[lMid];

		while(lBegin<lEnd) {
			while(lBegin<lEnd&&Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2)<0) lBegin++;
			while(lBegin<lEnd&&Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lEnd], tMid_1, tKey_2[lEnd], tMid_2)>0) lEnd--;
			if(lBegin<lEnd) {
				tTmp_1=tKey_1[lBegin];
				tKey_1[lBegin]=tKey_1[lEnd];
				tKey_1[lEnd]=tTmp_1;

				tTmp_2=tKey_2[lBegin];
				tKey_2[lBegin]=tKey_2[lEnd];
				tKey_2[lEnd]=tTmp_2;

				if(++lBegin<lEnd)
					lEnd--;
			}
		}

		if(Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2)<0)
			lBegin++;

		if(lEnd<n) {
			stack_buf[stack_pos++]=lEnd;
			stack_buf[stack_pos++]=n;
		}
		if(lBegin-1>m) {
			stack_buf[stack_pos++]=m;
			stack_buf[stack_pos++]=lBegin-1;
		}
	}

	return 0;
}

template <typename T_KEY_1, typename T_KEY_2, typename T_KEY_3>
static int32_t Q_Recursion_2K_1P(int32_t lBegin, int32_t lEnd, T_KEY_1* tKey_1, T_KEY_2* tKey_2, T_KEY_3* tKey_3)
{
	if(lBegin>=lEnd)
		return 0;

	T_KEY_1 tTmp_1;
	T_KEY_2 tTmp_2;
	T_KEY_3 tTmp_3;

	if(lBegin+1==lEnd) {
		if(Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lBegin], tKey_1[lEnd], tKey_2[lBegin], tKey_2[lEnd])>0) {
			tTmp_1=tKey_1[lBegin];
			tKey_1[lBegin]=tKey_1[lEnd];
			tKey_1[lEnd]=tTmp_1;

			tTmp_2=tKey_2[lBegin];
			tKey_2[lBegin]=tKey_2[lEnd];
			tKey_2[lEnd]=tTmp_2;

			tTmp_3=tKey_3[lBegin];
			tKey_3[lBegin]=tKey_3[lEnd];
			tKey_3[lEnd]=tTmp_3;
		}
		return 0;
	}

	int32_t lMid=(lBegin+lEnd)>>1;
	int32_t m=lBegin, n=lEnd;

	T_KEY_1 tMid_1=tKey_1[lMid];
	T_KEY_2 tMid_2=tKey_2[lMid];

	while(lBegin<lEnd) {
		while(lBegin<lEnd&&Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2)<0) lBegin++;
		while(lBegin<lEnd&&Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lEnd], tMid_1, tKey_2[lEnd], tMid_2)>0) lEnd--;
		if(lBegin<lEnd) {
			tTmp_1=tKey_1[lBegin];
			tKey_1[lBegin]=tKey_1[lEnd];
			tKey_1[lEnd]=tTmp_1;

			tTmp_2=tKey_2[lBegin];
			tKey_2[lBegin]=tKey_2[lEnd];
			tKey_2[lEnd]=tTmp_2;

			tTmp_3=tKey_3[lBegin];
			tKey_3[lBegin]=tKey_3[lEnd];
			tKey_3[lEnd]=tTmp_3;

			if(++lBegin<lEnd)
				lEnd--;
		}
	}

	if(Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2)<0)
		lBegin++;

	if(lBegin-1>m)
		Q_Recursion_2K_1P(m, lBegin-1, tKey_1, tKey_2, tKey_3);
	if(lEnd<n)
		Q_Recursion_2K_1P(lEnd, n, tKey_1, tKey_2, tKey_3);

	return 0;
}

template <typename T_KEY_1, typename T_KEY_2, typename T_KEY_3>
static int32_t Q_Unrecursion_2K_1P(int32_t lBegin, int32_t lEnd, T_KEY_1* tKey_1, T_KEY_2* tKey_2, T_KEY_3* tKey_3)
{
	int32_t stack_buf[1<<10];
	int32_t stack_pos=0;

	stack_buf[stack_pos++]=lBegin;
	stack_buf[stack_pos++]=lEnd;

	Q_FOREVER {
		if(stack_pos==0)
			break;

		lEnd=stack_buf[--stack_pos];
		lBegin=stack_buf[--stack_pos];

		if(lBegin>=lEnd)
			continue;

		T_KEY_1 tTmp_1;
		T_KEY_2 tTmp_2;
		T_KEY_3 tTmp_3;

		if(lBegin+1==lEnd) {
			if(Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lBegin], tKey_1[lEnd], tKey_2[lBegin], tKey_2[lEnd])>0) {
				tTmp_1=tKey_1[lBegin];
				tKey_1[lBegin]=tKey_1[lEnd];
				tKey_1[lEnd]=tTmp_1;

				tTmp_2=tKey_2[lBegin];
				tKey_2[lBegin]=tKey_2[lEnd];
				tKey_2[lEnd]=tTmp_2;

				tTmp_3=tKey_3[lBegin];
				tKey_3[lBegin]=tKey_3[lEnd];
				tKey_3[lEnd]=tTmp_3;
			}
			continue;
		}

		int32_t lMid=(lBegin+lEnd)>>1;
		int32_t m=lBegin, n=lEnd;

		T_KEY_1 tMid_1=tKey_1[lMid];
		T_KEY_2 tMid_2=tKey_2[lMid];

		while(lBegin<lEnd) {
			while(lBegin<lEnd&&Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2)<0) lBegin++;
			while(lBegin<lEnd&&Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lEnd], tMid_1, tKey_2[lEnd], tMid_2)>0) lEnd--;
			if(lBegin<lEnd) {
				tTmp_1=tKey_1[lBegin];
				tKey_1[lBegin]=tKey_1[lEnd];
				tKey_1[lEnd]=tTmp_1;

				tTmp_2=tKey_2[lBegin];
				tKey_2[lBegin]=tKey_2[lEnd];
				tKey_2[lEnd]=tTmp_2;

				tTmp_3=tKey_3[lBegin];
				tKey_3[lBegin]=tKey_3[lEnd];
				tKey_3[lEnd]=tTmp_3;

				if(++lBegin<lEnd)
					lEnd--;
			}
		}

		if(Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2)<0)
			lBegin++;

		if(lEnd<n) {
			stack_buf[stack_pos++]=lEnd;
			stack_buf[stack_pos++]=n;
		}
		if(lBegin-1>m) {
			stack_buf[stack_pos++]=m;
			stack_buf[stack_pos++]=lBegin-1;
		}
	}

	return 0;
}

template <typename T_KEY_1, typename T_KEY_2, typename T_KEY_3, typename T_KEY_4>
static int32_t Q_Recursion_2K_2P(int32_t lBegin, int32_t lEnd, T_KEY_1* tKey_1, T_KEY_2* tKey_2, T_KEY_3* tKey_3, T_KEY_4* tKey_4)
{
	if(lBegin>=lEnd)
		return 0;

	T_KEY_1 tTmp_1;
	T_KEY_2 tTmp_2;
	T_KEY_3 tTmp_3;
	T_KEY_4 tTmp_4;

	if(lBegin+1==lEnd) {
		if(Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lBegin], tKey_1[lEnd], tKey_2[lBegin], tKey_2[lEnd])>0) {
			tTmp_1=tKey_1[lBegin];
			tKey_1[lBegin]=tKey_1[lEnd];
			tKey_1[lEnd]=tTmp_1;

			tTmp_2=tKey_2[lBegin];
			tKey_2[lBegin]=tKey_2[lEnd];
			tKey_2[lEnd]=tTmp_2;

			tTmp_3=tKey_3[lBegin];
			tKey_3[lBegin]=tKey_3[lEnd];
			tKey_3[lEnd]=tTmp_3;

			tTmp_4=tKey_4[lBegin];
			tKey_4[lBegin]=tKey_4[lEnd];
			tKey_4[lEnd]=tTmp_4;
		}
		return 0;
	}

	int32_t lMid=(lBegin+lEnd)>>1;
	int32_t m=lBegin, n=lEnd;

	T_KEY_1 tMid_1=tKey_1[lMid];
	T_KEY_2 tMid_2=tKey_2[lMid];

	while(lBegin<lEnd) {
		while(lBegin<lEnd&&Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2)<0) lBegin++;
		while(lBegin<lEnd&&Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lEnd], tMid_1, tKey_2[lEnd], tMid_2)>0) lEnd--;
		if(lBegin<lEnd) {
			tTmp_1=tKey_1[lBegin];
			tKey_1[lBegin]=tKey_1[lEnd];
			tKey_1[lEnd]=tTmp_1;

			tTmp_2=tKey_2[lBegin];
			tKey_2[lBegin]=tKey_2[lEnd];
			tKey_2[lEnd]=tTmp_2;

			tTmp_3=tKey_3[lBegin];
			tKey_3[lBegin]=tKey_3[lEnd];
			tKey_3[lEnd]=tTmp_3;

			tTmp_4=tKey_4[lBegin];
			tKey_4[lBegin]=tKey_4[lEnd];
			tKey_4[lEnd]=tTmp_4;

			if(++lBegin<lEnd)
				lEnd--;
		}
	}

	if(Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2)<0)
		lBegin++;

	if(lBegin-1>m)
		Q_Recursion_2K_2P(m, lBegin-1, tKey_1, tKey_2, tKey_3, tKey_4);
	if(lEnd<n)
		Q_Recursion_2K_2P(lEnd, n, tKey_1, tKey_2, tKey_3, tKey_4);

	return 0;
}

template <typename T_KEY_1, typename T_KEY_2, typename T_KEY_3, typename T_KEY_4>
static int32_t Q_Unrecursion_2K_2P(int32_t lBegin, int32_t lEnd, T_KEY_1* tKey_1, T_KEY_2* tKey_2, T_KEY_3* tKey_3, T_KEY_4* tKey_4)
{
	int32_t stack_buf[1<<10];
	int32_t stack_pos=0;

	stack_buf[stack_pos++]=lBegin;
	stack_buf[stack_pos++]=lEnd;

	Q_FOREVER {
		if(stack_pos==0)
			break;

		lEnd=stack_buf[--stack_pos];
		lBegin=stack_buf[--stack_pos];

		if(lBegin>=lEnd)
			continue;

		T_KEY_1 tTmp_1;
		T_KEY_2 tTmp_2;
		T_KEY_3 tTmp_3;
		T_KEY_4 tTmp_4;

		if(lBegin+1==lEnd) {
			if(Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lBegin], tKey_1[lEnd], tKey_2[lBegin], tKey_2[lEnd])>0) {
				tTmp_1=tKey_1[lBegin];
				tKey_1[lBegin]=tKey_1[lEnd];
				tKey_1[lEnd]=tTmp_1;

				tTmp_2=tKey_2[lBegin];
				tKey_2[lBegin]=tKey_2[lEnd];
				tKey_2[lEnd]=tTmp_2;

				tTmp_3=tKey_3[lBegin];
				tKey_3[lBegin]=tKey_3[lEnd];
				tKey_3[lEnd]=tTmp_3;

				tTmp_4=tKey_4[lBegin];
				tKey_4[lBegin]=tKey_4[lEnd];
				tKey_4[lEnd]=tTmp_4;
			}
			continue;
		}

		int32_t lMid=(lBegin+lEnd)>>1;
		int32_t m=lBegin, n=lEnd;

		T_KEY_1 tMid_1=tKey_1[lMid];
		T_KEY_2 tMid_2=tKey_2[lMid];

		while(lBegin<lEnd) {
			while(lBegin<lEnd&&Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2)<0) lBegin++;
			while(lBegin<lEnd&&Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lEnd], tMid_1, tKey_2[lEnd], tMid_2)>0) lEnd--;
			if(lBegin<lEnd) {
				tTmp_1=tKey_1[lBegin];
				tKey_1[lBegin]=tKey_1[lEnd];
				tKey_1[lEnd]=tTmp_1;

				tTmp_2=tKey_2[lBegin];
				tKey_2[lBegin]=tKey_2[lEnd];
				tKey_2[lEnd]=tTmp_2;

				tTmp_3=tKey_3[lBegin];
				tKey_3[lBegin]=tKey_3[lEnd];
				tKey_3[lEnd]=tTmp_3;

				tTmp_4=tKey_4[lBegin];
				tKey_4[lBegin]=tKey_4[lEnd];
				tKey_4[lEnd]=tTmp_4;

				if(++lBegin<lEnd)
					lEnd--;
			}
		}

		if(Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2)<0)
			lBegin++;

		if(lEnd<n) {
			stack_buf[stack_pos++]=lEnd;
			stack_buf[stack_pos++]=n;
		}
		if(lBegin-1>m) {
			stack_buf[stack_pos++]=m;
			stack_buf[stack_pos++]=lBegin-1;
		}
	}

	return 0;
}

template <typename T_KEY_1, typename T_KEY_2, typename T_KEY_3, typename T_KEY_4, typename T_KEY_5>
static int32_t Q_Recursion_2K_3P(int32_t lBegin, int32_t lEnd, T_KEY_1* tKey_1, T_KEY_2* tKey_2, T_KEY_3* tKey_3, T_KEY_4* tKey_4, T_KEY_5* tKey_5)
{
	if(lBegin>=lEnd)
		return 0;

	T_KEY_1 tTmp_1;
	T_KEY_2 tTmp_2;
	T_KEY_3 tTmp_3;
	T_KEY_4 tTmp_4;
	T_KEY_5 tTmp_5;

	if(lBegin+1==lEnd) {
		if(Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lBegin], tKey_1[lEnd], tKey_2[lBegin], tKey_2[lEnd])>0) {
			tTmp_1=tKey_1[lBegin];
			tKey_1[lBegin]=tKey_1[lEnd];
			tKey_1[lEnd]=tTmp_1;

			tTmp_2=tKey_2[lBegin];
			tKey_2[lBegin]=tKey_2[lEnd];
			tKey_2[lEnd]=tTmp_2;

			tTmp_3=tKey_3[lBegin];
			tKey_3[lBegin]=tKey_3[lEnd];
			tKey_3[lEnd]=tTmp_3;

			tTmp_4=tKey_4[lBegin];
			tKey_4[lBegin]=tKey_4[lEnd];
			tKey_4[lEnd]=tTmp_4;

			tTmp_5=tKey_5[lBegin];
			tKey_5[lBegin]=tKey_5[lEnd];
			tKey_5[lEnd]=tTmp_5;
		}
		return 0;
	}

	int32_t lMid=(lBegin+lEnd)>>1;
	int32_t m=lBegin, n=lEnd;

	T_KEY_1 tMid_1=tKey_1[lMid];
	T_KEY_2 tMid_2=tKey_2[lMid];

	while(lBegin<lEnd) {
		while(lBegin<lEnd&&Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2)<0) lBegin++;
		while(lBegin<lEnd&&Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lEnd], tMid_1, tKey_2[lEnd], tMid_2)>0) lEnd--;
		if(lBegin<lEnd) {
			tTmp_1=tKey_1[lBegin];
			tKey_1[lBegin]=tKey_1[lEnd];
			tKey_1[lEnd]=tTmp_1;

			tTmp_2=tKey_2[lBegin];
			tKey_2[lBegin]=tKey_2[lEnd];
			tKey_2[lEnd]=tTmp_2;

			tTmp_3=tKey_3[lBegin];
			tKey_3[lBegin]=tKey_3[lEnd];
			tKey_3[lEnd]=tTmp_3;

			tTmp_4=tKey_4[lBegin];
			tKey_4[lBegin]=tKey_4[lEnd];
			tKey_4[lEnd]=tTmp_4;

			tTmp_5=tKey_5[lBegin];
			tKey_5[lBegin]=tKey_5[lEnd];
			tKey_5[lEnd]=tTmp_5;

			if(++lBegin<lEnd)
				lEnd--;
		}
	}

	if(Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2)<0)
		lBegin++;

	if(lBegin-1>m)
		Q_Recursion_2K_3P(m, lBegin-1, tKey_1, tKey_2, tKey_3, tKey_4, tKey_5);
	if(lEnd<n)
		Q_Recursion_2K_3P(lEnd, n, tKey_1, tKey_2, tKey_3, tKey_4, tKey_5);

	return 0;
}

template <typename T_KEY_1, typename T_KEY_2, typename T_KEY_3, typename T_KEY_4, typename T_KEY_5>
static int32_t Q_Unrecursion_2K_3P(int32_t lBegin, int32_t lEnd, T_KEY_1* tKey_1, T_KEY_2* tKey_2, T_KEY_3* tKey_3, T_KEY_4* tKey_4, T_KEY_5* tKey_5)
{
	int32_t stack_buf[1<<10];
	int32_t stack_pos=0;

	stack_buf[stack_pos++]=lBegin;
	stack_buf[stack_pos++]=lEnd;

	Q_FOREVER {
		if(stack_pos==0)
			break;

		lEnd=stack_buf[--stack_pos];
		lBegin=stack_buf[--stack_pos];

		if(lBegin>=lEnd)
			continue;

		T_KEY_1 tTmp_1;
		T_KEY_2 tTmp_2;
		T_KEY_3 tTmp_3;
		T_KEY_4 tTmp_4;
		T_KEY_5 tTmp_5;

		if(lBegin+1==lEnd) {
			if(Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lBegin], tKey_1[lEnd], tKey_2[lBegin], tKey_2[lEnd])>0) {
				tTmp_1=tKey_1[lBegin];
				tKey_1[lBegin]=tKey_1[lEnd];
				tKey_1[lEnd]=tTmp_1;

				tTmp_2=tKey_2[lBegin];
				tKey_2[lBegin]=tKey_2[lEnd];
				tKey_2[lEnd]=tTmp_2;

				tTmp_3=tKey_3[lBegin];
				tKey_3[lBegin]=tKey_3[lEnd];
				tKey_3[lEnd]=tTmp_3;

				tTmp_4=tKey_4[lBegin];
				tKey_4[lBegin]=tKey_4[lEnd];
				tKey_4[lEnd]=tTmp_4;

				tTmp_5=tKey_5[lBegin];
				tKey_5[lBegin]=tKey_5[lEnd];
				tKey_5[lEnd]=tTmp_5;
			}
			continue;
		}

		int32_t lMid=(lBegin+lEnd)>>1;
		int32_t m=lBegin, n=lEnd;

		T_KEY_1 tMid_1=tKey_1[lMid];
		T_KEY_2 tMid_2=tKey_2[lMid];

		while(lBegin<lEnd) {
			while(lBegin<lEnd&&Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2)<0) lBegin++;
			while(lBegin<lEnd&&Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lEnd], tMid_1, tKey_2[lEnd], tMid_2)>0) lEnd--;
			if(lBegin<lEnd) {
				tTmp_1=tKey_1[lBegin];
				tKey_1[lBegin]=tKey_1[lEnd];
				tKey_1[lEnd]=tTmp_1;

				tTmp_2=tKey_2[lBegin];
				tKey_2[lBegin]=tKey_2[lEnd];
				tKey_2[lEnd]=tTmp_2;

				tTmp_3=tKey_3[lBegin];
				tKey_3[lBegin]=tKey_3[lEnd];
				tKey_3[lEnd]=tTmp_3;

				tTmp_4=tKey_4[lBegin];
				tKey_4[lBegin]=tKey_4[lEnd];
				tKey_4[lEnd]=tTmp_4;

				tTmp_5=tKey_5[lBegin];
				tKey_5[lBegin]=tKey_5[lEnd];
				tKey_5[lEnd]=tTmp_5;

				if(++lBegin<lEnd)
					lEnd--;
			}
		}

		if(Compare_2K<T_KEY_1, T_KEY_2>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2)<0)
			lBegin++;

		if(lEnd<n) {
			stack_buf[stack_pos++]=lEnd;
			stack_buf[stack_pos++]=n;
		}
		if(lBegin-1>m) {
			stack_buf[stack_pos++]=m;
			stack_buf[stack_pos++]=lBegin-1;
		}
	}

	return 0;
}

template <typename T_KEY_1, typename T_KEY_2, typename T_KEY_3, typename T_KEY_4>
static int32_t Q_Recursion_3K_1P(int32_t lBegin, int32_t lEnd, T_KEY_1* tKey_1, T_KEY_2* tKey_2, T_KEY_3* tKey_3, T_KEY_4* tKey_4)
{
	if(lBegin>=lEnd)
		return 0;

	T_KEY_1 tTmp_1;
	T_KEY_2 tTmp_2;
	T_KEY_3 tTmp_3;
	T_KEY_4 tTmp_4;

	if(lBegin+1==lEnd) {
		if(Compare_3K<T_KEY_1, T_KEY_2, T_KEY_3>(tKey_1[lBegin], tKey_1[lEnd], tKey_2[lBegin], tKey_2[lEnd], tKey_3[lBegin], tKey_3[lEnd])>0) {
			tTmp_1=tKey_1[lBegin];
			tKey_1[lBegin]=tKey_1[lEnd];
			tKey_1[lEnd]=tTmp_1;

			tTmp_2=tKey_2[lBegin];
			tKey_2[lBegin]=tKey_2[lEnd];
			tKey_2[lEnd]=tTmp_2;

			tTmp_3=tKey_3[lBegin];
			tKey_3[lBegin]=tKey_3[lEnd];
			tKey_3[lEnd]=tTmp_3;

			tTmp_4=tKey_4[lBegin];
			tKey_4[lBegin]=tKey_4[lEnd];
			tKey_4[lEnd]=tTmp_4;
		}
		return 0;
	}

	int32_t lMid=(lBegin+lEnd)>>1;
	int32_t m=lBegin, n=lEnd;

	T_KEY_1 tMid_1=tKey_1[lMid];
	T_KEY_2 tMid_2=tKey_2[lMid];
	T_KEY_3 tMid_3=tKey_3[lMid];

	while(lBegin<lEnd) {
		while(lBegin<lEnd&&Compare_3K<T_KEY_1, T_KEY_2, T_KEY_3>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2, tKey_3[lBegin], tMid_3)<0) lBegin++;
		while(lBegin<lEnd&&Compare_3K<T_KEY_1, T_KEY_2, T_KEY_3>(tKey_1[lEnd], tMid_1, tKey_2[lEnd], tMid_2, tKey_3[lEnd], tMid_3)>0) lEnd--;
		if(lBegin<lEnd) {
			tTmp_1=tKey_1[lBegin];
			tKey_1[lBegin]=tKey_1[lEnd];
			tKey_1[lEnd]=tTmp_1;

			tTmp_2=tKey_2[lBegin];
			tKey_2[lBegin]=tKey_2[lEnd];
			tKey_2[lEnd]=tTmp_2;

			tTmp_3=tKey_3[lBegin];
			tKey_3[lBegin]=tKey_3[lEnd];
			tKey_3[lEnd]=tTmp_3;

			tTmp_4=tKey_4[lBegin];
			tKey_4[lBegin]=tKey_4[lEnd];
			tKey_4[lEnd]=tTmp_4;

			if(++lBegin<lEnd)
				lEnd--;
		}
	}

	if(Compare_3K<T_KEY_1, T_KEY_2, T_KEY_3>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2, tKey_3[lBegin], tMid_3)<0)
		lBegin++;

	if(lBegin-1>m)
		Q_Recursion_3K_1P(m, lBegin-1, tKey_1, tKey_2, tKey_3, tKey_4);
	if(lEnd<n)
		Q_Recursion_3K_1P(lEnd, n, tKey_1, tKey_2, tKey_3, tKey_4);

	return 0;
}

template <typename T_KEY_1, typename T_KEY_2, typename T_KEY_3, typename T_KEY_4>
static int32_t Q_Unrecursion_3K_1P(int32_t lBegin, int32_t lEnd, T_KEY_1* tKey_1, T_KEY_2* tKey_2, T_KEY_3* tKey_3, T_KEY_4* tKey_4)
{
	int32_t stack_buf[1<<10];
	int32_t stack_pos=0;

	stack_buf[stack_pos++]=lBegin;
	stack_buf[stack_pos++]=lEnd;

	Q_FOREVER {
		if(stack_pos==0)
			break;

		lEnd=stack_buf[--stack_pos];
		lBegin=stack_buf[--stack_pos];

		if(lBegin>=lEnd)
			continue;

		T_KEY_1 tTmp_1;
		T_KEY_2 tTmp_2;
		T_KEY_3 tTmp_3;
		T_KEY_4 tTmp_4;

		if(lBegin+1==lEnd) {
			if(Compare_3K<T_KEY_1, T_KEY_2, T_KEY_3>(tKey_1[lBegin], tKey_1[lEnd], tKey_2[lBegin], tKey_2[lEnd], tKey_3[lBegin], tKey_3[lEnd])>0) {
				tTmp_1=tKey_1[lBegin];
				tKey_1[lBegin]=tKey_1[lEnd];
				tKey_1[lEnd]=tTmp_1;

				tTmp_2=tKey_2[lBegin];
				tKey_2[lBegin]=tKey_2[lEnd];
				tKey_2[lEnd]=tTmp_2;

				tTmp_3=tKey_3[lBegin];
				tKey_3[lBegin]=tKey_3[lEnd];
				tKey_3[lEnd]=tTmp_3;

				tTmp_4=tKey_4[lBegin];
				tKey_4[lBegin]=tKey_4[lEnd];
				tKey_4[lEnd]=tTmp_4;
			}
			continue;
		}

		int32_t lMid=(lBegin+lEnd)>>1;
		int32_t m=lBegin, n=lEnd;

		T_KEY_1 tMid_1=tKey_1[lMid];
		T_KEY_2 tMid_2=tKey_2[lMid];
		T_KEY_3 tMid_3=tKey_3[lMid];

		while(lBegin<lEnd) {
			while(lBegin<lEnd&&Compare_3K<T_KEY_1, T_KEY_2, T_KEY_3>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2, tKey_3[lBegin], tMid_3)<0) lBegin++;
			while(lBegin<lEnd&&Compare_3K<T_KEY_1, T_KEY_2, T_KEY_3>(tKey_1[lEnd], tMid_1, tKey_2[lEnd], tMid_2, tKey_3[lEnd], tMid_3)>0) lEnd--;
			if(lBegin<lEnd) {
				tTmp_1=tKey_1[lBegin];
				tKey_1[lBegin]=tKey_1[lEnd];
				tKey_1[lEnd]=tTmp_1;

				tTmp_2=tKey_2[lBegin];
				tKey_2[lBegin]=tKey_2[lEnd];
				tKey_2[lEnd]=tTmp_2;

				tTmp_3=tKey_3[lBegin];
				tKey_3[lBegin]=tKey_3[lEnd];
				tKey_3[lEnd]=tTmp_3;

				tTmp_4=tKey_4[lBegin];
				tKey_4[lBegin]=tKey_4[lEnd];
				tKey_4[lEnd]=tTmp_4;

				if(++lBegin<lEnd)
					lEnd--;
			}
		}

		if(Compare_3K<T_KEY_1, T_KEY_2, T_KEY_3>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2, tKey_3[lBegin], tMid_3)<0)
			lBegin++;

		if(lEnd<n) {
			stack_buf[stack_pos++]=lEnd;
			stack_buf[stack_pos++]=n;
		}
		if(lBegin-1>m) {
			stack_buf[stack_pos++]=m;
			stack_buf[stack_pos++]=lBegin-1;
		}
	}

	return 0;
}

template <typename T_KEY_1, typename T_KEY_2, typename T_KEY_3, typename T_KEY_4, typename T_KEY_5>
static int32_t Q_Recursion_3K_2P(int32_t lBegin, int32_t lEnd, T_KEY_1* tKey_1, T_KEY_2* tKey_2, T_KEY_3* tKey_3, T_KEY_4* tKey_4, T_KEY_5* tKey_5)
{
	if(lBegin>=lEnd)
		return 0;

	T_KEY_1 tTmp_1;
	T_KEY_2 tTmp_2;
	T_KEY_3 tTmp_3;
	T_KEY_4 tTmp_4;
	T_KEY_5 tTmp_5;

	if(lBegin+1==lEnd) {
		if(Compare_3K<T_KEY_1, T_KEY_2, T_KEY_3>(tKey_1[lBegin], tKey_1[lEnd], tKey_2[lBegin], tKey_2[lEnd], tKey_3[lBegin], tKey_3[lEnd])>0) {
			tTmp_1=tKey_1[lBegin];
			tKey_1[lBegin]=tKey_1[lEnd];
			tKey_1[lEnd]=tTmp_1;

			tTmp_2=tKey_2[lBegin];
			tKey_2[lBegin]=tKey_2[lEnd];
			tKey_2[lEnd]=tTmp_2;

			tTmp_3=tKey_3[lBegin];
			tKey_3[lBegin]=tKey_3[lEnd];
			tKey_3[lEnd]=tTmp_3;

			tTmp_4=tKey_4[lBegin];
			tKey_4[lBegin]=tKey_4[lEnd];
			tKey_4[lEnd]=tTmp_4;

			tTmp_5=tKey_5[lBegin];
			tKey_5[lBegin]=tKey_5[lEnd];
			tKey_5[lEnd]=tTmp_5;
		}
		return 0;
	}

	int32_t lMid=(lBegin+lEnd)>>1;
	int32_t m=lBegin, n=lEnd;

	T_KEY_1 tMid_1=tKey_1[lMid];
	T_KEY_2 tMid_2=tKey_2[lMid];
	T_KEY_3 tMid_3=tKey_3[lMid];

	while(lBegin<lEnd) {
		while(lBegin<lEnd&&Compare_3K<T_KEY_1, T_KEY_2, T_KEY_3>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2, tKey_3[lBegin], tMid_3)<0) lBegin++;
		while(lBegin<lEnd&&Compare_3K<T_KEY_1, T_KEY_2, T_KEY_3>(tKey_1[lEnd], tMid_1, tKey_2[lEnd], tMid_2, tKey_3[lEnd], tMid_3)>0) lEnd--;
		if(lBegin<lEnd) {
			tTmp_1=tKey_1[lBegin];
			tKey_1[lBegin]=tKey_1[lEnd];
			tKey_1[lEnd]=tTmp_1;

			tTmp_2=tKey_2[lBegin];
			tKey_2[lBegin]=tKey_2[lEnd];
			tKey_2[lEnd]=tTmp_2;

			tTmp_3=tKey_3[lBegin];
			tKey_3[lBegin]=tKey_3[lEnd];
			tKey_3[lEnd]=tTmp_3;

			tTmp_4=tKey_4[lBegin];
			tKey_4[lBegin]=tKey_4[lEnd];
			tKey_4[lEnd]=tTmp_4;

			tTmp_5=tKey_5[lBegin];
			tKey_5[lBegin]=tKey_5[lEnd];
			tKey_5[lEnd]=tTmp_5;

			if(++lBegin<lEnd)
				lEnd--;
		}
	}

	if(Compare_3K<T_KEY_1, T_KEY_2, T_KEY_3>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2, tKey_3[lBegin], tMid_3)<0)
		lBegin++;

	if(lBegin-1>m)
		Q_Recursion_3K_2P(m, lBegin-1, tKey_1, tKey_2, tKey_3, tKey_4, tKey_5);
	if(lEnd<n)
		Q_Recursion_3K_2P(lEnd, n, tKey_1, tKey_2, tKey_3, tKey_4, tKey_5);

	return 0;
}

template <typename T_KEY_1, typename T_KEY_2, typename T_KEY_3, typename T_KEY_4, typename T_KEY_5>
static int32_t Q_Unrecursion_3K_2P(int32_t lBegin, int32_t lEnd, T_KEY_1* tKey_1, T_KEY_2* tKey_2, T_KEY_3* tKey_3, T_KEY_4* tKey_4, T_KEY_5* tKey_5)
{
	int32_t stack_buf[1<<10];
	int32_t stack_pos=0;

	stack_buf[stack_pos++]=lBegin;
	stack_buf[stack_pos++]=lEnd;

	Q_FOREVER {
		if(stack_pos==0)
			break;

		lEnd=stack_buf[--stack_pos];
		lBegin=stack_buf[--stack_pos];

		if(lBegin>=lEnd)
			continue;

		T_KEY_1 tTmp_1;
		T_KEY_2 tTmp_2;
		T_KEY_3 tTmp_3;
		T_KEY_4 tTmp_4;
		T_KEY_5 tTmp_5;

		if(lBegin+1==lEnd) {
			if(Compare_3K<T_KEY_1, T_KEY_2, T_KEY_3>(tKey_1[lBegin], tKey_1[lEnd], tKey_2[lBegin], tKey_2[lEnd], tKey_3[lBegin], tKey_3[lEnd])>0) {
				tTmp_1=tKey_1[lBegin];
				tKey_1[lBegin]=tKey_1[lEnd];
				tKey_1[lEnd]=tTmp_1;

				tTmp_2=tKey_2[lBegin];
				tKey_2[lBegin]=tKey_2[lEnd];
				tKey_2[lEnd]=tTmp_2;

				tTmp_3=tKey_3[lBegin];
				tKey_3[lBegin]=tKey_3[lEnd];
				tKey_3[lEnd]=tTmp_3;

				tTmp_4=tKey_4[lBegin];
				tKey_4[lBegin]=tKey_4[lEnd];
				tKey_4[lEnd]=tTmp_4;

				tTmp_5=tKey_5[lBegin];
				tKey_5[lBegin]=tKey_5[lEnd];
				tKey_5[lEnd]=tTmp_5;
			}
			continue;
		}

		int32_t lMid=(lBegin+lEnd)>>1;
		int32_t m=lBegin, n=lEnd;

		T_KEY_1 tMid_1=tKey_1[lMid];
		T_KEY_2 tMid_2=tKey_2[lMid];
		T_KEY_3 tMid_3=tKey_3[lMid];

		while(lBegin<lEnd) {
			while(lBegin<lEnd&&Compare_3K<T_KEY_1, T_KEY_2, T_KEY_3>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2, tKey_3[lBegin], tMid_3)<0) lBegin++;
			while(lBegin<lEnd&&Compare_3K<T_KEY_1, T_KEY_2, T_KEY_3>(tKey_1[lEnd], tMid_1, tKey_2[lEnd], tMid_2, tKey_3[lEnd], tMid_3)>0) lEnd--;
			if(lBegin<lEnd) {
				tTmp_1=tKey_1[lBegin];
				tKey_1[lBegin]=tKey_1[lEnd];
				tKey_1[lEnd]=tTmp_1;

				tTmp_2=tKey_2[lBegin];
				tKey_2[lBegin]=tKey_2[lEnd];
				tKey_2[lEnd]=tTmp_2;

				tTmp_3=tKey_3[lBegin];
				tKey_3[lBegin]=tKey_3[lEnd];
				tKey_3[lEnd]=tTmp_3;

				tTmp_4=tKey_4[lBegin];
				tKey_4[lBegin]=tKey_4[lEnd];
				tKey_4[lEnd]=tTmp_4;

				tTmp_5=tKey_5[lBegin];
				tKey_5[lBegin]=tKey_5[lEnd];
				tKey_5[lEnd]=tTmp_5;

				if(++lBegin<lEnd)
					lEnd--;
			}
		}

		if(Compare_3K<T_KEY_1, T_KEY_2, T_KEY_3>(tKey_1[lBegin], tMid_1, tKey_2[lBegin], tMid_2, tKey_3[lBegin], tMid_3)<0)
			lBegin++;

		if(lEnd<n) {
			stack_buf[stack_pos++]=lEnd;
			stack_buf[stack_pos++]=n;
		}
		if(lBegin-1>m) {
			stack_buf[stack_pos++]=m;
			stack_buf[stack_pos++]=lBegin-1;
		}
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 并行快排
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T_KEY>
class QQSParallel {
	public:
		typedef struct __org_info {
			QMutexLock lock;
			int32_t flag;
			T_KEY* key;
		} ORG_INFO;

		typedef struct __sort_info {
			ORG_INFO* org;
			int32_t beg;
			int32_t end;
		} SORT_INFO;

	public:
		QQSParallel() :
			m_threshold_normal(0),
			m_list_size(0),
			m_thread_num(0)
		{}

		virtual ~QQSParallel()
		{}

		int32_t init(int32_t threshold_normal=1000, int32_t list_size=100000, int32_t thread_num=10)
		{
			m_threshold_normal=threshold_normal;
			m_list_size=list_size;
			m_thread_num=thread_num;

			if(m_idle_list.init(m_list_size))
				return -1;
			if(m_busy_list.init(m_list_size))
				return -2;

			SORT_INFO* node_list=q_new_array<SORT_INFO>(m_list_size);
			if(node_list==NULL)
				return -3;

			for(int32_t i=0; i<m_list_size; ++i)
				m_idle_list.push(node_list+i);

			for(int32_t i=0; i<m_thread_num; ++i)
				q_create_thread(thread_sort, this);

			return 0;
		}

		int32_t quick_sort(int32_t begin, int32_t end, T_KEY* key)
		{
			ORG_INFO org_info;
			org_info.key=key;
			org_info.flag=1;

			SORT_INFO* node=m_idle_list.pop();
			node->beg=begin;
			node->end=end;
			node->org=&org_info;

			node->org->flag++;
			m_busy_list.push(node);

			while(org_info.flag)
				q_sleep(1);

			return 0;
		}

	private:
		static Q_THREAD_T thread_sort(void* argv)
		{
			QQSParallel<T_KEY>* cc=reinterpret_cast< QQSParallel<T_KEY>* >(argv);

			Q_FOREVER {
				SORT_INFO* node=cc->m_busy_list.pop();

				T_KEY* tKey=node->org->key;
				int32_t lBegin=node->beg;
				int32_t lEnd=node->end;

				if(lEnd-lBegin<cc->m_threshold_normal) {
					Q_Unrecursion_1K<T_KEY>(lBegin, lEnd, tKey);
					node->org->lock.lock();
					node->org->flag-=2;
					node->org->lock.unlock();
					cc->m_idle_list.push(node);
					continue;
				}

				int32_t lMid=(lBegin+lEnd)>>1;
				int32_t m=lBegin;
				int32_t n=lEnd;
				T_KEY tMid=tKey[lMid];

				while(lBegin<lEnd) {
					while(lBegin<lEnd&&tKey[lBegin]<tMid) lBegin++;
					while(lBegin<lEnd&&tKey[lEnd]>tMid) lEnd--;
					if(lBegin<lEnd) {
						T_KEY tTmp=tKey[lBegin];
						tKey[lBegin]=tKey[lEnd];
						tKey[lEnd]=tTmp;

						if(++lBegin<lEnd)
							lEnd--;
					}
				}

				if(tKey[lBegin]<tMid)
					lBegin++;

				SORT_INFO* new_node=cc->m_idle_list.pop();
				new_node->beg=lEnd;
				new_node->end=n;
				new_node->org=node->org;

				new_node->org->lock.lock();
				new_node->org->flag++;
				new_node->org->lock.unlock();
				cc->m_busy_list.push(new_node);

				new_node=cc->m_idle_list.pop();
				new_node->beg=m;
				new_node->end=lBegin-1;
				new_node->org=node->org;

				new_node->org->lock.lock();
				new_node->org->flag++;
				new_node->org->lock.unlock();
				cc->m_busy_list.push(new_node);

				cc->m_idle_list.push(node);
			}

			return NULL;
		}

	protected:
		QQueue<SORT_INFO*> m_idle_list;
		QQueue<SORT_INFO*> m_busy_list;

		int32_t m_threshold_normal;
		int32_t m_list_size;
		int32_t m_thread_num;
};

Q_END_NAMESPACE

#endif // __QALGORITHM_H_
