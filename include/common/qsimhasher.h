/********************************************************************************************
**
** Copyright (C) 2010-2015 Terry Niu (Beijing, China)
** Filename:	qsimhasher.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2015/12/23
**
*********************************************************************************************/

#ifndef __QSIMHASHER_H_
#define __QSIMHASHER_H_

#include "qglobal.h"
#include "qfunc.h"
#include "qwordtokenizer.h"
#include "qmd5.h"

Q_BEGIN_NAMESPACE

#define SIM_OK		(0)
#define SIM_ERR		(-1)

class QSimHasher {
	public:
		QSimHasher();

		virtual ~QSimHasher();

		int32_t init();

		int32_t extract(const std::string& text, int32_t topN, std::vector< std::pair<std::string, double> >& keywords);

		int32_t calculateSimHash(const std::vector< std::pair<std::string, double> >& wordWeights, uint64_t& simVal);

		static bool isEqual(uint64_t lhs, uint64_t rhs, unsigned short n = 3);

		static void toBinaryString(uint64_t req, std::string& res);

		static uint64_t binaryStringToUint64(const std::string& bin);

		void printKeyWords(const std::vector< std::pair<std::string, double> >& keywords);

	private:
		int32_t str2listPOS(const std::string& strPOS, std::list< std::vector<std::string> >& listPOS);

		std::string listPOS2str(const std::list< std::vector<std::string> >& listPOS);

		static bool Compare(const std::pair<std::string, double>& lhs, const std::pair<std::string, double>& rhs);

	protected:
		QWordTokenizer*	tokenizer;
		QMD5		md5;
};

Q_END_NAMESPACE

#endif // __QSIMHASHER_H_
