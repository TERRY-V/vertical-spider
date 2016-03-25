/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qdir.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/11/12
**
*********************************************************************************************/

#ifndef __QDIR_H_
#define __QDIR_H_

#include "qglobal.h"
#include "qfile.h"

#define DIR_DEFAULT_PATH      ("./")
#define DIR_DEFAULT_PATH_SIZE (1<<8)

Q_BEGIN_NAMESPACE

class QDir: public noncopyable {
	public:
		QDir(const char* path=DIR_DEFAULT_PATH);
		virtual ~QDir();

		void setPath(const char* path);
		const char* path() const;

		bool remove(const char* fileName);
		bool rename(const char* oldName, const char* newName);
		bool exists(const char* fileName);

		bool mkdir();
		bool rmdir();
		bool chmod777();

		static bool mkdir(const char* dirpath);
		static bool rmdir(const char* dirpath);
		static bool chmod777(const char* dirpath);

		bool chdir(const char* path);
		char* getcwd(char* buf, int32_t size);

		bool opendir();
		bool readdir(char* fileName);
		bool closedir();

	protected:
#ifdef WIN32
		typedef struct dir_travel_handle
		{
			int32_t travel_flag;
			int32_t dir_handle;
			struct _finddata_t file_handle;
		} DIR_TRAVEL_HANDLE;
#endif

		char dirpath_[DIR_DEFAULT_PATH_SIZE];
		void* handle_;
};

Q_END_NAMESPACE

#endif // __QDIR_H_
