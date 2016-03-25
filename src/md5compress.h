/********************************************************************************************
**
** Copyright (C) 2010-2015 Terry Niu (Beijing, China)
** Filename:	md5compress.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2015/08/10
**
*********************************************************************************************/

#ifndef __MD5COMPRESS_H_
#define __MD5COMPRESS_H_

#include "../include/common/qglobal.h"
#include "../include/common/qmd5.h"

Q_BEGIN_NAMESPACE

static uint64_t make_md5(const char* buf, int32_t len)
{
	QMD5 md5;
	return md5.MD5Bits64((unsigned char*)buf, len);
}

Q_END_NAMESPACE

#endif // __MD5COMPRESS_H_
