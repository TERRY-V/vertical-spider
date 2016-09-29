/********************************************************************************************
**
** Copyright (C) 2010-2015 Terry Niu (Beijing, China)
** Filename:	qglobal.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2012/03/22
**
*********************************************************************************************/

#ifndef __QGLOBAL_H_
#define __QGLOBAL_H_

#ifndef __cplusplus
#error "This is a C++ source!"
#endif

// 命名空间
#define Q_NAMESPACE niu

#ifdef Q_NAMESPACE

#define Q_BEGIN_NAMESPACE namespace Q_NAMESPACE {
#define Q_END_NAMESPACE }
#define Q_USING_NAMESPACE using namespace Q_NAMESPACE;

#else

#define Q_NAMESPACE
#define Q_BEGIN_NAMESPACE
#define Q_END_NAMESPACE
#define Q_USING_NAMESPACE

#endif

// 系统信息
#define Q_VERSION_STR "2.1.0"
#define Q_VERSION 0x020100
#define Q_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))

#define Q_AUTHOR_NAME "TERRY-V"
#define Q_AUTHOR_EMAIL "cnbj8607@163.com"

// 调试信息
#if defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG__)
#ifndef DEBUG
#define DEBUG
#endif
#define DEBUG_FUNC(fragment) {fragment}
#else
#define DEBUG_FUNC(fragment)
#endif

// 识别编译器
#if defined(_MSC_VER)
/* Microsoft Visual C++ */
#elif defined(__GNUC__) || defined(__SUNPROC_CC)
/* GCC */
#define _GCC_VER
#elif defined(__BORLANDC__)
/* Borland c++ */
#elif defined(__INTEL_COMPILER)
/* Intel C++ */
#else
#error "So, what is your compiler?"
#endif

// 识别操作系统
#if defined(__WIN32__) || defined(__WIN32) || defined(_WIN32) || defined(WIN32)
#if defined(__WIN64__) || defined(__WIN64) || defined(_WIN64) || defined(WIN64)
#undef WIN64
#define WIN64
#else
#undef WIN32
#define WIN32
#endif

#elif defined(__unix__) || defined(__unix)
#if defined(__FreeBSD__) || defined (__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__)
#define BSD
#elif defined(__linux__) || defined(__linux) || defined(linux)
#define LINUX
#elif defined(__sun) || defined(sun)
#define SOLARIS
#elif defined(__APPLE__) || defined(_MAC) || defined(macintosh)
#define __DARWIN__
#else
#define UNIX
#endif

#else
#error "So, what is your platform?"
#endif

// 头文件包含定义
#ifdef _MSC_VER

#pragma warning(disable:4244)
#pragma warning(disable:4290)
#pragma warning(disable:4511)
#pragma warning(disable:4512)
#pragma warning(disable:4800)
#pragma warning(disable:4996)

#include <winsock2.h>
#pragma comment(lib, "ws2_32")

#include <UrlMon.h>
#pragma comment(lib, "UrlMon")

#include <Iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")

#include <mswsock.h>
#include <mstcpip.h>
#include <windows.h>

#include <io.h>
#include <time.h>
#include <share.h>
#include <direct.h>
#include <process.h>
#include <sys/stat.h>

#else

#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/vfs.h>
#include <sys/utsname.h>
#include <sys/wait.h>

#include <aio.h>
#include <dirent.h>
#include <fcntl.h>
#include <iconv.h>
#include <netdb.h>
#include <poll.h>
#include <pthread.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <uuid/uuid.h>

#endif

#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <stack>
#include <queue>
#include <deque>
#include <map>
#include <set>
#include <bitset>
#include <algorithm>

#if __cplusplus >= 201103L
#include <array>
#include <functional>
#include <forward_list>
#include <initializer_list>
#include <mutex>
#include <regex>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <wchar.h>
#include <wctype.h>

// 常用变量类型定义
typedef signed char int8_t;		// 8 bit signed
typedef unsigned char uint8_t;		// 8 bit unsigned
typedef short int int16_t;		// 16 bit signed
typedef unsigned short int uint16_t;	// 16 bit unsigned
typedef int int32_t;			// 32 bit signed
typedef unsigned int uint32_t;		// 32 bit unsigned

typedef float float32_t;		// 32 bit float
typedef double float64_t;		// 64 bit double
typedef bool bool8_t;			// 8 bit unsigned

#if defined(_MSC_VER)
typedef __int64 int64_t;			// 64 bit signed
typedef unsigned __int64 uint64_t;		// 64 bit unsigned
#else
#if __WORDSIZE==64
typedef long int int64_t;			// 64 bit signed
typedef unsigned long int uint64_t;		// 64 bit unsigned
#else
__extension__
typedef long long int int64_t;			// 64 bit signed
typedef unsigned long long int uint64_t;	// 64 bit unsigned
#endif
#endif

#if defined(_MSC_VER)
#ifdef WIN32
typedef int32_t ptrdiff_t;			// 32 bit signed
#else
typedef int64_t ptrdiff_t;			// 64 bit signed
#endif
#else
#if __WORDSIZE==64
typedef int64_t ptrdiff_t;			// 64 bit signed
#else
typedef int32_t ptrdiff_t;			// 32 bit signed
#endif
#endif

// 自定义类型值区间
#define __int64_c(c) c ## LL
#define __uint64_c(c) c ## ULL

#define MAX_INT8 (127)
#define MAX_INT16 (32767)
#define MAX_INT32 (2147483647)
#define MAX_INT64 (__int64_c(9223372036854775807))

#define MAX_UINT8 (255)
#define MAX_UINT16 (65535)
#define MAX_UINT32 (4294967295U)
#define MAX_UINT64 (__uint64_c(18446744073709551615))

#define MIN_INT8 (-128)
#define MIN_INT16 (-32767-1)
#define MIN_INT32 (-2147483647-1)
#define MIN_INT64 (-__int64_c(9223372036854775807)-1)

#define MIN_UINT8 (0x00)
#define MIN_UINT16 (0x0000)
#define MIN_UINT32 (0x00000000)
#define MIN_UINT64 (0x0000000000000000)

// 常用的分配内存空间大小的常量
#define BUFSIZ_16 (0x10)
#define BUFSIZ_32 (0x20)
#define BUFSIZ_64 (0x40)
#define BUFSIZ_128 (0x80)
#define BUFSIZ_256 (0x100)
#define BUFSIZ_512 (0x200)

#define BUFSIZ_1K (0x400)
#define BUFSIZ_2K (0x800)
#define BUFSIZ_4K (0x1000)
#define BUFSIZ_8K (0x2000)

#define BUFSIZ_1M (0x100000)
#define BUFSIZ_2M (0x200000)
#define BUFSIZ_3M (0x300000)
#define BUFSIZ_5M (0x500000)
#define BUFSIZ_8M (0x800000)

#define BUFSIZ_10M (10<<20)
#define BUFSIZ_1G (1L<<30)

#define KB(n) (n*1024)
#define MB(n) (n*KB(1024))
#define GB(n) (n*MB(1024))

// 自定义回车换行宏
#define LF (uint8_t)10
#define CR (uint8_t)13
#define CRLF "\x0d\x0a"

// 自定义终端颜色
#define Q_COLOR_NULL ("\x1B[0m")
#define Q_COLOR_RED ("\x1B[31m")
#define Q_COLOR_GREEN ("\x1B[32m")
#define Q_COLOR_YELLOW ("\x1B[33m")

// 自定义状态码
#define STAT_OK (0)
#define STAT_ERR (-1)

// 自定义宏关键字
#ifdef WIN32
#define Q_FASTCALL __fastcall
#define Q_NORETURN __declspec(noreturn)
#else
#define Q_FASTCALL __attribute__((regparm(3)))
#define Q_NORETURN __attribute__((__noreturn__))
#endif

#define Q_CONST_EXPR const
#define Q_EXPLICIT_EXPR explicit
#define Q_INLINE_EXPR inline
#define Q_STATIC_EXPR static

#define Q_CONST_CAST(T_TYPE, A) const_cast<T_TYPE>(A)
#define Q_DYNAMIC_CAST(T_TYPE, A) dynamic_cast<T_TYPE>(A)
#define Q_REINTERPRET_CAST(T_TYPE, A) reinterpret_cast<T_TYPE>(A)
#define Q_STATIC_CAST(T_TYPE, A) static_cast<T_TYPE>(A)

#define Q_FORWARD_CLASS(name) class name;
#define Q_FORWARD_STRUCT(name) struct name;

#define Q_NOTUSED(V) (void)V

#define Q_TRY try
#define Q_CATCH(A) catch(A)
#define Q_THROW(A) throw(A)
#define Q_DECL_NOTHROW throw()

Q_BEGIN_NAMESPACE

// 自定义语法
#define Q_FOREVER for(;;)

#define Q_STRINGIFY(conditional) #conditional

