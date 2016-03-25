/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qneologismrecognizer.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2013/11/05
**
*********************************************************************************************/

#ifndef __QNEOLOGISMRECOGNIZER_H_
#define __QNEOLOGISMRECOGNIZER_H_

#include "qglobal.h"
#include "qfunc.h"
#include "qhashmap.h"
#include "qstring.h"

Q_BEGIN_NAMESPACE

// 基于head-middle-tail结构的构词模式
// 该构词模式视未登录词识别是词语边界识别的后处理过程
// 对于任一汉字, 它在构成多字词时有如下3种模式:
// (1): 词首(head) c是多字词的首字, 以H表示
// (2): 词尾(tail) c是多字词的尾字, 以T表示
// (3): 词中(middle) c是多字词的中部, 以M表示

// 未登录词识别算法类
class QNeologismRecognizer {
	public:
#pragma pack(1)
		struct dictInfo {
			uint64_t magicNumber;
			uint64_t libVersion;
			uint64_t author;
			uint32_t totalNum;
		};

		struct recordHeader {
			int32_t id;
			uint16_t qchar;
			double headPbl;
			double tailPbl;
			double middlePbl;
		};

		struct htmInfo {
			int32_t headNum;
			int32_t tailNum;
			int32_t middleNum;

			// 构造函数, 初始化为1为平滑所使用
			htmInfo(int32_t iHeadNum=1, int32_t iTailNum=1, int32_t iMiddleNum=1) {
				headNum=iHeadNum;
				tailNum=iTailNum;
				middleNum=iMiddleNum;
			}
		};
#pragma pack()

	public:
		// @函数名: 未登录词识别算法类构造函数
		QNeologismRecognizer()
		{
			pURHashTable=NULL;
			fpR=NULL;
		}

		// @函数名: 未登录词识别算法类析构函数
		virtual ~QNeologismRecognizer()
		{
			if(pURHashTable!=NULL) {
				delete pURHashTable;
				pURHashTable=NULL;
			}
			if(fpR!=NULL) {
				fclose(fpR);
				fpR=NULL;
			}
		}

		// @函数名: 初始化函数
		// @参数01: 读取HTM概率信息数据
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t init(const char* pszFileName)
		{
			// 检查传入的参数值
			if(pszFileName==NULL)
				return -1;

			// 构建哈希表
			pURHashTable=new(std::nothrow) QLinearHash<uint16_t, recordHeader>(10000);
			if(pURHashTable==NULL)
				return -2;

			// 文件头信息
			char* pszHeadBuffer=new(std::nothrow) char[BUFSIZ_256];
			if(pszHeadBuffer==NULL)
				return -3;

			// 打开数据资源文件
			fpR=fopen(pszFileName, "rb");
			if(fpR==NULL)
				return -5;

			// 读取数据资源
			int32_t ret=0;
			try {
				// 读取文件头信息
				if(fread(pszHeadBuffer, BUFSIZ_256, 1, fpR)!=1)
					throw -8;
				dictInfo* pki=(dictInfo*)pszHeadBuffer;

				// 检查文件类型是否正确
				if(pki->magicNumber!=0x123456789) {
					Q_DEBUG("File is not recognized by this application");
					throw -9;
				}

				// 检查文件版本号
				if(pki->libVersion<*(uint64_t*)"14.08.08") {
					Q_DEBUG("Version is invalid: (%.*s)", 8, (char *)pki->libVersion);
					throw -10;
				}

				// 检查作者姓名是否正确
				if(pki->author!=0x67975c0f725bfeff) {
					Q_DEBUG("Author is not correct, open failure");
					throw -11;
				}

				// 检查总记录数是否正确
				if(pki->totalNum<0) {
					Q_DEBUG("Invalid record number, totalNum = (%d)", pki->totalNum);
					throw -12;
				}

				// 读取文件记录信息
				recordHeader st_ri;
				for(int32_t i=1; i<=(int32_t)pki->totalNum; ++i) {
					// 读取记录头信息
					if(fread(&st_ri, sizeof(recordHeader), 1, fpR)!=1)
						throw -13;

					// 检查记录ID是否一致
					if((int32_t)st_ri.id!=i) {
						Q_DEBUG("Record ID error: st_ri.id (%d) != i (%d)", st_ri.id, i);
						throw -14;
					}

					// 将该记录写入哈希表
					if(!pURHashTable->push(st_ri.qchar, st_ri.qchar, st_ri)) {
						Q_DEBUG("Duplicate hash key, st_ri.qchar = (%d)", st_ri.qchar);
						throw -15;
					}
				}

				// 读取文件结束标识
				uint64_t end_mark=0;
				if(fread(&end_mark, sizeof(uint64_t), 1, fpR)!=1)
					throw -21;

				// 判断文件结束标识是否正确
				if(end_mark!=*(uint64_t *)"@#@#@#@#")
					throw -22;

				// 输出文件总记录数
				// Q_DEBUG("Loaded total (%d) records......", pURHashTable->size());
			} catch(int32_t err) {
				ret=err;
			}

			// 释放申请的内存空间
			delete [] pszHeadBuffer;
			pszHeadBuffer=NULL;
			return ret;
		}

