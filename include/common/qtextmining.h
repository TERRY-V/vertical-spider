/********************************************************************************************
**
** Copyright (C) 2010-2016 Terry Niu (Beijing, China)
** Filename:	qtextmining.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2016/03/10
**
*********************************************************************************************/

#ifndef __QTEXTMINING_H_
#define __QTEXTMINING_H_

class QWordTokenizer;

class QTextMining {
	public:
		inline QTextMining() :
			tokenizer_(NULL)
		{
		}

		virtual ~QTextMining()
		{
			q_delete< QWordTokenizer >(tokenizer_);
		}

		int init()
		{
			tokenzer_ = q_new< QWordTokenizer >();
			if(tokenizer_ == NULL) {
				Q_INFO("QWordTokenizer: alloc error!");
				return -1;
			}

			int ret = tokenizer_->init();
			if(ret<0) {
				Q_INFO("QWordTokenizer: init failed, ret = (%d)!", ret);
				return -2;
			}

			return 0;
		}

		int analyzeFeatures(const char* inString, int inLength, char* outString, int outSize, int* outLength)
		{
			if(inString == NULL || inLength <= 0 || outString == NULL || outSize <= 0 || outLength == NULL)
				return -1;

			std::string src(inString, inLength);
			std::vector<std::string> lines;

			char token_str[1<<20] = {0};
			int ret = 0;

		       	q_sentence_tokenize(src, lines);
			for(int i=0; i<lines.size(); ++i)
			{
				ret = tokenizer_->word_tokenize(lines[i].c_str(), lines[i].length(), token_str, sizeof(token_str));
				if(ret<0)
					return -2;

				std::list< std::vector<std::string> > listPOS;
			}

			return 0;
		}

	private:
		int32_t str2listPOS(const std::string& strPOS, std::list< std::vector<std::string> >& listPOS)
		{
			if(!strPOS.length() || strPOS.at(0)!='(' || strPOS.at(strPOS.length()-1)!=')')
				return -1;

			std::vector<std::string> words=q_split(strPOS.substr(1, strPOS.length()-2), "),(");
			for(size_t i=0; i<words.size(); ++i)
			{
				std::vector<std::string> meanings=q_split_any(words.at(i), "> ");
				listPOS.push_back(meanings);
			}

			return 0;
		}

	protected:
		QWordTokenizer* tokenizer_;
};

#endif // __QTEXTMINING_H_
