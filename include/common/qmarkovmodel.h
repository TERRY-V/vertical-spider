/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qmarkovmodel.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2012/11/06
**
*********************************************************************************************/

#ifndef __QMARKOVMODEL_H_
#define __QMARKOVMODEL_H_

#include "qglobal.h"

Q_BEGIN_NAMESPACE

// 马尔可夫模型(一阶)
class QMarkovModel {
	public:
		// N个状态的一阶马尔可夫过程有N^2次状态转移, 状态转移概率可表示成一个状态转移矩阵。
		// 请参考宗成庆<<统计自然语言处理>>第二版109页

		// @函数名: 马尔可夫模型构造函数
		QMarkovModel()
		{
			pszStateTransitionMatrix=NULL;
			m_iStateNum=0;
		}

		// @函数名: 析构函数
		virtual ~QMarkovModel()
		{
			if(pszStateTransitionMatrix!=NULL) {
				for(int32_t i=0; i!=m_iStateNum; ++i) {
					delete [] pszStateTransitionMatrix[i];
					pszStateTransitionMatrix[i]=NULL;
				}
				delete [] pszStateTransitionMatrix;
				pszStateTransitionMatrix=NULL;
			}
		}

		// @函数名: 初始化函数, 初始化概率矩阵
		// @参数01: 二维概率数组
		// @参数02: 有限状态个数, S={s(1), s(2), s(3), ..., s(n)}
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t init(double* pszProbability, int32_t iStateNum)
		{
			// 检查传入的参数值
			if(pszProbability==NULL||iStateNum<=0)
				return -1;

			// 创建状态转移矩阵的行
			m_iStateNum=iStateNum;
			pszStateTransitionMatrix=new(std::nothrow) double*[m_iStateNum];
			if(pszStateTransitionMatrix==NULL)
				return -2;

			// 创建状态转移矩阵的列
			for(int32_t i=0; i!=m_iStateNum; ++i) {
				pszStateTransitionMatrix[i]=new(std::nothrow) double[m_iStateNum];
				if(pszStateTransitionMatrix[i]==NULL)
					return -3;
				// 状态转移矩阵赋值
				for(int32_t j=0; j<m_iStateNum; ++j)
					pszStateTransitionMatrix[i][j]=*(pszProbability+i*m_iStateNum+j);
			}

			return 0;
		}

		// @函数名: 输出状态转移矩阵
		void printStateTransitionMatrix()
		{
			Q_INFO("State Transition Matrix:");
			for(int32_t i=0; i!=m_iStateNum; ++i) {
				printf("S%d:\t", i);
				for(int32_t j=0; j<m_iStateNum; ++j)
					printf("%lf\t", pszStateTransitionMatrix[i][j]);
				printf("\n");
			}
		}

		// @函数名: 计算序列的出现概率
		// @参数01: 状态序列, 首个状态是确定的
		// @参数02: 状态数量
		// @返回值: 成功返回序列的概率, 失败返回<0的错误码
		double getSequenceProbability(int32_t* pszSequence, int32_t iSequenceNum)
		{
			// 检查传入的参数值
			if(pszSequence==NULL||iSequenceNum<=0)
				return -1;

			// 初始化为1, 方便做乘法运算
			double probability=1;
			int32_t prevState=0;
			int32_t curState=0;

			// 为什么从1开始计数呢? 因为首个状态是确定的, 概率为1
			for(int32_t i=1; i!=iSequenceNum; ++i) {
				// 前一状态
				prevState=pszSequence[i-1];
				// 当前状态
				curState=pszSequence[i];
				// 状态转移概率累乘
				probability*=pszStateTransitionMatrix[prevState][curState];
			}

			return probability;
		}

	private:
		// 状态转移矩阵
		double** pszStateTransitionMatrix;
		// 有限状态数量
		int32_t m_iStateNum;
};

Q_END_NAMESPACE

#endif // __QMARKOVMODEL_H_
