/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qchinesespelling.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2015/01/22
**
*********************************************************************************************/

#ifndef __QCHINESESPELLING_H_
#define __QCHINESESPELLING_H_

#include "qglobal.h"
#include "qfunc.h"

Q_BEGIN_NAMESPACE

#define MAX_SPELLING_LEN	(8)		// 汉字拼音的最大长度
#define MAX_SPELLING_NUM	(8)		// 单个汉字的最大拼音数
#define MAX_GBK_COUNT		(0xffff)	// GBK编码最大字符数

#pragma pack(1)

// GBK汉字拼音结构体
typedef struct __spelling_node {
	char spelling[MAX_SPELLING_LEN];	// 汉字拼音
	int32_t	spelling_len;			// 汉字拼音的实际长度

	__spelling_node() {
		memset(spelling, 0, sizeof(spelling));
		spelling_len=0;
	}
} SPELLING_NODE;

// GBK汉字拼音信息结构体, 支持多音字
typedef struct __spelling_info {
	int32_t spelling_num;			// 当前汉字的实际拼音数
	struct __spelling_node* spelling_vec;	// 拼音节点信息

	__spelling_info() :
		spelling_num(0),
		spelling_vec(0)
	{}
} SPELLING_INFO;

#pragma pack()

// 中文字符串转拼音类
class QChineseSpelling {
	public:
		QChineseSpelling() :
			spelling_table(0),
			word_num(0),
			read_fp(0)
		{}

		virtual ~QChineseSpelling()
		{
			for(int32_t i=0; i!=MAX_GBK_COUNT; ++i)
				if(spelling_table[i].spelling_vec)
					q_delete_array<SPELLING_NODE>(spelling_table[i].spelling_vec);

			if(spelling_table)
				q_delete_array<SPELLING_INFO>(spelling_table);

			if(read_fp)
				::fclose(read_fp);
		}

		int32_t init(char* cfg_file)
		{
			if(cfg_file==NULL||*cfg_file==0)
				return -1;

			spelling_table=q_new_array<SPELLING_INFO>(MAX_GBK_COUNT);
			if(spelling_table==NULL) {
				Q_INFO("QChineseSpelling: alloc error!");
				return -2;
			}

			read_fp=::fopen(cfg_file, "r");
			if(read_fp==NULL) {
				Q_INFO("QChineseSpelling: file (%s) open error!", cfg_file);
				return -3;
			}

			char read_buf[1<<10]={0};
			int32_t read_buf_len=0;

			uint16_t gbk_idx=0;
			char* blank=NULL;

			char* spelling_beg=NULL;
			int32_t spelling_len=0;

			while(fgets(read_buf, sizeof(read_buf), read_fp)) {
				read_buf_len=q_right_trim(read_buf, strlen(read_buf));
				if(read_buf_len<=0)
					continue;

				gbk_idx=*(uint16_t*)read_buf;

				spelling_beg=read_buf+sizeof(uint16_t);
				while((blank=::strchr(spelling_beg, ' '))) {
					spelling_len=(ptrdiff_t)(blank-spelling_beg);
					if(spelling_len<=0||spelling_len>MAX_SPELLING_LEN) {		// 拼音太短或太长
						spelling_beg=blank+1;
						continue;
					}

					if(insert(gbk_idx, spelling_beg, spelling_len)) {
						Q_INFO("QChineseSpelling: insert error!");
						return -3;
					}

					spelling_beg=blank+1;
				}

				if(insert(gbk_idx, spelling_beg, read_buf+read_buf_len-spelling_beg)) {
					Q_INFO("QChineseSpelling: insert error!");
					return -4;
				}
			}

			return 0;
		}

		int32_t to_spelling(const char* src, int32_t src_len, char* dest, int32_t dest_size)
		{
			if(src==NULL||src_len<=0||dest==NULL||dest_size<=0)
				return -1;

			uint16_t gbk_idx=0;

			int32_t i=0;
			int32_t j=0;
			while(i<src_len) {
				// 判断是否为中文汉字
				if(i<src_len&&src[i]<0) {
					if(j+MAX_SPELLING_LEN>=dest_size) {
						Q_INFO("dest_size is too small, j = (%d)", j);
						return -2;
					}

					gbk_idx=*(uint16_t*)(src+i);
					if(!spelling_table[gbk_idx].spelling_num&&!spelling_table[gbk_idx].spelling_vec) {
						memcpy(dest+j, src+i, sizeof(uint16_t));
						j+=sizeof(uint16_t);
					} else {
						memcpy(dest+j, spelling_table[gbk_idx].spelling_vec[0].spelling, spelling_table[gbk_idx].spelling_vec[0].spelling_len);
						j+=spelling_table[gbk_idx].spelling_vec[0].spelling_len;
					}

					i+=sizeof(uint16_t);
				} else {
					while(i<src_len&&src[i]>=0) {
						if(j+2>=dest_size) {
							Q_INFO("dest_size is too small, j = (%d)", j);
							return -3;
						}
						dest[j++]=src[i++];
					}
				}
			}

			dest[j]=0;
			return j;
		}

		int32_t size() const
		{
			return word_num;
		}

	private:
		int32_t insert(uint16_t gbk_idx, char* spelling_beg, int32_t spelling_len)
		{
			if(spelling_beg==NULL||spelling_len<=0) {
				Q_INFO("QChineseSpelling: spelling_beg is null or spelling_len <=0!");
				return -1;
			}

			if(!spelling_table[gbk_idx].spelling_num&&!spelling_table[gbk_idx].spelling_vec) {
				spelling_table[gbk_idx].spelling_vec=q_new_array<SPELLING_NODE>(MAX_SPELLING_NUM);
				if(spelling_table[gbk_idx].spelling_vec==NULL) {
					Q_INFO("QChineseSpelling: alloc error!");
					return -2;
				}

				++spelling_table[gbk_idx].spelling_num;
				spelling_table[gbk_idx].spelling_vec[0].spelling_len=spelling_len;
				memcpy(spelling_table[gbk_idx].spelling_vec[0].spelling, spelling_beg, spelling_len);
			} else {
				if(spelling_table[gbk_idx].spelling_num+1>=MAX_SPELLING_NUM) {
					Q_INFO("QChineseSpelling: (%s) has too many spellings!", (char*)&gbk_idx);
					return -3;
				}

				++spelling_table[gbk_idx].spelling_num;
				spelling_table[gbk_idx].spelling_vec[spelling_table[gbk_idx].spelling_num].spelling_len=spelling_len;
				memcpy(spelling_table[gbk_idx].spelling_vec[spelling_table[gbk_idx].spelling_num].spelling, spelling_beg, spelling_len);
			}

			++this->word_num;

			return 0;
		}

	protected:
		SPELLING_INFO* spelling_table;
		int32_t word_num;
		FILE* read_fp;
};

Q_END_NAMESPACE

#endif // __QCHINESESPELLING_H_