static inline void Q_DEBUG(const char* format, ...)
{
#if defined (DEBUG)
	time_t now=time(NULL);
	struct tm* ptm=localtime(&now);

	va_list args;
	va_start(args, format);
	fprintf(stdout, "%s[%04d-%02d-%02d %02d:%02d:%02d] ", Q_COLOR_YELLOW, ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	fprintf(stdout, "DEBUG: %s", Q_COLOR_NULL);
	vfprintf(stdout, format, args);
	fprintf(stdout, "\n");
	va_end(args);
#endif
}

static inline void Q_INFO(const char* format, ...)
{
	time_t now=time(NULL);
	struct tm* ptm=localtime(&now);

	va_list args;
	va_start(args, format);
	fprintf(stdout, "%s[%04d-%02d-%02d %02d:%02d:%02d] ", Q_COLOR_YELLOW, ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	fprintf(stdout, "INFO: %s", Q_COLOR_NULL);
	vfprintf(stdout, format, args);
	fprintf(stdout, "\n");
	va_end(args);
}

static inline void Q_FATAL(const char* format, ...)
{
	time_t now=time(NULL);
	struct tm* ptm=localtime(&now);

	va_list args;
	va_start(args, format);
	fprintf(stderr, "%s[%04d-%02d-%02d %02d:%02d:%02d] ", Q_COLOR_RED, ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	fprintf(stderr, "FATAL failed: %s", Q_COLOR_NULL);
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	va_end(args);
	abort();
}

static inline void Q_ASSERT_X(bool conditional, const char* file, int32_t line, const char* function, const char* format, ...)
{
	if(!conditional) {
		va_list args;
		va_start(args, format);
		fprintf(stderr, "Q_ASSERT failed at %s:%d in %s: ", file, line, function);
		vfprintf(stderr, format, args);
		fprintf(stderr, "\n");
		va_end(args);
		abort();
	}
}

#define Q_ASSERT(conditional, ...) \
	( \
	  (conditional)?(void)(0):( \
		  fprintf(stderr, "Q_ASSERT failed at (%s) in %s:%d in %s: ", \
			  Q_STRINGIFY(conditional), __FILE__, __LINE__, __FUNCTION__), \
		  fprintf(stderr, __VA_ARGS__), \
		  fprintf(stderr, "\n"), \
		  abort()) \
	)

template <bool> struct StaticAssert {};
#define Q_STATIC_ASSERT(conditional, message) \
	typedef StaticAssert<(bool(conditional))> message[bool(conditional)?1:-1]

#define Q_CHECK_PTR(p) \
	do {if(!(p)) q_check_pointer(__FILE__, __LINE__, __FUNCTION__);} while (0)

#define Q_DISABLE_COPY(TypeName) \
	  TypeName(const TypeName&); \
	  TypeName& operator=(const TypeName&)

#define Q_JUST_CONTINUE(conditional) \
	if(conditional) continue

#define Q_JUST_BREAK(conditional) \
	if(conditional) break

#define Q_JUST_RETURN(conditional, ret) \
	if(conditional) return ret

// 自定义宏函数
#define q_is_alpha(x) ((x>='a'&&x<='z')||(x>='A'&&x<='Z'))
#define q_is_number(x) (x>='0'&&x<='9')
#define q_is_alpha_or_number(x) ((x>='a'&&x<='z')||(x>='A'&&x<='Z')||(x>='0'&&x<='9'))
#define q_is_upper(x) (x>='A'&&x<='Z')
#define q_is_lower(x) (x>='a'&&x<='z')
#define q_upper(x) ((x>='a'&&x<='z')?x&0x5f:x)
#define q_lower(x) ((x>='A'&&x<='Z')?x|0x20:x)

#define q_abs(x) (x>=0?x:-x)
#define q_is_odd(x) (x&1==1)
#define q_max(x, y) (x>=y?x:y)
#define q_min(x, y) (x<=y?x:y)
#define q_max_3(x, y, z) ((x>y)?((x>z)?x:z):((y>z)?y:z))
#define q_min_3(x, y, z) ((x<y)?((x<z)?x:z):((y<z)?y:z))
#define q_average(x, y) ((x+y)>>1)
#define q_average_3(x, y, z) ((x+y+z)>>1)

#define q_array_size(a) (sizeof(a)/sizeof(*a))
#define q_bit(n, m) ((n>>(m-1))&1)

#define q_terminate() (exit(0))

#ifdef WIN32
#define q_sscanf sscanf_s
#define q_snprintf _snprintf

#define q_strcasecmp stricmp
#define q_strncasecmp strnicmp
#else
#define q_sscanf sscanf
#define q_snprintf snprintf

#define q_strcasecmp strcasecmp
#define q_strncasecmp strncasecmp
#endif

#ifdef WIN32
#define q_sleep(x) Sleep(x)
#else
#define q_sleep(x) usleep(x*1000)
#endif
#define q_serve_forever() Q_FOREVER q_sleep(5000)

// 自定义枚举类型
enum ByteOrder {BigEndian, LittleEndian};

enum CaseSensitivity {CaseInsensitive, CaseSensitive};

enum DayOfWeek {Monday=1, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday};

enum Orientation {Horizontal=0x01, Vertical};

enum SortOrder {NoSort=0x00, AscendingOrder, DescendingOrder};

enum State {Stopped=0x00, Paused, Running};

// 原子操作
__inline uint32_t q_add_and_fetch(uint32_t* val)
{
#ifdef WIN32
	return InterlockedIncrement(val);
#else
	return __sync_add_and_fetch(val, 1);
#endif
}

__inline uint32_t q_sub_and_fetch(uint32_t* val)
{
#ifdef WIN32
	return InterlockedDecrement(val);
#else
	return __sync_sub_and_fetch(val, 1);
#endif
}

__inline uint64_t q_add_and_fetch(uint64_t* val)
{
#ifdef WIN32
	return InterlockedIncrement(val);
#else
	return __sync_add_and_fetch(val, 1);
#endif
}

__inline uint64_t q_sub_and_fetch(uint64_t* val)
{
#ifdef WIN32
	return InterlockedDecrement(val);
#else
	return __sync_sub_and_fetch(val, 1);
#endif
}

__inline void q_lock_inc(uint32_t* val)
{
#ifdef WIN32
	InterlockedIncrement(val);
#else
	__sync_add_and_fetch(val, 1);
#endif
}

__inline void q_lock_dec(uint32_t* val)
{
#ifdef WIN32    
	InterlockedDecrement(val);
#else
	__sync_sub_and_fetch(val, 1);
#endif
}

__inline void q_lock_inc(uint64_t* val)
{
#ifdef WIN32
	InterlockedIncrement(val);
#else
	__sync_add_and_fetch(val, 1);
#endif
}

__inline void q_lock_dec(uint64_t* val)
{
#ifdef WIN32
	InterlockedDecrement(val);
#else
	__sync_sub_and_fetch(val, 1);
#endif
}

// 字节处理函数
static inline int32_t q_byte_order()
{
	union {
		uint16_t value;
		char union_bytes[sizeof(uint16_t)];
	} endian;

	endian.value=0x0102;
	if((endian.union_bytes[0]==1)&&(endian.union_bytes[1]==2)) {
		// big endian
		return 2;
	} else if((endian.union_bytes[0]==2)&&(endian.union_bytes[1]==1)) {
		// little endian(主机字节序)
		return 1;
	} else {
		return -1;
	}
}

static inline void q_byte_swap16(void* p)
{
	register uint8_t *x=(uint8_t*)p, t;
	t=x[0];
	x[0]=x[1];
	x[1]=t;
}

static inline void q_byte_swap32(void* p)
{
	register uint8_t *x=(uint8_t*)p, t;
	t=x[0];
	x[0]=x[3];
	x[3]=t;
	t=x[1];
	x[1]=x[2];
	x[2]=t;
}

static inline void q_byte_swap64(void* p)
{
	register uint8_t *x=(uint8_t*)p, t;
	t=x[0];
	x[0]=x[7];
	x[7]=t;
	t=x[1];
	x[1]=x[6];
	x[6]=t;
	t=x[2];
	x[2]=x[5];
	x[5]=t;
	t=x[3];
	x[3]=x[4];
	x[4]=t;
}

static inline uint16_t q_byte_swap(uint16_t source)
{
	return uint16_t(0
			|((source&0x00ff)<<8)
			|((source&0xff00)>>8));
}

static inline uint32_t q_byte_swap(uint32_t source)
{
	return 0
		|((source&0x000000ff)<<24)
		|((source&0x0000ff00)<<8)
		|((source&0x00ff0000)>>8)
		|((source&0xff000000)>>24);
}

static inline uint64_t q_byte_swap(uint64_t source)
{
	return 0
		|((source&__uint64_c(0x00000000000000ff))<<56)
		|((source&__uint64_c(0x000000000000ff00))<<40)
		|((source&__uint64_c(0x0000000000ff0000))<<24)
		|((source&__uint64_c(0x00000000ff000000))<<8)
		|((source&__uint64_c(0x000000ff00000000))>>8)
		|((source&__uint64_c(0x0000ff0000000000))>>24)
		|((source&__uint64_c(0x00ff000000000000))>>40)
		|((source&__uint64_c(0xff00000000000000))>>56);
}

template<typename T_TYPE> static inline void q_swap(T_TYPE& val1, T_TYPE& val2)
{
	using std::swap;
	swap(val1, val2);
}

// 内存分配释放相关
#ifdef MALLOC_SIZE
#define PREFIX_SIZE (sizeof(uint32_t))
#else
#define PREFIX_SIZE (0)
#endif

static inline void *q_malloc(uint32_t size)
{
	register void* ptr=::malloc(size+PREFIX_SIZE);
	if(!ptr)
		Q_FATAL("q_malloc out of memory, size = (%lu)!", size);
#ifdef MALLOC_SIZE
	*((uint32_t*)ptr)=size;
	return (char*)ptr+PREFIX_SIZE;
#else
	return ptr;
#endif
}

static inline void *q_calloc(uint32_t size)
{
	register void* ptr=::calloc(1, size+PREFIX_SIZE);
	if(!ptr)
		Q_FATAL("q_calloc out of memory, size = (%lu)!", size);
#ifdef MALLOC_SIZE
	*((uint32_t*)ptr)=size;
	return (char*)ptr+PREFIX_SIZE;
#else
	return ptr;
#endif
}

#ifdef MALLOC_SIZE
static inline uint32_t q_malloc_size(void* ptr)
{
	register void* realptr=(char*)ptr-PREFIX_SIZE;
	uint32_t size=*((uint32_t*)realptr);
	return size+PREFIX_SIZE;
}
#endif

static inline void *q_realloc(void* ptr, uint32_t size)
{
	if(ptr==NULL) return q_malloc(size);

	register void* newptr=::realloc(ptr, size);
	if(!newptr)
		Q_FATAL("q_realloc out of memory, size = (%lu)!", size);
#ifdef MALLOC_SIZE
	*((uint32_t*)((char*)newptr-PREFIX_SIZE))=size;
	return newptr;
#else
	return newptr;
#endif
}

static inline void q_free(void* ptr)
{
	if(ptr==NULL) return;
#ifdef MALLOC_SIZE
	register void* realptr=(char*)ptr-PREFIX_SIZE;
	return ::free(realptr);
#else
	return ::free(ptr);
#endif
}

static inline char* q_strdup(const char* ptr)
{
	if(ptr==NULL) return NULL;
	register uint32_t len=strlen(ptr)+1;
	register char* p=(char*)q_malloc(len);
	memcpy(p, ptr, len);
	return p;
}

template<typename T_TYPE> static inline T_TYPE *q_new()
{
	register T_TYPE *Pointer=NULL;
	try {
		Pointer=new T_TYPE;
	} catch (...) {
		Pointer=NULL;
	}
	return Pointer;
}

template<typename T_TYPE> static inline T_TYPE *q_new_array(uint32_t size)
{
	register T_TYPE *Pointer=NULL;
	try {
		Pointer=new T_TYPE[size];
	} catch (...) {
		Pointer=NULL;
	}
	return Pointer;
}

#define Q_NEW(Pointer, Class, ...) \
	do { \
		try { \
			Pointer=new Class(__VA_ARGS__); \
		} catch (...) { \
			Pointer = NULL; \
		} \
	} while(0)

template<typename T_TYPE> static inline void q_delete(T_TYPE*& rp)
{
	if(rp!=NULL) {
		delete rp;
		rp=NULL;
	}
}

template<typename T_TYPE> static inline void q_delete_array(T_TYPE*& rp)
{
	if(rp!=NULL) {
		delete [] rp;
		rp=NULL;
	}
}

static void q_check_pointer(const char* file, int32_t line, const char* function)
{
	Q_FATAL("In file %s, line %d, function %s: out of memory!", file, line, function);
}

static inline void q_hex_dump(const char *descr, void *value, int32_t len)
{
	char buf[48];
	char* b=buf;
	uint8_t* v=(uint8_t*)value;
	char charset[]="0123456789abcdef";

	Q_INFO("%s (hexdump):", descr);
	while(len) {
		b[0]=charset[(*v)>>4];
		b[1]=charset[(*v)&0xf];
		b[2]=' ';
		b+=3;
		len--;
		v++;
		if(b-buf==sizeof(buf)||len==0) {
			*(b-1)='\0';
			Q_INFO("%s -> %.*s", buf, (b-buf)/3, (char*)(v-(b-buf)/3));
			b=buf;
		}
	}
}

// 字符串编码
#ifdef WIN32
static char *q_memfrob(char* s, uint32_t n)
{
	for(uint32_t i=0; i<n; ++i)
		s[i]^=42;
	return s;
}
#else
static char *q_memfrob(char* s, uint32_t n)
{
	return (char*)memfrob(s, n);
}
#endif

// 斐波那契递归相关(递归算法效率远低于非递归算法)
static int64_t q_recursion_fibnacci(int64_t n)
{
	if(n<=1) return n;
	else return q_recursion_fibnacci(n-1)+q_recursion_fibnacci(n-2);
}

static int64_t q_iterator_fibnacci(int64_t n)
{
	if(n<=1) return n;
	int64_t twoback=0, oneback=1, current;
	for(int64_t i=2; i<=n; ++i) {
		current=twoback+oneback;
		twoback=oneback;
		oneback=current;
	}
	return current;
}

// 文件相关
static int64_t q_get_file_size(char* file)
{
	struct stat file_info;
	if(stat(file, &file_info))
		return -1;
	return file_info.st_size;
}

static int32_t q_change_file_size(FILE* fp, int64_t size)
{
#ifdef WIN32
	if(chsize(fileno(fp), size))
		return -1;
#else
	if(ftruncate(fileno(fp), size))
		return -1;
#endif
	return 0;
}

static int32_t q_repair_file(char* file, const char* end_flag, int32_t end_flag_len, int64_t* repair_len=NULL)
{
	if(::access(file, 0))
		return -1;

	int64_t real_len=0;
	int64_t file_len=q_get_file_size(file);
	if(file_len<0)
		return -1;

	if(file_len==0) {
		if(repair_len)
			*repair_len=file_len;
		return 0;
	}

	int32_t read_len=0;
	char* read_buf=q_new_array<char>(1<<20);
	if(read_buf==NULL)
		return -1;

	FILE* fp=fopen(file, "rb+");
	if(fp==NULL) {
		q_delete_array<char>(read_buf);
		return -1;
	}

	for(;;) {
		read_len=1<<20;
		if(file_len<read_len)
			read_len=(int32_t)file_len;
		file_len-=read_len;

		::fseek(fp, file_len, SEEK_SET);

		if(::fread(read_buf, read_len, 1, fp)!=1) {
			::fclose(fp);
			q_delete_array<char>(read_buf);
			return -1;
		}

		int32_t loop_num=read_len-end_flag_len+1;
		if(loop_num<=0)
			break;

		char* beg_pos=read_buf+(read_len-end_flag_len);
		char* cur_pos=beg_pos;
		for(int32_t i=0; i<loop_num; ++i) {
			if(::memcmp(cur_pos, end_flag, end_flag_len)==0)
				break;
			cur_pos--;
		}

		if(cur_pos+loop_num!=beg_pos) {
			real_len=file_len+read_len-(beg_pos-cur_pos);
			break;
		}

		if(file_len<=0)
			break;
	}

	if(q_change_file_size(fp, real_len)) {
		fclose(fp);
		q_delete_array<char>(read_buf);
		return -1;
	}

	fclose(fp);
	q_delete_array<char>(read_buf);

	if(repair_len)
		*repair_len=real_len;

	return 0;
}

static int32_t q_swap_file(char* new_file, char* org_file)
{
	char bak_file[1<<8];
	sprintf(bak_file, "%s.recovery", org_file);

	while(access(bak_file, 0)==0&&remove(bak_file)) {
		Q_DEBUG("q_swap_file - remove %s error!", bak_file);
		q_sleep(5000);
	}

	while(access(org_file, 0)==0&&rename(org_file, bak_file)) {
		Q_DEBUG("q_swap_file - rename %s to %s error!", org_file, bak_file);
		q_sleep(5000);
	}

	while(access(new_file, 0)==0&&rename(new_file, org_file)) {
		Q_DEBUG("q_swap_file - rename %s to %s error!", new_file, org_file);
		q_sleep(5000);
	}

	return 0;
}

static int32_t q_recovery_file(char* org_file)
{
	if(access(org_file, 0)==0)
		return 0;

	char bak_file[1<<8];
	sprintf(bak_file, "%s.recovery", org_file);

	if(access(bak_file, 0))
		return 0;

	if(rename(bak_file, org_file)) {
		Q_DEBUG("q_recovery_file - rename %s to %s error!", bak_file, org_file);
		return -1;
	}

	return 0;
}

static int32_t q_clear_file(char* org_file)
{
	char bak_file[1<<8];
	sprintf(bak_file, "%s.recovery", org_file);

	if(access(bak_file, 0)==0&&remove(bak_file)) {
		Q_DEBUG("q_clear_file - remove %s error!", bak_file);
		return -1;
	}

	if(access(org_file, 0)==0&&remove(org_file)) {
		Q_DEBUG("q_clear_file - remove %s error!", org_file);
		return -1;
	}

	return 0;
}

// 系统信息相关
static int32_t q_get_username(char* username, int32_t size)
{
#ifdef WIN32
	DWORD length=size;
	if(!GetUserName(username, &length)) {
		Q_DEBUG("q_get_username - couldn't get the user name!");
		return -1;
	}
	return 0;
#else
	struct passwd* pw=getpwuid(getuid());
	if(pw==0) {
		Q_DEBUG("q_get_username - couldn't get the user name!");
		return -1;
	}
	::strcpy(username, pw->pw_name);
	return 0;
#endif
}

static int32_t q_get_user_id(const char* username)
{
#ifdef WIN32
	return 0;
#else
	struct passwd* pw=getpwnam(username);
	if(pw==0)
		return -1;
	else
		return pw->pw_uid;
#endif
}

static inline int32_t q_get_kernel_info(char* kernel_info, int32_t size)
{
#ifdef WIN32
	OSVERSIONINFOEX osver;
	osver.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEX);
	if(!GetVersionEx((LPOSVERSIONINFOA)&osver))
		return -1;
	// https://msdn.microsoft.com/en-us/library/ms724833.aspx
	if(sprintf(kernel_info, "Version: %d.%d Build: %d Service Pack: %d.%d", osver.dwMajorVersion, osver.dwMinorVersion, \
				osver.dwBuildNumber, osver.wServicePackMajor, osver.wServicePackMinor)<0)
		return -1;
	return 0;
#else
	struct utsname name;
	if(uname(&name))
		return -1;
	// get name and information about current kernel.
	if(sprintf(kernel_info, "%s %s %s %s %s", name.sysname, name.nodename, name.release, name.version, name.machine)<0)
		return -1;
	return 0;
#endif
}

static inline int32_t q_get_cpu_processors()
{
#ifdef WIN32
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
#else
	return get_nprocs();
#endif
}

static inline int32_t q_get_load_avg()
{
#ifdef WIN32
	return 0;
#else
	register int l=0;
	register double d;
	// The getloadavg() function returns the number of processes in the system run queue averaged over various periods of time.
	if(getloadavg(&d, 1)==1) {
		l=(int32_t)(d*100);
		if(l<10)
			l=10;
	}
	return l;
#endif
}

static inline int32_t q_get_disk_usage(const char* path, uint64_t* used_bytes, uint64_t* total_bytes)
{
#ifdef WIN32
	ULARGE_INTEGER lpfree2caller;
	ULARGE_INTEGER lptotal;
	ULARGE_INTEGER lpfree;
	if(GetDiskFreeSpaceEx(LPCSTR(L"./"), &lpfree2caller, &lptotal, &lpfree)) {
		*used_bytes=lptotal.QuadPart-lpfree2caller.QuadPart;
		*total_bytes=lptotal.QuadPart;
		return 0;
	}
	return -1;
#else
	*used_bytes=1;
	*total_bytes=1;
	struct statfs buf;
	if(statfs(path, &buf)!=0)
		return -1;
	*used_bytes=(uint64_t)(buf.f_blocks-buf.f_bfree)*buf.f_bsize;
	*total_bytes=(uint64_t)(buf.f_blocks)*buf.f_bsize;
	return 0;
#endif
}

static inline int32_t q_get_mem_usage(uint64_t* used_mem_bytes, uint64_t* total_mem_bytes)
{
#ifdef WIN32
	MEMORYSTATUS memstatus;
	GlobalMemoryStatus(&memstatus);
	*used_mem_bytes=memstatus.dwAvailPhys;
	*total_mem_bytes=memstatus.dwTotalPhys;
	return 0;
#else
	struct sysinfo __si;
	if(::sysinfo(&__si)!=0)
		return -1;

	*used_mem_bytes=__si.totalram-__si.freeram;
	*total_mem_bytes=__si.totalram;

	return 0;
#endif
}

#ifdef WIN32
#else
static inline int32_t q_getrlimit(__rlimit_resource_t resource, uint32_t& rlim_cur, uint32_t& rlim_max)
{
	struct rlimit limit;
	if(getrlimit(resource, &limit)==-1)	/* RLIMIT_NOFILE */
		return -1;
	rlim_cur=limit.rlim_cur;
	rlim_max=limit.rlim_max;
	return 0;
}
#endif

#ifdef WIN32
#else
static inline int32_t q_setrlimit(__rlimit_resource_t resource, uint32_t rlim)
{
	struct rlimit limit;
	if(getrlimit(resource, &limit)==-1)	/* RLIMIT_NOFILE */
		return -1;
	if(rlim<=limit.rlim_cur)
		return 0;
	limit.rlim_cur=rlim;
	limit.rlim_max=(rlim<=limit.rlim_max)?limit.rlim_max:rlim;
	if(setrlimit(resource, &limit)==-1)
		return -1;
	return 0;
}
#endif

static inline std::string q_format_byte_size(double bytes)
{
	static const char _sizeunits[]="KMGTP";
	char s[16];
	int32_t level=0;
	while(bytes>=1024.0) {
		bytes/=1024.0;
		level++;
		if(level>=5) break;
	}
	if(level>0) {
		q_snprintf(s, 16, "%.1f%c", bytes, _sizeunits[level-1]);
	} else {
		q_snprintf(s, 16, "%d", (int32_t)bytes);
	}
	return s;
}

// 多线程相关
#ifdef WIN32
#define Q_THREAD_T DWORD WINAPI
#else
#define Q_THREAD_T void*
#endif

#ifdef WIN32
static int32_t q_create_thread(LPTHREAD_START_ROUTINE in_function, void* in_argv)
{
	DWORD dwThreadID;
	HANDLE hThreadHandle=CreateThread(NULL, 0, in_function, in_argv, 0, &dwThreadID);
	if(hThreadHandle==NULL)
		return -1;
	CloseHandle(hThreadHandle);
	return 0;
}
#else
static int32_t q_create_thread(void*(in_function)(void*), void* in_argv)
{
	pthread_t tid;
	pthread_attr_t attr;
	// Initialize the thread attributes object pointed to by attr with default attribute values.
	if(pthread_attr_init(&attr))
		return -1;

	// Threads that are created using PTHREAD_CREATE_DETACHED will be created in a detached state.
	if(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))
		return -1;

#ifdef __CONTENT_SCOPE__
	// The thread competes for resources with all other threads in all processes on the system that 
	// are in the same scheduling allocation domain (a group of one or more processors).
	if(pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM))
		return -1;
#endif

#ifdef __SCHED_POLICY__
	// Set the thread's scheduling policy, SCHED_FIFO, SCHED_RR, and SCHED_OTHER.
	if(pthread_attr_setschedpolicy(&attr, SCHED_FIFO))
		return -1;
#endif

#ifdef __GUARD_SIZE__
	// If guardsize is greater than 0, then for each new thread created using attr the system allocates 
	// an addi-tional region of at least guardsize bytes at the end of the thread's stack to act as 
	// the guard area for the stack.
	if(pthread_attr_setguardsize(&attr, BUFSIZ_1M))
		return -1;
#endif

#ifdef __INHERIT_SCHED_
	// Threads that are created using attr inherit scheduling attributes from the creating thread; 
	// the scheduling attributes in attr are ignored.
	if(pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED))
		return -1;
