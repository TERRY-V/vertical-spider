#include "qfile.h"

Q_BEGIN_NAMESPACE

std::string QFile::file_name() const
{
	int32_t pos=(file_name_.find_last_of('/')!=file_name_.npos)?(file_name_.find_last_of('/')+1):0;
	return file_name_.substr(pos, file_name_.length());
}

std::string QFile::path() const
{
	int32_t pos=(file_name_.find_last_of('/')!=file_name_.npos)?(file_name_.find_last_of('/')+1):0;
	return file_name_.substr(0, pos);
}

std::string QFile::baseName() const
{
	int32_t pos1=(file_name_.find_last_of('/')!=file_name_.npos)?(file_name_.find_last_of('/')+1):0;
	int32_t pos2=(file_name_.find_last_of('.')!=file_name_.npos)?(file_name_.find_last_of('.')-1):file_name_.length();
	return file_name_.substr(pos1, pos2-pos1+1);
}

std::string QFile::suffix() const
{
	int32_t pos=(file_name_.find_last_of('.')!=file_name_.npos)?(file_name_.find_last_of('.')+1):file_name_.length();
	return file_name_.substr(pos, file_name_.length());
}

bool QFile::exists() const
{
	// mode说明
	// F_OK 测试文件是否已经存在
	// R_OK 测试读权限
	// W_OK 测试写权限
	// X_OK 测试执行权限
	return (access(file_name_.c_str(), 0)==0);
}

bool QFile::exists(const char* file_name)
{
	return QFile(file_name).exists();
}

bool QFile::remove()
{
	while(exists()&&::remove(file_name_.c_str())!=0) {
		Q_DEBUG("QFile::remove (%s) failed, errno = (%d)", file_name_.c_str(), errno);
		q_sleep(1000);
	}
	return true;
}

bool QFile::remove(const char* file_name)
{
	return QFile(file_name).remove();
}

bool QFile::rename(const char* file_name)
{
	if(!exists()) {
		error_string_=q_format("file (%s) does not exist!", file_name_.c_str());
		return false;
	}

	QFile::remove(file_name);
	while(exists()&&!exists(file_name)&&::rename(file_name_.c_str(), file_name)!=0) {
		Q_DEBUG("QFile::rename (%s) failed, errno = (%d)", file_name_.c_str(), errno);
		q_sleep(1000);
	}
	return true;
}

bool QFile::rename(const char* oldName, const char* newName)
{
	return QFile(oldName).rename(newName);
}

bool QFile::copy(const char* file_name)
{
	FILE *fp_R=fopen(file_name_.c_str(), "rb");
	if(fp_R==NULL) {
		error_string_=q_format("QFile::copy: oepn file (%s) error!", file_name_.c_str());
		return false;
	}

	FILE *fp_W=fopen(file_name, "wb");
	if(fp_W==NULL) {
		error_string_=q_format("QFile::copy: oepn file (%s) error!", file_name);
		fclose(fp_R);
		return false;
	}

	int32_t iLen=0, iNowLen=0, iFileSize=size();
	char buffer[BUFSIZ_1K]={0};

	try {
		while(iNowLen<iFileSize) {
			iLen=fread(buffer, 1, sizeof(buffer), fp_R);
			if(iLen<=0) {
				error_string_=q_format("QFile::copy: fread error, errno = (%u)", errno);
				throw -1;
			}
			if(fwrite(buffer, iLen, 1, fp_W)!=1) {
				error_string_=q_format("QFile::copy: fwrite error, errno = (%u)", errno);
				throw -2;
			}
			iNowLen+=iLen;
		}
	} catch(const int32_t) {
		fclose(fp_R);
		fclose(fp_W);
		return false;
	}

	fclose(fp_R);
	fclose(fp_W);
	return true;
}

bool QFile::copy(const char* file_name, const char* newName)
{
	return QFile(file_name).copy(newName);
}

bool QFile::hasdir() const
{
#ifdef WIN32
	return true;
#else
	if(q_starts_with(file_name_, "/")||q_starts_with(file_name_, "./") \
			||q_starts_with(file_name_, "../"))
		return true;
	return false;
#endif
}

bool QFile::hasdir(const char* file_name)
{
	return QFile(file_name).hasdir();
}

// openMode流类型:
// r	打开只读文件, 该文件必须存在
// r+	打开可读写的文件, 该文件必须存在
// w	打开只写文件, 若文件存在则文件长度清零, 即文件内容消失, 若不存在则创建文件
// w+	打开可读写文件, 若文件存在则文件长度清零, 即文件内容消失, 若不存在则创建文件
// a	以追加的方式打开只写文件, 若文件不存在, 则创建文件
// a+	以追加的方式打开可读写的文件, 若文件不存在, 则创建文件
bool QFile::open(const char* openMode)
{
	fp_=fopen(file_name_.c_str(), openMode);
	if(fp_==NULL) {
		error_string_=q_format("file (%s) open error!", file_name_.c_str());
		return false;
	}
	return true;
}

bool QFile::atEnd()
{
	return (feof(fp_)?true:false);
}

std::string QFile::readLine()
{
	char buf[BUFSIZ_1K]={0};
	if(fgets(buf, sizeof(buf), fp_))
		q_right_trim(buf, strlen(buf));
	return buf;
}

