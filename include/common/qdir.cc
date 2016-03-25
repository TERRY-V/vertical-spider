#include "qdir.h"

Q_BEGIN_NAMESPACE

QDir::QDir(const char* path) :
	handle_(NULL)
{
	if(path && *path)
		strcpy(dirpath_, path);
	else
		dirpath_[0]='\0';
}

QDir::~QDir()
{
	if(handle_) closedir();
}

void QDir::setPath(const char* path)
{
	strcpy(dirpath_, path);
}

const char* QDir::path() const
{
	return dirpath_;
}

bool QDir::remove(const char* fileName)
{
	char absolutePath[DIR_DEFAULT_PATH_SIZE];
	if(sprintf(absolutePath, "%s/%s", dirpath_, fileName)<0)
		return false;
	return QFile::remove(absolutePath);
}

bool QDir::rename(const char* oldName, const char* newName)
{
	char oldPathName[DIR_DEFAULT_PATH_SIZE];
	char newPathName[DIR_DEFAULT_PATH_SIZE];

	if(sprintf(oldPathName, "%s/%s", dirpath_, oldName)<0)
		return false;
	if(sprintf(newPathName, "%s/%s", dirpath_, newName)<0)
		return false;
	return QFile::rename(oldPathName, newPathName);
}

bool QDir::exists(const char* fileName)
{
	char absolutePath[DIR_DEFAULT_PATH_SIZE];
	if(sprintf(absolutePath, "%s/%s", dirpath_, fileName)<0)
		return false;
	return QFile::exists(absolutePath);
}