#endif

#ifdef __STACK_SIZE__
	// The stack size attribute determines the minimum size (in bytes) that will be allocated for 
	// threads created using the thread attributes object attr.
	if(pthread_attr_setstacksize(&attr, BUFSIZ_8M))
		return -1;
#endif

	// Starts a new thread in the calling process.
	if(pthread_create(&tid, &attr, in_function, in_argv))
		return -1;

	// When a thread attributes object is no longer required, it should be destroyed using the 
	// pthread_attr_destroy() function. Destroying a thread attributes object has no effect on threads 
	// that were created using that object.
	if(pthread_attr_destroy(&attr))
		return -1;
	return 0;
}
#endif

#ifdef WIN32
static int32_t q_create_thread(HANDLE& hThreadHandle, LPTHREAD_START_ROUTINE in_function, void* in_argv)
{
	DWORD dwThreadID;
	hThreadHandle=CreateThread(NULL, 0, in_function, in_argv, 0, &dwThreadID);
	if(hThreadHandle==NULL)
		return -1;
	return 0;
}
#else
static int32_t q_create_thread(pthread_t* tid, void*(in_function)(void*), void* in_argv)
{
	pthread_attr_t attr;
	// Initialize the thread attributes object pointed to by attr with default attribute values.
	if(pthread_attr_init(&attr))
		return -1;

#ifdef __CONTENT_SCOPE__
	// The thread competes for resources with all other threads in all processes on the system that 
	// are in the same scheduling allocation domain (a group of one or more processors).
	if(pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM))
		return -1;