		// @函数名: 从训练数据中统计构词能力
		// @参数01: 训练数据文件路径名
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t train(const char* pszFileName)
		{
			if(pszFileName==NULL)
				return -1;

			// 读取文件数据内容
			QString qss;
			qss=qss.readAll(pszFileName);

			// 数据统计
			std::map<uint16_t, htmInfo> countMap;
			std::list<QString> q_lines=qss.line_tokenize();
			int32_t wordLength=0;
			for(std::list<QString>::iterator it=q_lines.begin(); it!=q_lines.end(); ++it) {
				if(it->startsWith("$")) {
					wordLength=it->indexOf("<<", 2);
					// 过滤掉单字
					if(wordLength<=2)
						continue;

					// 统计多字词中的字在词首词中词尾出现的次数
					for(int32_t i=1; i!=wordLength; ++i) {
						if(countMap.find(it->at(i).unicode())!=countMap.end()) {
							if(i==1) countMap[it->at(i).unicode()].headNum++;
							else if(i==it->length()-1) countMap[it->at(i).unicode()].tailNum++;
							else countMap[it->at(i).unicode()].middleNum++;
						} else {
							htmInfo htm;
							if(i==1) htm.headNum++;
							else if(i==it->length()-1) htm.tailNum++;
							else htm.middleNum++;
							countMap.insert(std::make_pair(it->at(i).unicode(), htm));
						}
					}
				}
			}

			// 打开写文件操作
			FILE* fpW=fopen("__htmprob.lib", "wb");
			if(fpW==NULL)
				return -2;

			// 文件头信息
			char* pszHeadBuffer=new(std::nothrow) char[BUFSIZ_256];
			Q_ASSERT(pszHeadBuffer!=NULL, "pszHeadBuffer allocate error, size = (%d)", BUFSIZ_256);

			// 文件记录信息
			recordHeader* pri=new(std::nothrow) recordHeader;
			Q_ASSERT(pri!=NULL, "pri allocate error, pri is null");

			// 赋值操作
			dictInfo* pki=(dictInfo*)pszHeadBuffer;
			pki->magicNumber=0x123456789;
			pki->libVersion=*(uint64_t*)"14.08.08";
			pki->author=0x67975c0f725bfeff;
			pki->totalNum=0;

			try {
				// 写入文件头信息
				if(fwrite(pszHeadBuffer, BUFSIZ_256, 1, fpW)!=1)
					throw -3;

				// 写入统计记录数据
				for(std::map<uint16_t, htmInfo>::iterator it=countMap.begin(); it!=countMap.end(); ++it) {
					pri->id=++pki->totalNum;
					pri->qchar=it->first;
					pri->headPbl=static_cast<double>(it->second.headNum)/(it->second.headNum+it->second.tailNum+it->second.middleNum);
					pri->tailPbl=static_cast<double>(it->second.tailNum)/(it->second.headNum+it->second.tailNum+it->second.middleNum);
					pri->middlePbl=static_cast<double>(it->second.middleNum)/(it->second.headNum+it->second.tailNum+it->second.middleNum);

					if(fwrite((char*)pri, sizeof(recordHeader), 1, fpW)!=1)
						throw -4;
				}

				// 写入文件结束标识
				uint64_t end_mark=*(uint64_t*)"@#@#@#@#";
				if(fwrite(&end_mark, sizeof(uint64_t), 1, fpW)!=1)
					throw -5;

				// 写入总记录数
				fseek(fpW, sizeof(dictInfo)-4, SEEK_SET);
				if(fwrite(&(pki->totalNum), sizeof(pki->totalNum), 1, fpW)!=1)
					throw -6;

				Q_DEBUG("Total: (%d) was written!", pki->totalNum);
			} catch(int32_t err) {
				if(fpW) fclose(fpW);
				return err;
			}

			delete [] pszHeadBuffer;
			pszHeadBuffer=NULL;
			delete pri;
			pri=NULL;

			return 0;
		}

		// @函数名: 计算词语的概率
		// @参数01: 词语字符串指针
		// @参数02: 词语字符串长度
		// @返回值: 是未登录词的概率
		double calculateWordProbability(const char* pszWord, int32_t iWordLen)
		{
			// 检查传入的参数值
			if(pszWord==NULL||iWordLen<=0)
				return 0;
			// 计算可能是词语的概率
			double probability=1;
			for(int32_t i=0; i!=iWordLen; i+=sizeof(uint16_t)) {
				recordHeader st_ri;
				if(pURHashTable->search(static_cast<uint64_t>(*(uint16_t *)(pszWord+i)), *(uint16_t *)(pszWord+i), st_ri)) {
					if(i==0) {
						probability*=st_ri.headPbl;
					} else if(i==iWordLen-(int32_t)sizeof(uint16_t)) {
						probability*=st_ri.tailPbl;
					} else {
						probability*=st_ri.middlePbl;
					}
				} else {
					probability=0;
					break;
				}
			}
			return probability;
		}

	private:
		// HTM构词模式哈希表
		QLinearHash<uint16_t, recordHeader> *pURHashTable;
		// 训练数据文件句柄
		FILE *fpR;
};

Q_END_NAMESPACE

#endif // __QNEOLOGISMRECOGNIZER_H_
