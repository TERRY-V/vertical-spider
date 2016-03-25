/********************************************************************************************
 **
 ** Copyright (C) 2010-2015 Terry Niu (Beijing, China)
 ** Filename:	qdistance.h
 ** Author:	TERRY-V
 ** Email:	cnbj8607@163.com
 ** Support:	http://blog.sina.com.cn/terrynotes
 ** Date:	2014/04/07
 **
 *********************************************************************************************/

#ifndef __QDISTANCE_H_
#define __QDISTANCE_H_

#include <unordered_map>
#include "qglobal.h"

Q_BEGIN_NAMESPACE

class QDistance {
	public:
		/*
		 * 在有纬度的空间下余弦距离才有意义，这些空间包括欧式空间及离散欧式空间。
		 * 两个点的余弦距离实际上是点所代表的向量之间的夹角，
		 * 不管空间有多少维，该夹角的范围是0~180度。
		 *
		 * c1和c2存储的是两个向量
		 * 其夹角余弦等于它们的内积除以两个向量的L2范式(即它们到原点的欧氏距离)乘积。
		 */
		static double calCosineDistance(std::unordered_map<std::string, double>& c1, std::unordered_map<std::string, double>& c2)
		{
			std::set<std::string> words;
			double t1=0, t2=0;
			// 计算两个向量的L2范式，注意此处还没有开根号
			for(auto elem : c1) {
				words.insert(elem.first);
				t1+=pow(elem.second, 2);
			}
			for(auto elem : c2) {
				words.insert(elem.first);
				t2+=pow(elem.second, 2);
			}
			// 计算两个向量的内积
			double innerProduct=0;
			for(std::string word : words) {
				auto iter1=c1.find(word);
				double prob1=(iter1==c1.end())?0:iter1->second;
				auto iter2=c2.find(word);
				double prob2=(iter2==c2.end())?0:iter2->second;
				innerProduct+=prob1*prob2;
			}
			// 计算两个向量夹角的余弦
			double tt=sqrt(t1)*sqrt(t2);
			double cosineDist=(tt==0)?0:innerProduct/tt;
			return cosineDist;
		}

		/*
		 * 欧氏距离是最为人熟知的距离测度
		 * 在n维欧式空间中，每个点就是一个n维实数向量
		 *
		 * L1范式距离
		 * d([x1, x2, ..., xn], [y1, y2, ..., yn])=sum(1->n)(|xi-yi|);
		 */
		static double calL1Norm(std::unordered_map<std::string, double>& c1, std::unordered_map<std::string, double>& c2)
		{
			std::set<std::string> words;
			for(auto elem : c1) words.insert(elem.first);
			for(auto elem : c2) words.insert(elem.first);
			double edist=0;
			for(std::string word : words) {
				auto iter1=c1.find(word);
				double prob1=(iter1==c1.end())?0:iter1->second;
				auto iter2=c2.find(word);
				double prob2=(iter2==c2.end())?0:iter2->second;
				edist+=fabs(prob1-prob2);
			}
			return edist;
		}

		/*
		 * L2范式距离
		 * d([x1, x2, ..., xn], [y1, y2, ..., yn])=sqrt(sum(1->n)((xi-yi)^2));
		 */
		static double calL2Norm(std::unordered_map<std::string, double>& c1, std::unordered_map<std::string, double>& c2)
		{
			std::set<std::string> words;
			for(auto elem : c1) words.insert(elem.first);
			for(auto elem : c2) words.insert(elem.first);
			double edist=0;
			for(std::string word : words) {
				auto iter1=c1.find(word);
				double prob1=(iter1==c1.end())?0:iter1->second;
				auto iter2=c2.find(word);
				double prob2=(iter2==c2.end())?0:iter2->second;
				edist+=pow(prob1-prob2, 2);
			}
			return sqrt(edist);
		}

		/*
		 * L无穷范式距离
		 * d([x1, x2, ..., xn], [y1, y2, ..., yn])=max(1->n)(|xi-yi|);
		 */
		static double calLInfiniteNorm(std::unordered_map<std::string, double>& c1, std::unordered_map<std::string, double>& c2)
		{
			std::set<std::string> words;
			for(auto elem : c1) words.insert(elem.first);
			for(auto elem : c2) words.insert(elem.first);
			double edist=0;
			for(std::string word : words) {
				auto iter1=c1.find(word);
				double prob1=(iter1==c1.end())?0:iter1->second;
				auto iter2=c2.find(word);
				double prob2=(iter2==c2.end())?0:iter2->second;
				double temp=fabs(prob1-prob2);
				if(temp>edist) edist=temp;
			}
			return edist;
		}

		/* 
		 * 最小编辑距离
		 */
		static inline int32_t calMinEditDistance(char* str1, int32_t str1_len, char* str2, int32_t str2_len)
		{
			if(str1_len==0||str2_len==0)
				return str1_len+str2_len;

			if(str1[0]==str2[0])
				return calMinEditDistance(str1+1, str1_len-1, str2+1, str2_len-1);

			int32_t dis1=calMinEditDistance(str1+1, str1_len-1, str2+1, str2_len-1);
			int32_t dis2=calMinEditDistance(str1, str1_len, str2+1, str2_len-1);
			int32_t dis3=calMinEditDistance(str1+1, str1_len-1, str2, str2_len);

			dis1=dis1<dis2?dis1:dis2;
			dis1=dis1<dis3?dis1:dis3;

			return dis1+1;
		}

	private:
		QDistance();
		virtual ~QDistance();
};

Q_END_NAMESPACE

#endif // __QDISTANCE_H_