#endif

#ifdef __SCHED_POLICY__
	// Set the thread's scheduling policy, SCHED_FIFO, SCHED_RR, and SCHED_OTHER.
	if(pthread_attr_setschedpolicy(&attr, SCHED_FIFO))
		return -1;
#endif

#ifdef __GUARD_SIZE__
	// If guardsize is greater than 0, then for each new thread created using attr the system allocates 
	// an addi-tional region of at least guardsize bytes at the end of the thread's stack to act as 
	// the guard area for the stack.
	if(pthread_attr_setguardsize(&attr, BUFSIZ_1M))
		return -1;
#endif

#ifdef __INHERIT_SCHED_
	// Threads that are created using attr inherit scheduling attributes from the creating thread; 
	// the scheduling attributes in attr are ignored.
	if(pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED))
		return -1;
#endif

#ifdef __STACK_SIZE__
	// The stack size attribute determines the minimum size (in bytes) that will be allocated for 
	// threads created using the thread attributes object attr.
	if(pthread_attr_setstacksize(&attr, BUFSIZ_8M))
		return -1;
#endif

	// Starts a new thread in the calling process.
	if(pthread_create(tid, &attr, in_function, in_argv))
		return -1;

	// When a thread attributes object is no longer required, it should be destroyed using the 
	// pthread_attr_destroy() function. Destroying a thread attributes object has no effect on threads 
	// that were created using that object.
	if(pthread_attr_destroy(&attr))
		return -1;
	return 0;
}
#endif

#ifdef WIN32
static int32_t q_thread_join(HANDLE hThreadHandle)
{
	WaitForSingleObject(hThreadHandle, INFINITE);
	CloseHandle(hThreadHandle);
	return 0;
}
#else
static int32_t q_thread_join(pthread_t tid)
{
	// The pthread_join() function waits for the thread specified by thread to terminate.
	if(pthread_join(tid, NULL))
		return -1;
	return 0;
}
#endif

static uint64_t q_thread_id()
{
#ifdef WIN32
	return GetCurrentThreadId();
#else
	return pthread_self();
#endif
}

// Socket套接字
#ifdef WIN32
#define Q_SOCKET_T SOCKET
#else
#define Q_SOCKET_T int32_t
#endif

#ifdef __NETINET_HEADER__
#ifdef WIN32
typedef struct HeadIP {
	unsigned char headerlen:4;
	unsigned char version:4;
	unsigned char servertype;
	unsigned short totallen;
	unsigned short id;
	unsigned short idoff;
	unsigned char ttl;
	unsigned char proto;
	unsigned short checksum;
	unsigned int sourceIP;
	unsigned int destIP;
} HEADIP;

