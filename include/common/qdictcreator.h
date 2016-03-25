/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qdictcreator.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/08/08
**
*********************************************************************************************/

#ifndef __QDICTCREATOR_H_
#define __QDICTCREATOR_H_

#include "qglobal.h"
#include "qfile.h"
#include "qfunc.h"
#include "qmd5.h"
#include "qtextcodec.h"

#define DICT_OK           (0)
#define DICT_ERR          (-1)

#define DICT_MAGIC_NUMBER (0x123456789)
#define DICT_LIB_VERSION  (*(uint64_t*)"14.08.08")
#define DICT_AUTHOR       (0x67975c0f725bfeff)
#define DICT_END_MARK     (*(uint64_t*)"@#@#@#@#")

Q_BEGIN_NAMESPACE

#pragma pack(1)
struct dictInfo {
	uint64_t magicNumber;
	uint64_t libVersion;
	uint64_t author;
	uint32_t totalNum;
};

struct recordHeader {
	uint64_t id;
	uint32_t length;
};
#pragma pack()

class QDictCreator {
	public:
		static int32_t combine(const char* pszInFileName, const char* pszInCustomFileName);

		static int32_t pack(const char* pszInFileName, const char* pszOutFileName);

		static int32_t pack_pos(const char* pszInFileName, const char* pszOutFileName);

	private:
		QDictCreator();
		virtual ~QDictCreator();
};

Q_END_NAMESPACE

#endif // __QDICTCREATOR_H_
