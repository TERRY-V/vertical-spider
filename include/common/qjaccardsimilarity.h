/********************************************************************************************
**
** Copyright (C) 2010-2015 Terry Niu (Beijing, China)
** Filename:	qjaccardsimilarity.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/04/01
**
*********************************************************************************************/

#ifndef __QJACCARDSIMILARITY_H_
#define __QJACCARDSIMILARITY_H_

#include "qglobal.h"
#include "qmd5.h"

Q_BEGIN_NAMESPACE

class QJaccardSimilarity {
	public:
		/*
		 * 文档的Shingling算法
		 *
		 * 将k-shingle算法得到的字符串哈希，映射成桶编号，并将得桶编号看成最终的shingle
		 * 如此不仅将数据得到了压缩，并且可以对哈希后的整数进行单字机器运算
		 */
		static std::list<uint64_t> kHashShingle(const std::string& ss, uint32_t k, bool tag=true)
		{
			std::list<uint64_t> shingle_lst;
			std::list<uint64_t>::iterator shingle_lst_iter;
			std::string elem;

			QMD5 md5;
			uint64_t md5sum;

			for(uint32_t pos=0; pos!=ss.length(); ++pos)
			{
				if(pos+k>ss.length())
					break;

				elem=ss.substr(pos, k);
				md5sum=md5.MD5Bits64((unsigned char*)elem.c_str(), elem.length());

				if(tag) {
					shingle_lst_iter=find(shingle_lst.begin(), shingle_lst.end(), md5sum);
					if(shingle_lst_iter==shingle_lst.end())
						shingle_lst.push_back(md5sum);
				} else {
					shingle_lst.push_back(md5sum);
				}
			}

			return shingle_lst;
		}

		/*
		 * 计算两个集合的Jaccard相似度
		 *
		 * 在实际应用环境中，可使用Jaccard方法的一个有趣变形，先分词并忽略掉停用词
		 * Jaccard距离=1-Jaccard相似度
		 */
		static double calc(const std::list<uint64_t>& list1, const std::list<uint64_t>& list2)
		{
			double similarity(0);
			uint32_t all_num=0;
			uint32_t distinct_num=0;

			std::list<uint64_t> ttList(list1.begin(), list1.end());
			ttList.insert(ttList.end(), list2.begin(), list2.end());

			all_num=ttList.size();

			ttList.sort();

			ttList.erase(unique(ttList.begin(), ttList.end()), ttList.end());
			distinct_num=ttList.size();

			if(distinct_num)
				similarity=(all_num-distinct_num)/static_cast<double>(distinct_num);

			return similarity;
		}

	private:
		QJaccardSimilarity();
		virtual ~QJaccardSimilarity();
};

Q_END_NAMESPACE

#endif // __QJACCARDSIMILARITY_H_
