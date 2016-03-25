/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qfile.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/11/11
**
*********************************************************************************************/

#ifndef __QFILE_H_
#define __QFILE_H_

#include "qglobal.h"
#include "qfunc.h"

Q_BEGIN_NAMESPACE

class QFile {
	public:
		inline QFile() :
			fp_(NULL)
		{}

		inline QFile(const char* file_name) :
			fp_(NULL),
			file_name_(file_name)
		{}

		virtual ~QFile()
		{if(fp_!=NULL) fclose(fp_);}

		inline void setFileName(const char* file_name)
		{this->file_name_=file_name;}

		std::string path() const;

		std::string baseName() const;

		std::string file_name() const;

		std::string suffix() const;

		bool exists() const;
		static bool exists(const char* file_name);

		bool remove();
		static bool remove(const char* file_name);

		bool rename(const char* file_name);
		static bool rename(const char* oldName, const char* newName);

		bool copy(const char* file_name);
		static bool copy(const char* file_name, const char* newName);

		bool hasdir() const;
		static bool hasdir(const char* file_name);

		// openMode流类型:
		// r	打开只读文件, 该文件必须存在
		// r+	打开可读写的文件, 该文件必须存在
		// w	打开只写文件, 若文件存在则文件长度清零, 即文件内容消失, 若不存在则创建文件
		// w+	打开可读写文件, 若文件存在则文件长度清零, 即文件内容消失, 若不存在则创建文件
		// a	以追加的方式打开只写文件, 若文件不存在, 则创建文件
		// a+	以追加的方式打开可读写的文件, 若文件不存在, 则创建文件
		bool open(const char* openMode="r");

		bool atEnd();

		std::string readLine();

		std::string readAll();

		// 需释放申请的内存空间
		static char* readAll(char *file_name, int64_t* len);

		// 采用C++读文件方式
		static std::string readAll(const char* file_name) throw (std::runtime_error);

		int64_t read(char* data, int64_t maxlen);

		void writeLine(const std::string& line);

		void write(const std::string& line);
		int64_t write(const char* data, int64_t len);

		void reset();

		int64_t size();
		static int64_t size(const char* file_name);

		int32_t setSize(int64_t size);

		int32_t fileno();

		int32_t fileMode() const;

		inline std::string errorString() const
		{return this->error_string_;}

	private:
		Q_DISABLE_COPY(QFile);

	protected:
		FILE* fp_;
		std::string file_name_;
		std::string error_string_;
};

Q_END_NAMESPACE

#endif // __QFILE_H_