typedef struct HeadTCP {
	WORD SourcePort;
	WORD DePort;
	DWORD SequenceNo;
	DWORD ConfirmNo;
	BYTE HeadLen;
	BYTE Flag;
	WORD WndSize;
	WORD CheckSum;
	WORD UrgPtr;
} HEADTCP;

typedef struct HeadICMP {
	BYTE Type;
	BYTE Code;
	WORD ChkSum;
} HEADICMP;

typedef struct HeadUDP {
	WORD SourcePort;
	WORD DePort;
	WORD Len;
	WORD ChkSum;
} HEADUDP;
#else
struct iphdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u8	ihl:4,
		version:4;
#elif defined (__BIG_ENDIAN_BITFIELD)
	__u8	version:4,
  		ihl:4;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
	__u8	tos;
	__be16	tot_len;
	__be16	id;
	__be16	frag_off;
	__u8	ttl;
	__u8	protocol;
	__sum16	check;
	__be32	saddr;
	__be32	daddr;
	/*The options start here. */
};

struct tcphdr {
	__be16	source;
	__be16	dest;
	__be32	seq;
	__be32	ack_seq;
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u16	res1:4,
		doff:4,
		fin:1,
		syn:1,
		rst:1,
		psh:1,
		ack:1,
		urg:1,
		ece:1,
		cwr:1;
#elif defined(__BIG_ENDIAN_BITFIELD)
	__u16	doff:4,
		res1:4,
		cwr:1,
		ece:1,
		urg:1,
		ack:1,
		psh:1,
		rst:1,
		syn:1,
		fin:1;
#else
#error	"Adjust your <asm/byteorder.h> defines"
#endif	
	__be16	window;
	__sum16	check;
	__be16	urg_ptr;
};

struct icmphdr
{
	u_int8_t type;
	u_int8_t code;
	u_int16_t checksum;
	union
	{
		struct
		{
			u_int16_t	id;
			u_int16_t	sequence;
		} echo;
		u_int32_t	gateway;
		struct
		{
			u_int16_t	__glibc_reserved;
			u_int16_t	mtu;
		} frag;
	} un;
};

struct udphdr {
	__be16	source;
	__be16	dest;
	__be16	len;
	__sum16	check;
};
#endif
#endif // __NETINET_HEADER__

static int32_t q_init_socket()
{
#ifdef WIN32
	WSAData cWSAData;
	if(WSAStartup(MAKEWORD(2, 2), &cWSAData)) {
		Q_DEBUG("q_init_socket: init socket failure!");
		return -1;
	}
#else
	sigset_t signal_mask;
	if(sigemptyset(&signal_mask)) {
		Q_DEBUG("q_init_socket: sigemptyset failure!");
		return -1;
	}
	if(sigaddset(&signal_mask, SIGPIPE)) {
		Q_DEBUG("q_init_socket: sigaddset failure!");
		return -1;
	}
	if(pthread_sigmask(SIG_BLOCK, &signal_mask, NULL)) {
		Q_DEBUG("q_init_socket: block sigpipe failure!");
		return -1;
	}
#endif
	return 0;
}

static void q_close_socket(Q_SOCKET_T in_socket)
{
#ifdef WIN32
	::closesocket(in_socket);
#else
	::close(in_socket);
#endif
}

static inline char* q_local_ip()
{
#ifdef WIN32
	char hostname[1<<7]={0};
	struct hostent* ptr_hostent=NULL;

	gethostname(hostname, sizeof(hostname));
	ptr_hostent=gethostbyname(hostname); 
	return inet_ntoa(*(struct in_addr*)ptr_hostent->h_addr_list[0]); 
#else
	struct ifreq ifr;
	Q_SOCKET_T sock;
	struct sockaddr_in sin;

	sock=socket(AF_INET, SOCK_DGRAM, 0); 
	if(sock<0) {
		Q_DEBUG("q_local_ip: create socket failure!");
		return NULL;
	}

	strcpy(ifr.ifr_name, "eth0");
	if(ioctl(sock, SIOCGIFADDR, &ifr)<0) { 
		q_close_socket(sock);
		Q_DEBUG("q_local_ip: call ioctl failure!");
		return NULL;
	} 
	memcpy(&sin, &ifr.ifr_addr, sizeof(struct sockaddr_in));

	q_close_socket(sock);
	return inet_ntoa(sin.sin_addr); 
#endif
}

static inline int32_t q_local_mac(char* mac, int32_t mac_size)
{
	Q_ASSERT(mac_size>16, "mac address must be greater than 16!");

#ifdef WIN32
	IP_ADAPTER_INFO AdapterInfo[16];
	DWORD dwBufLen=sizeof(AdapterInfo);

	DWORD dwStatus=GetAdaptersInfo(AdapterInfo, &dwBufLen);
	if(dwStatus!=ERROR_SUCCESS)
		return -1;

	PIP_ADAPTER_INFO pAdapterInfo=AdapterInfo;
	do {
		sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", pAdapterInfo->Address[0], pAdapterInfo->Address[1], \
				pAdapterInfo->Address[2], pAdapterInfo->Address[3], \
				pAdapterInfo->Address[4], pAdapterInfo->Address[5]);
		pAdapterInfo=pAdapterInfo->Next;
#ifndef ALL_ADAPTER
		break;
#endif
	} while(pAdapterInfo);
	return 0;
#else
	struct ifreq ifr;
	Q_SOCKET_T sock;

	sock=socket(AF_INET, SOCK_STREAM, 0);
	if(sock<0) {
		Q_DEBUG("q_local_mac: create socket failure!");
		return -1;
	}

	strcpy(ifr.ifr_name, "eth0");
	if(ioctl(sock, SIOCGIFHWADDR, &ifr)<0) { 
		q_close_socket(sock);
		Q_DEBUG("q_local_mac: call ioctl failure!");
		return -1;
	}

	for(int32_t i=0; i<6; ++i)
		sprintf(mac+3*i, "%02x:", (uint8_t)ifr.ifr_hwaddr.sa_data[i]);
	mac[17]='\0';
	return 0;
#endif
}

static int32_t q_set_overtime(Q_SOCKET_T in_socket, int32_t in_time)
{
#ifdef WIN32
	if(setsockopt(in_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&in_time, sizeof(in_time))<0) {
		Q_DEBUG("q_set_overtime: set recv timeout error!");
		return -1;
	}
	if(setsockopt(in_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&in_time, sizeof(in_time))<0) {
		Q_DEBUG("q_set_overtime: set send timeout error!");
		return -1;
	}
#else
	struct timeval over_time;
	over_time.tv_sec=in_time/1000;
	over_time.tv_usec=(in_time%1000)*1000;

	if(setsockopt(in_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&over_time, sizeof(over_time))<0) {
		Q_DEBUG("q_set_overtime: set recv timeout error!");
		return -1;
	}
	if(setsockopt(in_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&over_time, sizeof(over_time))<0) {
		Q_DEBUG("q_set_overtime: setsockopt SO_SNDTIMEO error!");
		return -1;
	}
#endif
	return 0;
}

static int32_t q_set_nonblocking(Q_SOCKET_T in_socket)
{
#ifdef WIN32
	uint64_t flag;
	if(ioctlsocket(in_socket, FIONBIO, (u_long*)&flag)==SOCKET_ERROR)
		return -1;
#else
	int32_t flag;
	flag=fcntl(in_socket, F_GETFL);
	if(flag==-1)
		return -1;
	if(fcntl(in_socket, F_SETFL, flag|O_NONBLOCK)<0)
		return -1;
#endif
	return 0;
}

static int32_t q_set_keepalive(Q_SOCKET_T in_socket, int32_t keepalive_time, int32_t keepalive_interval)
{
#ifdef WIN32
	BOOL bKeepAlive=TRUE;
	tcp_keepalive alive_in, alive_out;
	alive_in.keepalivetime=keepalive_time;
	alive_in.keepaliveinterval=keepalive_interval;
	alive_in.onoff=TRUE;
	unsigned long ulBytesReturn=0;

	if(setsockopt(in_socket, SOL_SOCKET, SO_KEEPALIVE, (char*)&bKeepAlive, sizeof(bKeepAlive))<0) {
		Q_DEBUG("q_set_keepalive: setsockopt SO_KEEPALIVE error!");
		return -1;
	}
	if(WSAIoctl(in_socket, SIO_KEEPALIVE_VALS, &alive_in, sizeof(alive_in), &alive_out, sizeof(alive_out), &ulBytesReturn, NULL, NULL)<0) {
		Q_DEBUG("q_set_keepalive: WSAIoctl error!");
		return -1;
	}
	return 0;
#else
	int32_t iKeepAlive=1;
	int32_t iKeepIdle=keepalive_time/1000;
	if(iKeepIdle==0)
		iKeepIdle=1;
	int32_t iKeepInterval=keepalive_interval/1000;
	if(iKeepInterval==0)
		iKeepInterval=1;
	int32_t iKeepCount=3;

	if(setsockopt(in_socket, SOL_SOCKET, SO_KEEPALIVE, (void*)&iKeepAlive, sizeof(iKeepAlive))<0) {
		Q_DEBUG("q_set_keepalive: setsockopt SO_KEEPALIVE error!");
		return -1;
	}
	// Default setting are more or less garbage, with the keepalive time set to 7200 by default on Linux.
	// Modify setting to make the feature actually useful.
	if(setsockopt(in_socket, SOL_TCP, TCP_KEEPIDLE, (void*)&iKeepIdle, sizeof(iKeepIdle))<0) {
		Q_DEBUG("q_set_keepalive: setsockopt TCP_KEEPIDLE error!");
		return -1;
	}
	if(setsockopt(in_socket, SOL_TCP, TCP_KEEPINTVL, (void*)&iKeepInterval, sizeof(iKeepInterval))<0) {
		Q_DEBUG("q_set_keepalive: setsockopt TCP_KEEPINTVL error!");
		return -1;
	}
	if(setsockopt(in_socket, SOL_TCP, TCP_KEEPCNT, (void*)&iKeepCount, sizeof(iKeepCount))<0) {
		Q_DEBUG("q_set_keepalive: setsockopt TCP_KEEPCNT error!");
		return -1;
	}
	return 0;
#endif
}