std::string QFile::readAll()
{
	std::string s;
	int64_t len=size();

	char *data=new(std::nothrow) char[len+1];
	Q_CHECK_PTR(data);

	fseek(fp_, 0, SEEK_SET);
	if(fread(data, len, 1, fp_)!=1) {
		error_string_=q_format("QFile::readAll: fread (%s) error!", file_name_.c_str());
	} else {
		data[len]=0;
		s=data;
	}

	delete [] data;
	return s;
}

// 需释放申请的内存空间
char* QFile::readAll(char *file_name, int64_t* len)
{
	FILE *thisFile=fopen(file_name, "rb");
	if(thisFile==NULL) {
		Q_DEBUG("QFile::readAll: unable to open %s", file_name);
		return (NULL);
	}

	*len=0;
	fseek(thisFile, 0, SEEK_END);
	*len=ftell(thisFile);
	fseek(thisFile, 0, SEEK_SET);

	char *data=new(std::nothrow) char[*len+1];
	int32_t ncount=fread(data, *len, 1, thisFile);
	fclose(thisFile);

	if(ncount!=1) {
		Q_DEBUG("QFile::readAll: read file (%s) error - (%s)", file_name, strerror(errno));
		delete [] data;
		*len=0;
		return (NULL);
	}

	data[*len+1]=0;
	return data;
}

// 采用C++读文件方式
std::string QFile::readAll(const char* file_name) throw (std::runtime_error)
{
	std::ifstream infile(file_name, std::ios::in|std::ios::binary);
	if(infile.fail()) throw std::runtime_error("file: unable to open "+std::string(file_name));
	std::string s;
	infile.seekg(0, std::ios::end);
	s.resize(infile.tellg(), '\t');
	infile.seekg(0, std::ios::beg);
	infile.read(&s[0], s.size());
	s=q_trim(s);
	infile.close();
	infile.clear();
	return s;
}

int64_t QFile::read(char* data, int64_t maxlen)
{
	return fread(data, 1, maxlen, fp_);
}

void QFile::writeLine(const std::string& line)
{
	fputs(line.c_str(), fp_); fputs("\n", fp_);
}

void QFile::write(const std::string& line)
{
	fputs(line.c_str(), fp_);
}

int64_t QFile::write(const char* data, int64_t len)
{
	return fwrite(data, 1, len, fp_);
}

void QFile::reset()
{
	fseek(fp_, 0, SEEK_SET);
}

int64_t QFile::size()
{
	if(!exists(file_name_.c_str())) {
		error_string_=q_format("file (%s) does not exist!", file_name_.c_str());
		return -1;
	}

	/*
	 *
	 * stat文件状态分析
	 * struct stat {
	 * 	dev_t		st_dev;		// 文件的设备编号
	 * 	ino_t		st_ino;		// 文件的i-node
	 * 	mode_t		st_mode;	// 文件的类型和访问权限
	 * 	nlink_t		st_nlink;	// 连接到该文件的硬链接数目
	 * 	uid_t		st_uid;		// 文件拥有者的用户识别码
	 * 	gid_t		st_gid;		// 文件拥有者的组识别码
	 * 	dev_t		st_rdev;	// 如果该文件为装置设备文件, 则为其设备编号
	 * 	off_t		st_size;	// 文件字节大小
	 * 	unsigned long	st_blksize;	// 文件系统I/O缓冲区大小
	 * 	unsigned long	st_blocks;	// 占用文件区块的个数, 每个区块的大小为512字节
	 * 	time_t		st_atime;	// 文件最近一次访问或执行的时间
	 * 	time_t		st_mtime;	// 文件最后一次修改的时间
	 * 	time_t		st_ctime;	// i-node最近一次更改的时间
	 * };
	 *
	 */

	struct stat buf;
	if(stat(file_name_.c_str(), &buf)!=0)
		return 0;
	return buf.st_size;
}

int64_t QFile::size(const char* file_name)
{
	return QFile(file_name).size();
}

int32_t QFile::setSize(int64_t size)
{
#ifdef WIN32
	if(chsize(::fileno(fp_), size))
		return -1;
#else
	if(ftruncate(::fileno(fp_), size))
		return -1;
#endif
	return 0;
}

int32_t QFile::fileno()
{
	return ::fileno(fp_);
}

int32_t QFile::fileMode() const
{
#ifdef WIN32
	return 0;
#else
	struct stat buf;
	if(lstat(file_name_.c_str(), &buf)<0)
		return -1;

	if(S_ISREG(buf.st_mode))		// 普通文件
		return 0;
	else if(S_ISDIR(buf.st_mode))		// 目录文件
		return 1;
	else if(S_ISCHR(buf.st_mode))		// 字符特殊文件
		return 2;
	else if(S_ISBLK(buf.st_mode))		// 块特殊文件
		return 3;
	else if(S_ISFIFO(buf.st_mode))		// FIFO
		return 4;
	else if(S_ISLNK(buf.st_mode))		// 符号链接
		return 5;
	else if(S_ISSOCK(buf.st_mode))		// 套接字
		return 6;
	else
		return -1;
#endif
}

Q_END_NAMESPACE