bool QDir::chdir(const char* path)
{
#ifdef WIN32
	if(_chdir(path)) {
#else
	if(::chdir(path)) {
#endif
		switch(errno) {
			case ENOENT:
				Q_DEBUG("QDir::chdir: unable to locate the directory: %s", path);
				break;
			case EINVAL:
				Q_DEBUG("QDir::chdir: invalid buffer!");
				break;
			default:
				Q_DEBUG("QDir::chdir: unknwon error!");
				break;
		}
		return false;
	}
	strcpy(dirpath_, path);
	return true;
}

char* QDir::getcwd(char* buf, int32_t size)
{
#ifdef WIN32
	return _getcwd(buf, size);
#else
	return ::getcwd(buf, size);
#endif
}

bool QDir::mkdir()
{
	int32_t len=strlen(dirpath_);
	if(len<=0||len>=DIR_DEFAULT_PATH_SIZE)
		return false;

	char dir_name[DIR_DEFAULT_PATH_SIZE];
	char* dir_ptr=dir_name;

	char* dirpath=dirpath_;
	char* ptr=dirpath;

	if(*ptr=='\\'||*ptr=='/')
		*dir_ptr++=*ptr++;
	if(*ptr=='\0')
		return false;

	Q_FOREVER {
		while(*ptr) {
			if(*ptr=='\\'||*ptr=='/') break;
			*dir_ptr++=*ptr++;
		}
		*dir_ptr=0;

#ifdef WIN32
		if(_mkdir(dir_name)) {
#else
		if(::mkdir(dir_name, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH)) {
#endif
			if(errno!=EEXIST)
				return false;
		}

		if(*ptr)
			*dir_ptr++=*ptr++;

		if(ptr-dirpath>=len)
			break;
	}
	return true;
}

bool QDir::rmdir()
{
#ifdef WIN32
	return false;
#else
	if(dirpath_[0]=='\0')
		return false;

	char* dirpath=dirpath_;

	bool bDir=true;
	int32_t iLen=strlen(dirpath);
	if(dirpath[iLen-1]!='\\'&&dirpath[iLen-1]!='/')
		bDir=false;

	char szFile[DIR_DEFAULT_PATH_SIZE]={0};
	if(bDir) iLen=q_snprintf(szFile, sizeof(szFile)-1, "%s", dirpath);
	else iLen=q_snprintf(szFile, sizeof(szFile)-1, "%s/", dirpath);
	if(iLen<=0)
		return false;

	struct dirent* spDE=NULL;
	DIR* spDir=::opendir(szFile);
	if(spDir==NULL)
		return false;

	int32_t iNum=0;
	while((spDE=::readdir(spDir))!=NULL) {
		if(strcmp(".", spDE->d_name)==0||strcmp("..", spDE->d_name)==0)
			continue;

		if(bDir) iLen=q_snprintf(szFile, sizeof(szFile)-1, "%s%s", dirpath, spDE->d_name);
		else iLen=q_snprintf(szFile, sizeof(szFile)-1, "%s/%s", dirpath, spDE->d_name);
		if(iLen<=0) {
			::closedir(spDir);
			return false;
		}

		if(::remove(szFile))
			Q_DEBUG("QDir::remove (%s) error, errno = (%d)", szFile, errno);
		iNum++;
	}

	::closedir(spDir);
	Q_INFO("QDir::rmdir (%s), file num: (%d)", dirpath, iNum);
	return true;
#endif
}

bool QDir::chmod777()
{
#ifdef WIN32
	return true;
#else
	if(chmod(dirpath_, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH)==-1)
		return false;
	return true;
#endif
}

bool QDir::mkdir(const char* dirpath)
{
	return QDir(dirpath).mkdir();
}

bool QDir::rmdir(const char* dirpath)
{
	return QDir(dirpath).rmdir();
}

bool QDir::chmod777(const char* dirpath)
{
	return QDir(dirpath).chmod777();
}

bool QDir::opendir()
{
#ifdef WIN32
	DIR_TRAVEL_HANDLE* dir_handle=q_new<DIR_TRAVEL_HANDLE>();

	char dirpath[DIR_DEFAULT_PATH_SIZE];
	sprintf(dirpath, "%s*.*", dirpath_);

	dir_handle->travel_flag=1;
	dir_handle->dir_handle=_findfirst(dirpath, &dir_handle->file_handle);
	if(dir_handle->dir_handle==-1) {
		delete dir_handle;
		return false;
	}

	handle_=(void*)dir_handle;
	return true;
#else
	handle_=(void*)::opendir(dirpath_);
	if(handle_==NULL)
		return false;
	return true;
#endif
}

bool QDir::readdir(char* fileName)
{
#ifdef WIN32
	DIR_TRAVEL_HANDLE* dir_handle=(DIR_TRAVEL_HANDLE*)handle;

	Q_FOREVER {
		if(dir_handle->travel_flag!=1) {
			if(_findnext(dir_handle->dir_handle, &dir_handle->file_handle))
				return false;
		} else {
			dir_handle->travel_flag=0;
		}

		int32_t len=strlen(dir_handle->file_handle.name);
		if(len==1&&dir_handle->file_handle.name[0]=='.')
			continue;
		if(len==2&&*(uint16_t*)dir_handle->file_handle.name==*(uint16_t*)"..")
			continue;
		if(dir_handle->file_handle.attrib&_A_SUBDIR)
			continue;

		memcpy(fileName, dir_handle->file_handle.name, len);
		fileName[len]=0;
		break;
	}
	return true;
#else
	/*
	 * dirent结构体定义如下:
	 *
	 * struct dirent {
	 * 	ino_t d_ino;			// 为这个目录进入点的inode
	 * 	off_t d_off;			// 为目录文件开头到这个目录进入点的位移
	 * 	unsigned short int d_reclen;	// d_name的长度, 不包含NULL字符
	 * 	unsigned char d_type;		// d_name所指的文件类型
	 * 	char d_name[256];		// 文件名
	 * };
	 *
	 */
	struct dirent* dirent;

	Q_FOREVER {
		dirent=::readdir((DIR*)handle_);
		if(dirent==NULL)
			return false;

		int32_t len=(int32_t)strlen(dirent->d_name);
		if(len==1&&dirent->d_name[0]=='.')
			continue;
		if(len==2&&*(uint16_t*)dirent->d_name==*(uint16_t*)"..")
			continue;
		if(dirent->d_type==DT_DIR)
			continue;

		memcpy(fileName, dirent->d_name, len);
		fileName[len]=0;
		break;
	}
	return true;
#endif
}

bool QDir::closedir()
{
#ifdef WIN32
	_findclose(((DIR_TRAVEL_HANDLE*)handle_)->dir_handle);

	q_delete<DIR_TRAVEL_HANDLE>(handle_);
	handle_=NULL;
	return true;
#else
	::closedir((DIR*)handle_);
	handle_=NULL;
	return true;
#endif
}

Q_END_NAMESPACE