static int32_t q_set_nodelay(Q_SOCKET_T in_socket)
{
	int32_t flag=1;
#ifdef WIN32
	if(setsockopt(in_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag))<0) {
		Q_DEBUG("q_set_nodelay: setsockopt TCP_NODELAY error!");
		return -1;
	}
#else
	if(setsockopt(in_socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag))<0) {
		Q_DEBUG("q_set_nodelay: setsockopt TCP_NODELAY error!");
		return -1;
	}
	return 0;
#endif
}

static int32_t q_TCP_server(Q_SOCKET_T& in_listen, uint16_t in_port, int32_t backlog=511)
{
	struct sockaddr_in my_server_addr;
	my_server_addr.sin_family=AF_INET;
	my_server_addr.sin_port=htons(in_port);
	my_server_addr.sin_addr.s_addr=htons(INADDR_ANY);

	in_listen=::socket(AF_INET, SOCK_STREAM, 0);
	if(in_listen<0) {
		Q_DEBUG("q_listen_socket: create socket failure!");
		return -1;
	}
#ifdef WIN32
	BOOL bReuse=TRUE;
	setsockopt(in_listen, SOL_SOCKET, SO_REUSEADDR, (char*)&bReuse, sizeof(BOOL));
#else
	int32_t reuse=1;
	setsockopt(in_listen, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
#endif
	if(bind(in_listen, (struct sockaddr*)&my_server_addr, sizeof(my_server_addr))<0) {
		q_close_socket(in_listen);
		Q_DEBUG("q_listen_socket: bind to port %d failure!", in_port);
		return -1;
	}
	if(listen(in_listen, backlog)<0) {
		q_close_socket(in_listen);
		Q_DEBUG("q_listen_socket: listen to port %d failure!", in_port);
		return -1;
	}
	return 0;
}

static int32_t q_UDP_server(Q_SOCKET_T& in_listen, uint16_t in_port)
{
	struct sockaddr_in my_server_addr;
	my_server_addr.sin_family=AF_INET;
	my_server_addr.sin_port=htons(in_port);
	my_server_addr.sin_addr.s_addr=htons(INADDR_ANY);

	in_listen=::socket(AF_INET, SOCK_DGRAM, 0);
	if(in_listen<0) {
		Q_DEBUG("q_listen_udp_socket: create udp socket failure!");
		return -1;
	}
	if(bind(in_listen, (struct sockaddr*)&my_server_addr, sizeof(my_server_addr))<0) {
		q_close_socket(in_listen);
		Q_DEBUG("q_listen_udp_socket: bind to port %d failure!", in_port);
		return -1;
	}
	return 0;
}

static int32_t q_connect_socket(Q_SOCKET_T& in_socket, char* in_ip, uint16_t in_port)
{
	struct sockaddr_in my_server_addr;
	my_server_addr.sin_family=AF_INET;
	my_server_addr.sin_port=htons(in_port);
	my_server_addr.sin_addr.s_addr=inet_addr(in_ip);

	in_socket=::socket(AF_INET, SOCK_STREAM, 0);
	if(in_socket<0) {
		Q_DEBUG("q_connect_socket: create socket failure!");
		return -1;
	}
	if(connect(in_socket, (struct sockaddr*)&my_server_addr, sizeof(my_server_addr))<0) {
		q_close_socket(in_socket);
		Q_DEBUG("q_connect_socket: connect %s:%d failure!", in_ip, in_port);
		return -1;
	}
	return 0;
}

static int32_t q_connect_socket_retry(Q_SOCKET_T& in_socket, char* in_ip, uint16_t in_port)
{
	struct sockaddr_in my_server_addr;
	my_server_addr.sin_family=AF_INET;
	my_server_addr.sin_port=htons(in_port);
	my_server_addr.sin_addr.s_addr=inet_addr(in_ip);

	// 指数补偿(exponential backoff), 最大延迟2分钟左右
	int32_t max_sleep=1<<7;
	for(int32_t secs=1; secs<=max_sleep; secs<<=1) {
		in_socket=::socket(AF_INET, SOCK_STREAM, 0);
		if(in_socket<0) {
			Q_DEBUG("q_connect_socket: create socket failure!");
			return -1;
		}
		if(connect(in_socket, (struct sockaddr*)&my_server_addr, sizeof(my_server_addr))<0) {
			Q_DEBUG("q_connect_socket: connect %s:%d failure!", in_ip, in_port);
			q_close_socket(in_socket);
			if(secs<=max_sleep/2)
				q_sleep(secs*1000);
			continue;
		}
		return 0;
	}

	return -1;
}

static int32_t q_accept_socket(Q_SOCKET_T in_listen, Q_SOCKET_T& out_socket)
{
	struct sockaddr_in my_client_addr;
#ifdef WIN32
	int32_t length=sizeof(my_client_addr);
#else
	socklen_t length=sizeof(my_client_addr);
#endif
	out_socket=::accept(in_listen, (struct sockaddr*)&my_client_addr, &length);
	if(out_socket==-1)
		return -1;
	return 0;
}

static int32_t q_accept_socket(Q_SOCKET_T in_listen, Q_SOCKET_T& out_socket, char* ip, int32_t& port)
{
	struct sockaddr_in my_client_addr;
#ifdef WIN32
	int32_t length=sizeof(my_client_addr);
#else
	socklen_t length=sizeof(my_client_addr);
#endif
	out_socket=::accept(in_listen, (struct sockaddr*)&my_client_addr, &length);
	if(out_socket==-1)
		return -1;
	sprintf(ip, "%s", inet_ntoa(my_client_addr.sin_addr));
	port=my_client_addr.sin_port;
	return 0;
}

static int32_t q_recvbuf(Q_SOCKET_T in_socket, char* in_buffer, int32_t in_buflen)
{
	int32_t retval=0, finlen=0;
	do {
		retval=(int32_t)::recv(in_socket, in_buffer+finlen, in_buflen-finlen, 0);
		if(retval>0) finlen+=retval;
		if(retval==-1&&(errno==EAGAIN||errno==EWOULDBLOCK))
			continue;
	} while(retval>0&&finlen<in_buflen);
	if(retval<0||finlen<in_buflen)
		return -1;
	return 0;
}

static int32_t q_recvbuf_onebyte(Q_SOCKET_T in_socket, char end_char, char* in_buffer, int32_t& in_buflen)
{
	in_buflen=0;
	int32_t once_len=0;
	for(;;) {
		once_len=(int32_t)recv(in_socket, in_buffer+in_buflen, 1, 0);
		if(once_len<=0) {
			if(once_len==-1&&(errno==EAGAIN||errno==EWOULDBLOCK))
				continue;
			break;
		}
		in_buflen+=once_len;
		if(in_buffer[in_buflen-1]==end_char)
			break;
	}
	if(once_len<=0)
		return -1;
	in_buflen--;
	return 0;
}

static int32_t q_recvbuf_twobytes(Q_SOCKET_T in_socket, char end_one, char end_two, char* in_buffer, int32_t& in_buflen)
{
	in_buflen=0;
	int32_t once_len=0;
	for(;;) {
		once_len=(int32_t)recv(in_socket, in_buffer+in_buflen, 1, 0);
		if(once_len<=0) {
			if(once_len==-1&&(errno==EAGAIN||errno==EWOULDBLOCK))
				continue;
			break;
		}
		in_buflen+=once_len;
		if(in_buflen>=2&&in_buffer[in_buflen-2]==end_one&&in_buffer[in_buflen-1]==end_two)
			break;
	}
	if(once_len<=0)
		return -1;
	in_buflen-=2;
	return 0;
}

static int32_t q_sendbuf(Q_SOCKET_T in_socket, char* in_buffer, int32_t in_buflen)
{
	int32_t retval=0, finlen=0;
	do {
		retval=(int32_t)::send(in_socket, in_buffer+finlen, in_buflen-finlen, 0);
		if(retval>0) finlen+=retval;
		if(retval==-1&&(errno==EAGAIN||errno==EWOULDBLOCK))
			continue;
	} while(retval>0&&finlen<in_buflen);
	if(retval<0||finlen<in_buflen)
		return -1;
	return 0;
}

static int32_t q_sendfile(Q_SOCKET_T in_socket, char* in_file)
{
	int64_t file_len=q_get_file_size(in_file);
	if(file_len<0) {
		Q_DEBUG("q_sendfile: get file size failure!");
		return -1;
	}

	FILE* fp=fopen(in_file, "rb");
	if(fp==NULL) {
		Q_DEBUG("q_sendfile: open file %s failure!", in_file);
		return -1;
	}

	if(q_sendbuf(in_socket, (char*)&file_len, 4)) {
		Q_DEBUG("q_sendfile: send file size failure!");
		fclose(fp);
		return -1;
	}

	char send_buf[3<<10];
	int64_t send_len=0;

	while(file_len>0) {
		send_len=sizeof(send_buf);
		if(file_len<send_len)
			send_len=file_len;

		if(fread(send_buf, send_len, 1, fp)!=1) {
			Q_DEBUG("q_sendfile: read file %s error!", in_file);
			fclose(fp);
			return -1;
		}

		if(q_sendbuf(in_socket, send_buf, send_len)) {
			Q_DEBUG("q_sendfile: send file content failure!");
			fclose(fp);
			return -1;
		}

		file_len-=send_len;
	}

	fclose(fp);
	return 0;
}

// 非复制类
class noncopyable
{
	protected:
		noncopyable() {}
		virtual ~noncopyable() {}

	private:
		noncopyable(const noncopyable&);
		noncopyable& operator=(const noncopyable&);
};

// 字符串缓冲区
class QStringBuffer {
	public:
		QStringBuffer(int32_t size=DEFAULT_BUFFER_SIZE) :
			length_(0),
			max_size_(size+1)
		{
			buffer_=q_new_array<char>(max_size_);
			Q_CHECK_PTR(buffer_);
			buffer_[0]=0;
		}

		QStringBuffer(const char* str, int32_t len=-1)
		{
			if(len==-1) len=strlen(str);
			max_size_=(len>DEFAULT_BUFFER_SIZE)?(len+1):DEFAULT_BUFFER_SIZE;
			buffer_=q_new_array<char>(max_size_);
			Q_CHECK_PTR(buffer_);
			length_=len;
			strncpy(buffer_, str, len);
		}

		virtual ~QStringBuffer()
		{q_delete_array<char>(buffer_);}

		inline int32_t max_size() const
		{return max_size_;}

		inline int32_t length() const
		{return length_;}

		inline void clear()
		{
			buffer_[0]='\0';
			length_=0;
		}

		void append(const char character)
		{
			if(length_+1>max_size_)
				growBuffer(length_+1);
			buffer_[length_++]=character;
			buffer_[length_]='\0';
		}

		void append(const char* str, int32_t len=-1)
		{
			len=(len==-1)?strlen(str):len;
			if(length_+len+1>max_size_)
				growBuffer(length_+len+1);
			strncpy(buffer_+length_, str, len);
			length_+=len;
		}

		void append(const int32_t value)
		{
			char buf_[30];
			q_snprintf(buf_, sizeof(buf_)-1, "%d", value);
			append(buf_);
		}

		void append(const int64_t value)
		{
			char buf_[30];
			q_snprintf(buf_, sizeof(buf_)-1, "%ld", value);
			append(buf_);
		}

		char* getBuffer()
		{return buffer_;}

	private:
		void growBuffer(int32_t size)
		{
			max_size_<<=1;
			max_size_=(max_size_<size)?size:max_size_;
			char* new_buffer_=q_new_array<char>(max_size_);
			Q_CHECK_PTR(new_buffer_);
			strncpy(new_buffer_, buffer_, length_);
			q_delete_array<char>(buffer_);
			buffer_=new_buffer_;
		}

	protected:
		enum {DEFAULT_BUFFER_SIZE=1<<10};
		char* buffer_;
		int32_t length_;
		int32_t max_size_;
};

// 互斥锁
class QMutexLock: public noncopyable
{
	public:
		inline QMutexLock()
		{
#ifdef WIN32
			InitializeCriticalSection(&mutex);
#else
			pthread_mutex_init(&mutex, NULL);
#endif
		}

		virtual ~QMutexLock()
		{
#ifdef WIN32
			DeleteCriticalSection(&mutex);
#else
			pthread_mutex_destroy(&mutex);
#endif
		}

		inline void lock()
		{
#ifdef WIN32
			EnterCriticalSection(&mutex);
#else
			pthread_mutex_lock(&mutex);
#endif
		}

		inline void unlock()
		{
#ifdef WIN32
			LeaveCriticalSection(&mutex);
#else
			pthread_mutex_unlock(&mutex);
#endif
		}

	protected:
#ifdef WIN32
		CRITICAL_SECTION mutex;
#else
		pthread_mutex_t mutex;
#endif
};

class QScopeMutex: public noncopyable
{
	public:
		QScopeMutex(QMutexLock& mutex) : 
			mtx(&mutex)
		{mtx->lock();}

		virtual ~QScopeMutex()
		{mtx->unlock();}

	protected:
		QMutexLock* mtx;
};

// 自旋锁
class QSpinLock: public noncopyable
{
	public:
		inline QSpinLock()
		{
#ifdef WIN32
			InterlockedExchange(&spinlock, 0);
#else
			pthread_spin_init(&spinlock, 0);
#endif
		}

		virtual ~QSpinLock()
		{
#ifdef WIN32
#else
			pthread_spin_destroy(&spinlock);
#endif
		}

		inline void lock()
		{
#ifdef WIN32
			while(InterlockedExchange(&spinlock, 1)!=0)
				q_sleep(1);
#else
			pthread_spin_lock(&spinlock);
#endif
		}

		inline void unlock()
		{
#ifdef WIN32
			InterlockedExchange(&spinlock, 0);
#else
			pthread_spin_unlock(&spinlock);
#endif
		}

	protected:
#ifdef WIN32
		volatile uint32_t spinlock;
#else
		pthread_spinlock_t spinlock;
#endif
};

class QScopeSpin: public noncopyable
{
	public:
		QScopeSpin(QSpinLock& spinlock) : 
			spin(&spinlock)
		{spin->lock();}

		virtual ~QScopeSpin()
		{spin->unlock();}

	protected:
		QSpinLock* spin;
};

// 触发器
class QTrigger: public noncopyable
{
	public:
#ifdef WIN32
		inline QTrigger(bool autoreset=true, bool state=false)
		{
			handle=CreateEvent(0, !autoreset, state, 0);
			Q_ASSERT(handle!=0, "QTrigger: handle = (%d)", handle);
		}
#else
		inline QTrigger(bool autoreset=true, bool state=false) :
			m_autoreset(autoreset),
			m_state(state)
		{
			pthread_mutex_init(&mtx, NULL);
			pthread_cond_init(&cond, NULL);
		}
#endif

		virtual ~QTrigger()
		{
#ifdef WIN32
			CloseHandle(handle);
#else
			pthread_cond_destroy(&cond);
			pthread_mutex_destroy(&mtx);
#endif
		}

		void signal()
		{
#ifdef WIN32
			SetEvent(handle);
#else
			pthread_mutex_lock(&mtx);
			m_state=1;
			if(m_autoreset)
				pthread_cond_signal(&cond);
			else
				pthread_cond_broadcast(&cond);
			pthread_mutex_unlock(&mtx);
#endif
		}

		inline void wait()
		{
#ifdef WIN32
			WaitForSingleObject(handle, INFINITE);
#else
			pthread_mutex_lock(&mtx);
			while(m_state==0)
				pthread_cond_wait(&cond, &mtx);
			if(m_autoreset)
				m_state=0;
			pthread_mutex_unlock(&mtx);
#endif
		}

		inline void reset()
		{
#ifdef WIN32
			ResetEvent(handle);
#else
			m_state=0;
#endif
		}

	protected:
#ifdef WIN32
		HANDLE handle;
#else
		pthread_mutex_t mtx;
		pthread_cond_t cond;
		bool m_autoreset;
		int32_t m_state;
#endif
};

// 信号量
class QTimedSem: public noncopyable
{
	public:
		QTimedSem(int32_t val=0)
		{
#ifdef WIN32
			handle=CreateSemaphore(NULL, val, 65535, NULL);
#else
			count=val;
			pthread_mutex_init(&mtx, 0);
			pthread_cond_init(&cond, 0);
#endif
		}

		virtual ~QTimedSem()
		{
#ifdef WIN32
			CloseHandle(handle);
#else
			pthread_cond_destroy(&cond);
			pthread_mutex_destroy(&mtx);
#endif
		}

		bool wait(int32_t timeout=-1)
		{
#ifdef WIN32
			uint32_t rc=WaitForSingleObject(handle, timeout);
			return (rc!=WAIT_FAILED&&rc!=WAIT_TIMEOUT);
#else
			pthread_mutex_lock(&mtx);
			while(count<=0) { 
				if(timeout>=0) {
					timespec abs_ts; 
					timeval cur_tv;
					gettimeofday(&cur_tv, NULL);
					abs_ts.tv_sec=cur_tv.tv_sec+timeout/1000; 
					abs_ts.tv_nsec=cur_tv.tv_usec*1000+(timeout%1000)*1000000;
					int32_t rc=pthread_cond_timedwait(&cond, &mtx, &abs_ts);
					if(rc==ETIMEDOUT) { 
						pthread_mutex_unlock(&mtx);
						return false;
					}
				} else {
					pthread_cond_wait(&cond, &mtx);
				}
			} 
			count--;
			pthread_mutex_unlock(&mtx);
			return true;
#endif
		}

		void post()
		{
#ifdef WIN32
			ReleaseSemaphore(handle, 1, NULL);
#else
			pthread_mutex_lock(&mtx);
			count++; 
			pthread_cond_signal(&cond);
			pthread_mutex_unlock(&mtx);
#endif
		}

		void reset()
		{
#ifdef WIN32
#else
			count=0;
#endif
		}

	protected:
#ifdef WIN32
		HANDLE handle;
#else
		pthread_mutex_t mtx;
		pthread_cond_t cond;
		int32_t count;
#endif
};

// 读写锁
class QRWLock: public noncopyable
{
	public:
		inline QRWLock()
		{
#ifdef WIN32
			InitializeCriticalSection(&mtx);
			read_cnt=0;
			write_flg=0;
#else
			pthread_rwlock_init(&mtx, NULL);
#endif
		}

		virtual ~QRWLock()
		{
#ifdef WIN32
			DeleteCriticalSection(&mtx);
#else
			pthread_rwlock_destroy(&mtx);
#endif
		}

		inline void rdlock()
		{
#ifdef WIN32
			for(;;) {
				EnterCriticalSection(&mtx);
				if(write_flg!=0) {
					LeaveCriticalSection(&mtx);
					q_sleep(1);
					continue;
				}
				read_cnt++;
				LeaveCriticalSection(&mtx);
				break;
			}
#else
			pthread_rwlock_rdlock(&mtx);
#endif
		}

		inline void wrlock()
		{
#ifdef WIN32
			for(;;) {
				EnterCriticalSection(&mtx);
				if(write_flg!=0||read_cnt!=0) {
					LeaveCriticalSection(&mtx);
					q_sleep(1);
					continue;
				}
				write_flg=1;
				LeaveCriticalSection(&mtx);
				break;
			}
#else
			pthread_rwlock_wrlock(&mtx);
#endif
		}

		inline void unlock()
		{
#ifdef WIN32
			EnterCriticalSection(&mtx);
			if(write_flg!=0) {
				write_flg=0;
			} else {
				if(read_cnt<=0)
					Q_DEBUG("QRWLock unlock error!");
				else
					read_cnt--;
			}
			LeaveCriticalSection(&mtx);
#else
			pthread_rwlock_unlock(&mtx);
#endif
		}

	protected:
#ifdef WIN32
		CRITICAL_SECTION mtx;
		uint32_t read_cnt;
		uint32_t write_flg;
#else
		pthread_rwlock_t mtx;
#endif
};

class QRWLockFunc: protected QMutexLock
{
	public:
#ifdef WIN32
#else
		QRWLockFunc(): locks(0), writers(0), readers(0)
		{
			pthread_mutex_init(&mtx, 0);
			pthread_cond_init(&readcond, 0);
			pthread_cond_init(&writecond, 0);
		}
#endif

		virtual ~QRWLockFunc()
		{
#ifdef WIN32
#else
			pthread_cond_destroy(&writecond);
			pthread_cond_destroy(&readcond);
			pthread_mutex_destroy(&mtx);
#endif
		}

		void rdlock()
		{
#ifdef WIN32
#else
			pthread_mutex_lock(&mtx);
			readers++;
			while(locks<0)
				pthread_cond_wait(&readcond, &mtx);
			readers--;
			locks++;
			pthread_mutex_unlock(&mtx);
#endif
		}

		void wrlock()
		{
#ifdef WIN32
#else
			pthread_mutex_lock(&mtx);
			writers++;
			while(locks!=0)
				pthread_cond_wait(&writecond, &mtx);
			locks=-1;
			writers--;
			pthread_mutex_unlock(&mtx);
#endif
		}

		void unlock()
		{
#ifdef WIN32
#else
			pthread_mutex_lock(&mtx);
			if(locks>0) {
				locks--;
				if(locks==0)
					pthread_cond_signal(&writecond);
			} else {
				locks=0;
				if(readers!=0)
					pthread_cond_broadcast(&readcond);
				else
					pthread_cond_signal(&writecond);
			}
			pthread_mutex_unlock(&mtx);
#endif
		}

	protected:
#ifdef WIN32
#else
		pthread_mutex_t mtx;
		pthread_cond_t readcond;
		pthread_cond_t writecond;
		int32_t locks;
		int32_t writers;
		int32_t readers;
#endif
};

class QScopeRead: public noncopyable
{
	public:
		QScopeRead(QRWLock& irw) :
			rw(&irw)
		{rw->rdlock();}

		virtual ~QScopeRead()
		{rw->unlock();}

	protected:
		QRWLock* rw;
};

class QScopeWrite: public noncopyable
{
	public:
		QScopeWrite(QRWLock& irw):
			rw(&irw)
		{rw->wrlock();}

		virtual ~QScopeWrite()
		{rw->unlock();}

	protected:
		QRWLock* rw;
};

// 运行时间
class QStopwatch: public noncopyable
{
	private:
#ifdef WIN32
		DWORD dwTime_1, dwTime_2;
#else
		struct timeval tv_1, tv_2;
#endif

	public:
		inline void start()
		{
#ifdef WIN32
			dwTime_1=GetTickCount();
#else
			gettimeofday(&tv_1, 0);
#endif
		}

		inline void stop()
		{
#ifdef WIN32
			dwTime_2=GetTickCount();
#else
			gettimeofday(&tv_2, 0);
#endif
		}

		inline int32_t elapsed_s()
		{
#ifdef WIN32
			return (dwTime_2-dwTime_1)/1000;
#else
			return (int32_t)((tv_2.tv_sec-tv_1.tv_sec)+(tv_2.tv_usec-tv_1.tv_usec)/1000000);
#endif
		}

		inline int32_t elapsed_ms()
		{
#ifdef WIN32
			return dwTime_2-dwTime_1;
#else
			return (int32_t)((tv_2.tv_sec-tv_1.tv_sec)*1000+(tv_2.tv_usec-tv_1.tv_usec)/1000);
#endif
		}

		inline int32_t elapsed_us()
		{
#ifdef WIN32
			return dwTime_2-dwTime_1;
#else
			return (int32_t)((tv_2.tv_sec-tv_1.tv_sec)*1000000+(tv_2.tv_usec-tv_1.tv_usec));
#endif
		}

		static int64_t elapsed()
		{
#ifdef WIN32
			return GetTickCount();
#else
			struct timeval tv_T;
			gettimeofday(&tv_T, 0);
			return tv_T.tv_sec*1000000+tv_T.tv_usec;
#endif
		}

		inline void reset()
		{
#ifdef WIN32
			dwTime_1=0;
			dwTime_2=0;
#else
			memset(&tv_1, 0, sizeof(struct timeval));
			memset(&tv_2, 0, sizeof(struct timeval));
#endif
		}
};

// 异步信号触发器
class QWatchdog: public noncopyable
{
	public:
		QWatchdog(int32_t now=0, int32_t interval=20000) :
			now_(now),
			interval_(interval)
		{
#ifdef WIN32
			timer_id_=NULL;
#else
			memset(&tick_, 0, sizeof(tick_));
#endif
		}

		virtual ~QWatchdog()
		{
#ifdef WIN32
			timeKillEvent(timer_id_);
#else
#endif
		}

		void set_now(int32_t now)
		{now_=now;}

		void set_interval(int32_t interval)
		{interval_=interval;}

#ifdef WIN32
		int32_t start(void (*sig_handler)(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dwl, DWORD dw2))
		{
			q_sleep(now_);
			timer_id_=timeSetEvent(interval_, 1, (LPTIMECALLBACK)sig_handler, DWORD(this), TIME_PERIODIC);
			if(timer_id_==NULL) {
				Q_DEBUG("QWatchdog: signal error!");
				return -1;
			}
			return 0;
		}
#else
		int32_t start(void (*sig_handler)(int32_t signo))
		{
			struct sigaction act, oact;
			act.sa_handler=sig_handler;
			act.sa_flags=0;
			if(sigemptyset(&act.sa_mask)==-1)
				return -1;

			if(::sigaction((int32_t)SIGALRM, &act, &oact)==-1) {
				Q_DEBUG("QWatchdog: sigaction error!");
				return -1;
			}

			tick_.it_value.tv_sec=now_/1000;
			tick_.it_value.tv_usec=now_%1000*1000;

			tick_.it_interval.tv_sec=interval_/1000;
			tick_.it_interval.tv_usec=interval_%1000*1000;

			// Decrements in real time, and delivers SIGALRM upon expiration.
			if(::setitimer(ITIMER_REAL, &tick_, NULL)==-1) {
				Q_DEBUG("QWatchdog: settimer error!");
				return -1;
			}

			return 0;
		}
#endif

	protected:
		int32_t now_;
		int32_t interval_;
#ifdef WIN32
		MMRESULT timer_id_;
#else
		struct itimerval tick_;
#endif
};

// 随机数发生器
class QRandom: public noncopyable
{
	public:
		inline QRandom()
		{
#ifdef WIN32
			srand(static_cast<uint32_t>(GetTickCount()^_getpid()));
#else
			srand(static_cast<uint32_t>(time(NULL)^getpid()));
#endif
		}

		int32_t get_random_int32(int32_t min, int32_t max)
		{
			return (rand()%static_cast<int32_t>(max+1-min))+min;
		}

		int64_t get_random_int64(int64_t min, int64_t max)
		{
			if(max==min) return min;
			return (min+(((int64_t)rand()<<16)%((uint64_t)(max-min)+1)));
		}

		double get_random_float01()
		{
			return (rand()/(RAND_MAX+1.0));
		}

		void get_random_buffer(char* buf, int32_t len, const char* charset)
		{
			int32_t in_i=0, out_i=0;
			int32_t i32;
			uint8_t *in;
			size_t set_len=strlen(charset);

			Q_ASSERT(set_len>0&&set_len<=256, "charset length out of range");

			while(out_i<len) {
				i32=get_random_int32(0, MAX_INT32);
				in=(uint8_t*)&i32;

				in_i=0;
				buf[out_i++]=charset[in[in_i]%set_len];
				if(out_i==len)
					break;

				in_i++;
				buf[out_i++]=charset[in[in_i]%set_len];
				if(out_i==len)
					break;

				in_i++;
				buf[out_i++]=charset[in[in_i]%set_len];
				if(out_i==len)
					break;

				in_i++;
				buf[out_i++]=charset[in[in_i]%set_len];
			}
		}
};

Q_END_NAMESPACE

#endif // __QGLOBAL_H_
