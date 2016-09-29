/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qfunc.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2013/11/05
**
*********************************************************************************************/

#ifndef __QFUNC_H_
#define __QFUNC_H_

#include "qglobal.h"

Q_BEGIN_NAMESPACE

static inline void q_strcat(char* src, char* dest)
{
	while(*src) src++;
	while((*src++=*dest++));
}

static inline int32_t q_strcmp(char* src, char* dest)
{
	for(; *src==*dest; src++, dest++)
		if(!*src) return 0;
	return *src-*dest;
}

static inline void q_strcpy(char*src, char* dest)
{
	while((*src++=*dest++));
}

static inline int32_t q_strlen(char* src)
{
	register char *s=src;
	while(*s++);
	return (s-src-1);
}

static inline void q_strrev(char* beg, char* end)
{
	register char ch(0);
	for(end--; beg<end; beg++, end--) {
		ch=*beg;
		*beg=*end;
		*end=ch;
	}
}

static inline int32_t q_trim_skip(char* str, int32_t len, char*& beg, char*& end)
{
	register char* beg_pos=str;
	register char* end_pos=str+len-1;

	while(beg_pos<=end_pos&&(*beg_pos=='\t'||*beg_pos=='\n'||*beg_pos=='\r'||*beg_pos==' '))
		beg_pos++;
	if(beg_pos>end_pos)
		return -1;

	while(beg_pos<=end_pos&&(*end_pos=='\t'||*end_pos=='\n'||*end_pos=='\r'||*end_pos==' '))
		end_pos--;
	if(beg_pos>end_pos)
		return -1;
	
	beg=beg_pos;
	end=end_pos;

	return 0;
}

static inline int32_t q_right_trim(char* str, int32_t len)
{
	if(NULL==str||len<=0)
		return 0;
	while(len>0&&(str[len-1]=='\t'||str[len-1]=='\n'||str[len-1]=='\r'||str[len-1]==' '))
		len--;
	str[len]=0;
	return len;
}

static inline char* q_trim(char* str, int32_t len)
{
	register char* beg_pos=str;
	register char* end_pos=str+len-1;

	while(beg_pos<=end_pos&&(*beg_pos=='\t'||*beg_pos=='\n'||*beg_pos=='\r'||*beg_pos==' '))
		beg_pos++;
	if(beg_pos>end_pos)
		return NULL;

	while(beg_pos<=end_pos&&(*end_pos=='\t'||*end_pos=='\n'||*end_pos=='\r'||*end_pos==' '))
		end_pos--;
	if(beg_pos>end_pos)
		return NULL;
	*(++end_pos)='\0';

	return beg_pos;
}

static inline int32_t q_find_first_of(char* src, int32_t srclen, char c)
{
	register int32_t n;
	for(n=0; (n<srclen)&&(src[n]!=c); n++);
	return (n!=srclen)?n:-1;
}

static inline int32_t q_find_last_of(char* src, int32_t srclen, char c)
{
	register int32_t n;
	for(n=srclen-1; (n>=0)&&(src[n]!=c); n--);
	return n;
}

static inline int32_t q_find(char *src, int32_t srclen, const char *pat, int32_t patlen)
{
	register int32_t i;
	register int32_t j;
	for(i=0; i<=srclen-patlen; i++) {
		for(j=0; j<patlen; j++)
			if(src[i+j]!=pat[j]) break;
		if(j==patlen) return i;
	}
	return -1;
}

// KMP算法实现快速匹配
static inline int32_t q_find_KMP(char* src, int32_t srclen, const char* pat, int32_t patlen, int32_t* next, int32_t k=0)
{
	register int32_t posP=0;
	register int32_t posS=k;
	while(posP<patlen && posS<srclen) {
		if(posP==-1||pat[posP]==src[posS]) {
			posP++;
			posS++;
		} else {
			posP=next[posP];
		}
	}

	if(posP<patlen)
		return -1;
	else
		return posS-patlen;
}

static inline void q_next_KMP(char* src, int32_t srclen, int32_t* next)
{
	register int32_t i=0;
	register int32_t j=-1;

	next[0]=-1;
	while(i<srclen) {
		if(j==-1||src[i]==src[j]) {
			++i;
			++j;
			next[i]=j;
		} else {
			j=next[j];
		}
	}
}

static inline char *q_strchr(char *src, int32_t srclen, char c)
{
	if(src==NULL||srclen==0)
		return NULL;
	register char* p=src;
	register char* pend=p+srclen;
	for(; (p<pend)&&(*p!=c); p++);
	return (p!=pend)?p:NULL;
}

static inline char *q_strstr(char* src, int32_t srclen, const char* pat, int32_t patlen)
{
	if(src==NULL||srclen<=0||pat==NULL||patlen<=0)
		return NULL;
	register char* p=src;
	register char* pend=p+srclen-patlen;
	while(p<=pend) {
		if(*p==*pat) {
			if(memcmp(p, pat, patlen)==0)
				return (char*)p;
		}
		p++;
	}
	return NULL;
}

static inline bool q_starts_with(const char* src, int32_t srclen, const char* pat, int32_t patlen)
{
	if(src==NULL||srclen<=0||pat==NULL||patlen<=0||srclen<patlen)
		return false;

	if(memcpy((void*)src, (void*)pat, patlen)==0)
		return true;

	return false;
}

static inline bool q_ends_with(const char* src, int32_t srclen, const char* pat, int32_t patlen)
{
	if(src==NULL||srclen<=0||pat==NULL||patlen<=0||srclen<patlen)
		return false;

	if(memcpy((void*)(src+srclen-patlen), (void*)pat, patlen)==0)
		return true;

	return false;
}

static inline char* q_to_lower(char* psz_buf)
{
	if(psz_buf==NULL)
		return psz_buf;

	register char* p=psz_buf;
	while(*p) {
		if((*p)>='A'&&(*p)<='Z')
			(*p)+=32;
		p++;
	}
	return psz_buf;
}

static inline char* q_to_upper(char* psz_buf)
{
	if(psz_buf==NULL)
		return psz_buf;

	register char* p=psz_buf;
	while(*p) {
		if((*p)>='a'&&(*p)<='z')
			(*p)-=32;
		p++;
	}
	return psz_buf;
}

static inline bool q_is_integer(const char* p)
{
	if(p==NULL||(*p)=='\0')
		return false;
	if((*p)=='-') p++;
	while((*p)) {
		if((*p)<'0'||(*p)>'9') return false;
		p++;
	}
	return true;
}

static inline int32_t q_stoi(char *src, int32_t srclen)
{
	register int32_t n;
	register char *pPos=src;
	for(n=0; (pPos<src+srclen)&&(*pPos>='0')&&(*pPos<='9'); ++pPos)
		n=10*n+(*pPos-'0');
	return n;
}

static inline uint32_t q_stoui(char *src, int32_t srclen)
{
	register uint32_t n;
	register char *pPos=src;
	for(n=0; (pPos<src+srclen)&&(*pPos>='0')&&(*pPos<='9'); ++pPos)
		n=10*n+(*pPos-'0');
	return n;
}

static inline int64_t q_stol(char* src, int32_t srclen)
{
	register int64_t val=0;
	register int32_t max=srclen>22?22:srclen;
	for(int32_t i=0; i<max; ++i) {
		if(src[i]<'0'||src[i]>'9')
			break;
		val*=10;
		val+=(src[i]-'0');
	}
	return val;
}

static inline uint64_t q_stoull(char* src, int32_t srclen)
{
	register uint64_t val=0;
	register int32_t max=srclen>22?22:srclen;
	for(int32_t i=0; i<max; ++i) {
		if(src[i]<'0'||src[i]>'9')
			break;
		val*=10;
		val+=(src[i]-'0');
	}
	return val;
}

static inline bool q_stof(char* src, float* value)
{
	return ((q_sscanf(src, "%f", value)==1)?true:false);
}

static inline bool q_stod(char* src, double* value)
{
	return ((q_sscanf(src, "%lf", value)==1)?true:false);
}

#if 0
static inline int32_t q_to_str(int32_t n, char *src)
{
	register int32_t sign;
	register char *pPos=src;
	if((sign=n)<0) n=-n;
	do {
		*pPos++=n%10+'0';
	} while((n/=10)>0);
	if(sign<0) *pPos++='-';
	q_strrev(src, pPos);
	return pPos-src;
}

static inline int32_t q_to_str(uint32_t n, char *src)
{
	register char *pPos=src;
	do {
		*pPos++=n%10+'0';
	} while((n/=10)>0);
	q_strrev(src, pPos);
	return pPos-src;
}

static inline int32_t q_to_str(uint64_t n, char *src)
{
	register char *pPos=src;
	do {
		*pPos++=n%10+'0';
	} while((n/=10)>0);
	q_strrev(src, pPos);
	return pPos-src;
}
#endif

static inline void q_to_str(int32_t n, char* buffer, int32_t bufferSize)
{
	q_snprintf(buffer, bufferSize, "%d", n);
}

static inline void q_to_str(uint32_t n, char* buffer, int32_t bufferSize)
{
	q_snprintf(buffer, bufferSize, "%u", n);
}

static inline void q_to_str(int64_t n, char* buffer, int32_t bufferSize)
{
	q_snprintf(buffer, bufferSize, "%ld", n);
}

static inline void q_to_str(uint64_t n, char* buffer, int32_t bufferSize)
{
	q_snprintf(buffer, bufferSize, "%lu", n);
}

static inline void q_to_str(float n, char* buffer, int32_t bufferSize)
{
	q_snprintf(buffer, bufferSize, "%f", n);
}

static inline void q_to_str(double n, char* buffer, int32_t bufferSize)
{
	q_snprintf(buffer, bufferSize, "%f", n);
}

static inline int32_t q_strrep_in_place(char* src, int32_t srclen, char* from, int32_t fromlen, char* to, int32_t tolen)
{
	if(src==NULL||srclen<=0||from==NULL||fromlen<=0||to==NULL||tolen<=0||fromlen<tolen)
		return -1;
	register int32_t i, j;
	for(i=0, j=0; i<srclen; ++i, ++j) {
		if(*(src+i)==*from) {
			if(memcmp(src+i, from, fromlen)==0) {
				memcpy(src+j, to, tolen);
				i+=fromlen;
				j+=tolen;
			} else {
				*(src+j)=*(src+i);
			}
		}
		if(j!=i) *(src+j)=*(src+i);
	}
	*(src+j)='\0';
	return j;
}

static inline int32_t q_strrep(char* src, int32_t srclen, char* from, int32_t fromlen, char* to, int32_t tolen, char* dest, int32_t max_size)
{
	if(src==NULL||srclen<=0||from==NULL||fromlen<=0||to==NULL||tolen<=0||dest==NULL||max_size<=0)
		return -1;
	register char *p1=src, *p1end=src+srclen;
	register char *p2=dest, *p2end=dest+max_size;
	register char *p=NULL;
	register int32_t len=0;
	do {
		p=q_strstr(p1, p1end-p1, from, fromlen);
		if(p==NULL) {
			len=p1end-p1;
			if(p2+len>p2end)
				return -2;
			memcpy(p2, p1, len);
			p2+=len;
			break;
		} else {
			len=p-p1;
			if(p2+len+tolen>p2end)
				return -2;
			memcpy(p2, p1, len);
			memcpy(p2+len, to, tolen);
		}
		p1=p+fromlen;
		p2+=len+tolen;
	} while(p!=NULL);
	return p2-dest;
}

static inline int32_t q_strdel(char* src, int32_t srclen, char* str, int32_t len)
{
	if(src==NULL||srclen<=0||str==NULL||len<=0)
		return -1;
	register int32_t i, j;
	for(i=0, j=0; i<srclen; ++i, ++j) {
		if((*(src+i)==*str)&&(memcmp(src+i, str, len)==0))
			i+=len;
		if(j!=i)
			*(src+j)=*(src+i);
	}
	*(src+j)='\0';
	return j;
}

static inline int32_t q_strdel_cdata(char* src, int32_t srclen)
{
	if(src==NULL||srclen<=0)
		return -1;
	register int32_t i, j;
	for(i=0, j=0; i<srclen; ++i, ++j) {
		if((*(src+i)=='<')&&(memcmp(src+i, "<![CDATA[", 9)==0)) {
			i+=9;
		} else if((*(src+i)==']')&&(memcmp(src+i, "]]>", 3)==0)) {
			i+=3;
		} else {
			;
		}
		if(j!=i)
			*(src+j)=*(src+i);
	}
	*(src+j)='\0';
	return j;
}

static inline int32_t q_split(char* src, int32_t srclen, char* sep, int32_t seplen, int32_t pos[], int32_t pos_size)
{
	if(src==NULL||srclen==0||sep==NULL||seplen==0)
		return 0;
	register char *p1=src, *p2=NULL;
	register int32_t n=0;
	Q_FOREVER {
		p2=q_strstr(p1, src+srclen-p1, sep, seplen);
		// 空间不够
		if((n+1)*2>pos_size)
			return -1;
		if(p2!=NULL) {
			pos[2*n]=p1-src;
			pos[2*n+1]=p2-src;
			p2+=seplen;
			p1=p2;
			++n;
		} else {
			pos[2*n]=p1-src;
			pos[2*n+1]=srclen;
			++n;
			break;
		}
	}
	return n;
}

static inline int32_t q_split(char* src, int32_t srclen, char* sep, int32_t seplen, char** dest)
{
	if(src==NULL||srclen==0||sep==NULL||seplen==0)
		return 0;
	register char *p1=src, *p2=NULL;
	register int32_t n=0;
	Q_FOREVER {
		p2=::strstr(p1, sep);
		if(p2!=NULL) {
			memset(p2, 0, seplen);
			*dest++=p1;
			p2+=seplen;
			p1=p2;
			++n;
		} else {
			*dest=p1;
			++n;
			break;
		}
	}
	return n;
}

static inline int32_t q_normalize(char* src, int32_t srclen)
{
	if(src==NULL||srclen<=0)
		return -1;

	// 中英文逗号、句号、问号、感叹号、冒号、分号、圆括号、中括号、空格、顿号
	char *ch_punct=(char*)("，。？！：；（）【】　、");
	char *en_punct=(char*)(",.?!:;()[] ,");

	int32_t i=0, j=0;
	bool flag=false;
	while(i<srclen) {
		flag=false;
		for(int32_t k=0; k!=12; ++k) {
			if((ch_punct[3*k]==*(src+i)) && (memcmp(src+i, ch_punct+3*k, 3)==0)) {
				memcpy(src+j, &en_punct[k], 1);
				i+=3;
				j++;
				flag=true;
				break;
			}
		}
		if(!flag) {
			if(j!=i) *(src+j)=*(src+i);
			i++;
			j++;
		}
	}
	*(src+j)='\0';

	return j;
}

static inline int32_t q_denoise(char* src, int32_t srclen, char* dest, int32_t max_size)
{
	if(src==NULL||srclen<=0||dest==NULL||max_size<=0)
		return -1;
	
	char* ptr=src;
	char* ptr_end=ptr+srclen;
	char* ptr_out=dest;
	char* ptr_out_end=ptr_out+max_size;

	while(ptr<ptr_end&&(*ptr==0x20||*ptr=='\r'||*ptr=='\n'||*ptr=='\t'))
		ptr++;
	while(ptr_end>ptr&&(*(ptr_end-1)==0x20||*(ptr_end-1)=='\r'||*(ptr_end-1)=='\n'||*(ptr_end-1)=='\t'))
		ptr_end--;

	while(ptr<ptr_end) {
		if(*ptr=='<') {
			// 跳过html标签
			char* ptemp=ptr++;
			while(ptr<ptr_end&&*ptr!='>')
				ptr++;
			int32_t len=ptr-ptemp;
			if(ptr_out+len>ptr_out_end)
				return -2;

			if(len==5&&memcmp(ptemp, "</td>", 5)==0) {
				*ptr_out++=0x20;
			} else if(len==5&&memcmp(ptemp, "</tr>", 5)==0) {
				*ptr_out++='\n';
			} else if((len==4&&memcmp(ptemp, "<br>", 4)==0)||(len==5&&memcmp(ptemp, "<br/>", 5)==0) \
					||(len==6&&memcmp(ptemp, "<br />", 6)==0)) {
				*ptr_out++='\n';
			} else {
				;
			}

			if(ptr<ptr_end)
				ptr++;
#if defined (__KILL_CRLF)
		} else if(*ptr==0x20||*ptr==0x09||*ptr=='\r'||*ptr=='\n') {
			// 处理空白字符
			if(ptr_out+1>ptr_out_end)
				return -2;
			*ptr_out++=0x20;
			ptr++;
			while(ptr<ptr_end&&(*ptr==0x20||*ptr==0x09||*ptr=='\r'||*ptr=='\n'))
				ptr++;
#else
		} else if(*ptr==0x20||*ptr==0x09) {
			// 处理空格、TAB键
			if(ptr_out+1>ptr_out_end)
				return -3;
			*ptr_out++=0x20;
			ptr++;
			while(ptr<ptr_end&&(*ptr==0x20||*ptr==0x09))
				ptr++;
		} else if(*ptr=='\r'||*ptr=='\n') {
			// 处理回车换行
			if(ptr_out+1>ptr_out_end)
				return -4;
			*ptr_out++='\n';
			ptr++;
			while(ptr<ptr_end&&(*ptr==0x20||*ptr==0x09||*ptr=='\r'||*ptr=='\n'))
				ptr++;
#endif
		} else if(*ptr=='&') {
			// 处理参照实体
			char* ptemp=ptr++;
			while(ptr<ptr_end&&(*ptr>='a'&&*ptr<='z')&&(*ptr!=';'))
				ptr++;
			if(*ptr==';')
				ptr++;
			int32_t len=ptr-ptemp;
			if(ptr_out+len>ptr_out_end)
				return -5;

			if(len==6&&memcmp(ptemp, "&nbsp;", 6)==0) {
				*ptr_out++=0x20;
			} else if(len==6&&memcmp(ptemp, "&quot;", 6)==0) {
				*ptr_out++='"';
			} else if(len==6&&memcmp(ptemp, "&apos;", 6)==0) {
				*ptr_out++='\'';
			} else if(len==5&&memcmp(ptemp, "&amp;", 5)==0) {
				*ptr_out++='&';
			} else if(len==4&&memcmp(ptemp, "&lt;", 4)==0) {
				*ptr_out++='<';
			} else if(len==4&&memcmp(ptemp, "&gt;", 4)==0) {
				*ptr_out++='<';
			} else {
				memcpy(ptr_out, ptemp, len);
				ptr_out+=len;
			}
		} else {
			// 处理其它字符
			if(ptr_out+1>ptr_out_end)
				return -6;
			*ptr_out++=*ptr++;
		}
	}

	// 去除字符串首尾空格
	char* beg=NULL;
	char* end=NULL;
	int32_t final_len=0;
	if(q_trim_skip(dest, ptr_out-dest, beg, end)) {
		final_len=0;
		*dest='\0';
	} else {
		final_len=end-beg+1;
		memmove(dest, beg, final_len);
		*(dest+final_len)='\0';
	}

	return final_len;
}

static inline int32_t __num(int32_t* num, char* pos, int32_t len)
{
	int32_t w=len-1;
	int32_t rPos;
	// 下面检测从右到左, 单位大小是否从低到高
	while(w>0) {
		// 左边单位小于右边单位
		if(pos[w-1]<=pos[w]) {
			// 记录右边单位位置
			rPos=w;
			w--;
			// 循环检索至左边单位高于右边单位
			while(w&&(pos[w-1]<=pos[rPos]))
				w--;
			// 中间rPos->w个单位进行递归整合, 这个数值属于rPos记录
			num[w]=__num(num+w, pos+w, rPos-w)+num[rPos];
			// 单位数减少
			len=len-rPos-w;
			// 将原来rPos记录右边的单位及其数值左移
			for(int32_t i=w+1; i<len; i++)
				num[i]=num[i+rPos-w];
			for(int32_t i=w; i<len; i++)
				pos[i]=pos[i+rPos-w];
		}
		w--;
	}
	// 经过处理, 单位大小从左到右是从高到低了
	int32_t value=0;
	int32_t k=0;
	for(int32_t i=0; i<len; ++i) {
		// 计算位数
		switch(pos[i]) {
			case 10:
				k=10;
				break;
			case 11:
				k=100;
				break;
			case 12:
				k=1000;
				break;
			case 13:
				k=10000;
				break;
			case 14:
				k=100000000;
				break;
			default:
				k=1;
		}
		if(num[i]) {
			value+=k*num[i];
		} else if(pos[i]) {
			value+=k;
		}
	}
	return value;
}

static inline int32_t q_to_num(char* src, int32_t srclen)
{
	if(src==NULL||srclen<=0)
		return -1;

	char __NUMBER__[]="零一壹二贰三叁四肆五伍六陆七柒八捌九玖十拾百佰千仟万萬亿億两";
	int32_t num[32]={0};
	char pos[32]={0};

	char* p=src;
	int32_t len=0;
	char x=0;
	int32_t y=0;
	while(len<32&&(p<src+srclen)) {
		// 正常数字
		if(*p>='0'&&*p<='9') {
			x=*p-'0';
			p++;
		} else {
			// 与中文数字字符进行比较
			x=0;
			while(x<30&&(*p!=__NUMBER__[3*x]||*(p+1)!=__NUMBER__[3*x+1]||*(p+2)!=__NUMBER__[3*x+2]))
				x++;
			// 非中文数字字符
			if(x==30)
				break;
			// 计算序号
			if(x==29) x=2;
			else x=(x+1)/2;
			// utf8编码每个汉字占3个字节
			p+=3;
		}

		if(x>=10) {
			// 直接使用10-14作为序号
			pos[len++]=x;
			// 关注'二百五'的情况
			y=1;
		} else {
			// 遇到数字为0取消对'二百五'的关注, 如'二百零五'
			if(!x) y=0;
			// 允许连续的数字
			num[len]=num[len]*10+x;
		}

		// 提高兼容性, 消除空格和逗号的影响
		while(*p==' '||*p==',')
			p++;
	}

	// 首字符非数字
	if(p==src)
		return -2;

	// 无单位类型可直接返回
	if(!len)
		return num[0];

	// 不以0结尾
	if(num[len]) {
		if(y) {
			if(!pos[len]) {
				// 百以上的单位, 赋降一级的单位
				if(pos[len-1]>=11)
					pos[len]=pos[len-1]-1;
			}
		}
		// 凡不以0结尾, 计算单位要加1
		len++;
	}

	// 进入递归环节
	return __num(num, pos, len);
}

static inline unsigned int murmurHash(const void *key, int len)
{
	const unsigned int m = 0x5bd1e995;
	const int r = 24;
	const int seed = 97;
	unsigned int h = seed ^ len;
	// Mix 4 bytes at a time into the hash
	const unsigned char *data = (const unsigned char *)key;
	while(len >= 4)
	{
		unsigned int k = *(unsigned int *)data;
		k *= m; 
		k ^= k >> r; 
		k *= m; 
		h *= m; 
		h ^= k;
		data += 4;
		len -= 4;
	}
	// Handle the last few bytes of the input array
	switch(len)
	{
		case 3: h ^= data[2] << 16;
		case 2: h ^= data[1] << 8;
		case 1: h ^= data[0];
			h *= m;
	};
	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.
	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;
	return h;
}

static inline uint64_t murmurHash64A(const void * key, int len, unsigned int seed)
{
	const uint64_t m = 0xc6a4a7935bd1e995;
	const int r = 47;
	uint64_t h = seed ^ (len * m);
	const uint8_t *data = (const uint8_t *)key;
	const uint8_t *end = data + (len-(len&7));

	while(data != end) {
		uint64_t k;

		if(q_byte_order()==1) {
			k = *((uint64_t*)data);
		} else {
			k = (uint64_t) data[0];
			k |= (uint64_t) data[1] << 8;
			k |= (uint64_t) data[2] << 16;
			k |= (uint64_t) data[3] << 24;
			k |= (uint64_t) data[4] << 32;
			k |= (uint64_t) data[5] << 40;
			k |= (uint64_t) data[6] << 48;
			k |= (uint64_t) data[7] << 56;
		}

		k *= m;
		k ^= k >> r;
		k *= m;
		h ^= k;
		h *= m;
		data += 8;
	}

	switch(len & 7) {
		case 7: h ^= (uint64_t)data[6] << 48;
		case 6: h ^= (uint64_t)data[5] << 40;
		case 5: h ^= (uint64_t)data[4] << 32;
		case 4: h ^= (uint64_t)data[3] << 24;
		case 3: h ^= (uint64_t)data[2] << 16;
		case 2: h ^= (uint64_t)data[1] << 8;
		case 1: h ^= (uint64_t)data[0];
			h *= m;
	};

	h ^= h >> r;
	h *= m;
	h ^= h >> r;
	return h;
}

// \uBF03 to unicode str
static inline int32_t q_unicode_hex_decode(char* pszSrc, int32_t iSrcLen, char* pszDest, int32_t iMaxSize)
{
	if(pszSrc==NULL||pszDest==NULL||iSrcLen<=0||iMaxSize<=0)
		return -1;

	char ch(0);
	int32_t i(0);
	int32_t j(0);
	int32_t k(0);

	for(i=0, j=0; (i<iSrcLen)&&(j<iMaxSize); ++i) {
		ch=pszSrc[i];
		switch(ch) {
			case '\\':
				if((i+5<iSrcLen)&&(pszSrc[i+1]=='u')) {
					int32_t c=0;
					for(k=4; k<=5; ++k) {
						c<<=4;
						if(pszSrc[i+k]>='0'&&pszSrc[i+k]<='9') {
							c|=(pszSrc[i+k]-'0');
						} else if(pszSrc[i+k]>='a'&&pszSrc[i+k]<='f') {
							c|=(pszSrc[i+k]-'a')+10;
						} else if(pszSrc[i+k]>='A'&&pszSrc[i+k]<='F') {
							c|=(pszSrc[i+k]-'A')+10;
						}
					}
					pszDest[j++]=(char)(c&0xff);

					c=0;
					for(k=2; k<=3; ++k) {
						c<<=4;
						if(pszSrc[i+k]>='0'&&pszSrc[i+k]<='9') {
							c|=(pszSrc[i+k]-'0');
						} else if(pszSrc[i+k]>='a'&&pszSrc[i+k]<='f') {
							c|=(pszSrc[i+k]-'a')+10;
						} else if(pszSrc[i+k]>='A'&&pszSrc[i+k]<='F') {
							c|=(pszSrc[i+k]-'A')+10;
						}
					}
					pszDest[j++]=(char)(c&0xff);

					i+=5;
				} else {
					pszDest[j++]=ch;
				}
				break;
			default:
				pszDest[j++]=ch;
				break;
		}
	}
	if(j==iMaxSize)
		return -1;

	pszDest[j]='\0';
	return j;
}

static inline bool q_is_url(const char* pszSrc, int32_t iSrcLen=-1)
{
	if(iSrcLen==-1) iSrcLen=strlen(pszSrc);
	return (iSrcLen>=7&&!strncasecmp(pszSrc, "http://", 7))||(iSrcLen>=8&&!strncasecmp(pszSrc, "https://", 8));
}

static inline int32_t q_url_decode(char* pszSrc, int32_t iSrcLen, char* pszDest, int32_t iMaxSize)
{
	if(pszSrc==NULL||pszDest==NULL||iSrcLen<=0||iMaxSize<=0)
		return -1;

	char ch(0);
	int32_t i(0);
	int32_t j(0);
	int32_t k(0);

	for(i=0, j=0; (i<iSrcLen)&&(j<iMaxSize); ++i) {
		ch=pszSrc[i];
		switch(ch) {
			case '+':
				pszDest[j++]=' ';
				break;
			case '%':
				if(i+2<iSrcLen) {
					int32_t c=0;
					for(k=1; k<=2; ++k) {
						c<<=4;
						if(pszSrc[i+k]>='0'&&pszSrc[i+k]<='9') {
							c|=(pszSrc[i+k]-'0');
						} else if(pszSrc[i+k]>='a'&&pszSrc[i+k]<='f') {
							c|=(pszSrc[i+k]-'a')+10;
						} else if(pszSrc[i+k]>='A'&&pszSrc[i+k]<='F') {
							c|=(pszSrc[i+k]-'A')+10;
						}
					}
					pszDest[j++]=(char)(c&0xff);
					i+=2;
				} else {
					pszDest[j++]=ch;
				}
				break;
			default:
				pszDest[j++]=ch;
				break;
		}
	}
	if(j==iMaxSize)
		return -1;

	pszDest[j]='\0';
	return j;
}

static inline int32_t q_url_encode(char* pszSrc, int32_t iSrcLen, char* pszDest, int32_t iMaxSize)
{
	if(pszSrc==NULL||pszDest==NULL||iSrcLen<=0||iMaxSize<=0)
		return -1;

	char ch;
	int32_t i, j;
	for(i=0, j=0; (i<iSrcLen)&&(j<iMaxSize); ++i) {
		ch=pszSrc[i];
		if(((ch>='A')&&(ch<='Z'))||((ch>='a')&&(ch<='z'))||((ch>='0')&&(ch<='9'))) {
			pszDest[j++]=ch;
		} else if(ch==' ') {
			pszDest[j++]='+';
		} else if(ch=='.'||ch=='-'||ch=='_'||ch=='*') {
			pszDest[j++]=ch;
		} else {
			if(j+3<iMaxSize) {
				sprintf(pszDest+j, "%%%02X", (uint8_t)ch);
				j+=3;
			} else {
				return -2;
			}
		}
	}
	if(j==iMaxSize)
		return -1;

	pszDest[j]='\0';
	return j;
}

static inline bool q_is_palindrome(const char* s, int32_t n)
{
	if(s==NULL||n<=0)
		return false;

	const char* front=s;
	const char* back=s+n-1;
	while(front<back) {
		if(*front!=*back)
			return false;
		++front;
		--back;
	}

	return true;
}

static inline void q_full_permutation(char* perm, int32_t from, int32_t to)
{
	if(from==to) {
		for(int32_t i=0; i<=to; ++i)
			printf("%c", perm[i]);
		printf("\n");
	} else {
		for(int32_t j=from; j<=to; ++j)
		{
			q_swap(perm[j], perm[from]);
			q_full_permutation(perm, from+1, to);
			q_swap(perm[j], perm[from]);
		}
	}
}

static inline std::string q_id_to_62string(uint64_t value)
{
	uint64_t rest(value);
	const std::string ss("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
	std::string s;
	while(rest) {
		s.insert(s.begin(), ss.at(rest-(rest/62)*62));
		rest/=62;
	}
	return s;
}

static inline uint64_t q_62string_to_id(std::string s) throw(std::runtime_error)
{
	uint64_t multiple(1);
	uint64_t result(0);
	const std::string ss("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
	char ch=0;
	size_t pos=0;
	for(size_t i=0; i!=s.length(); ++i) {
		ch=s.at(s.length()-i-1);
		pos=ss.find(ch);
		if(pos==ss.npos)
			throw std::runtime_error(std::string("Unknown char: (")+ch+") error!");
		result+=pos*multiple;
		multiple*=62;
	}
	return result;
}

static inline int32_t q_stoi(const std::string& ss)
{
	int32_t temp;
	std::istringstream istr(ss);
	istr>>temp;
	if(istr.fail()) return (0);
	return temp;
}

static inline uint32_t q_stoui(const std::string& ss)
{
	uint32_t temp;
	std::istringstream istr(ss);
	istr>>temp;
	if(istr.fail()) return (0);
	return temp;
}

static inline uint64_t q_stoull(const std::string& ss)
{
	uint64_t temp;
	std::istringstream istr(ss);
	istr>>temp;
	if(istr.fail()) return (0);
	return temp;
}

static inline double q_stod(const std::string& ss)
{
	double temp;
	std::istringstream istr(ss);
	istr>>temp;
	if(istr.fail()) return (0);
	return temp;
}

static inline std::string q_to_string(int32_t n)
{
	std::ostringstream ostr;
	ostr<<n;
	return ostr.str();
}

static inline std::string q_to_string(uint32_t n)
{
	std::ostringstream ostr;
	ostr<<n;
	return ostr.str();
}

static inline std::string q_to_string(uint64_t n)
{
	std::ostringstream ostr;
	ostr<<n;
	return ostr.str();
}

static inline std::string q_to_string(double val)
{
	std::ostringstream ostr;
	ostr<<val;
	return ostr.str();
}

static inline std::string& q_left_trim(std::string& ss)
{
	return ss.erase(0, ss.find_first_not_of(" \t\r\n\v\f"));
}

static inline std::string& q_right_trim(std::string& ss)
{
	return ss.erase(ss.find_last_not_of(" \t\r\n\v\f")+1);
}

static inline std::string& q_trim(std::string& ss)
{
	return q_left_trim(q_right_trim(ss));
}

static inline std::string q_simplify(const std::string& ss)
{
	std::string spaces=" \t\r\n\f\v";
	std::string s;
	size_t start=0;
	size_t end;
	while((end=ss.find_first_of(spaces, start))!=ss.npos) {
		if(start!=end) s+=ss.substr(start, end-start)+' ';
		start=end+1;
	}
	if(start!=ss.length()) s+=ss.substr(start);
	return q_trim(s);
}

static inline std::string q_format(const char* fmt, ...)
{
	char buf[1024];			// should be enough for most cases
	int32_t n, size=sizeof(buf);
	char* p=buf;
	va_list args;

	do {
		va_start(args, fmt);
		n=vsnprintf(p, sizeof(buf), fmt, args);
		va_end(args);

		if(n>-1&&n<size)
			break;		// worked!

		if(n>-1)		// glibc 2.1+/iso c99
			size=n+1;	// exactly what's needed
		else			// glibc 2.0
			size<<=1;	// double the size and try again

		p=(char*)(p==buf?malloc(size):realloc(p, size));
		Q_ASSERT(p!=NULL, "q_format alloc error, size = (%d)", size);
	} while(true);

	if(buf==p)
		return std::string(p, n);

	std::string ret(p, n);
	free(p);

	return ret;
}

static inline std::string q_hex_dump(const uint8_t* buffer, int32_t len)
{
	std::stringstream sstr;
	sstr.exceptions(std::ifstream::failbit|std::ifstream::badbit);
	int64_t addr=0;
	int32_t cnt2=0;
	int32_t i=0;
	int32_t n=0;

	try {
		sstr<<std::endl;
		sstr<<"Dump "<<len<<" bytes."<<std::endl;

		for(n=0; n!=len; ++n) {
			if(cnt2==0) {
				sstr<<std::setw(7)<<std::setfill('0')<<int(addr)<<" - ";
				addr+=16;
			}
			cnt2=(cnt2+1)%18;
			if(cnt2<=16) {
				sstr<<std::hex<<std::setw(2)<<std::setfill('0')<<int(buffer[n])<<' ';
			} else {
				sstr<<"  ";
				sstr<<std::setfill(' ');
				for(i=n-cnt2+1; i<n; ++i) {
					if(buffer[i]<32||126<buffer[i]) {
						sstr<<'.';
					} else {
						sstr<<buffer[i];
					}
				}
				sstr<<std::endl;
				sstr<<std::dec;
				cnt2=0;
				n--;
			}
		}

		sstr<<std::setfill(' ');
		for(i=cnt2+1; i<=16; ++i)
			sstr<<std::setw(2)<<"--"<<" ";
		sstr<<"  ";

		for(i=n-cnt2; cnt2<=16&&i<n; ++i) {
			if(buffer[i]<32||126<buffer[i]) {
				sstr<<'.';
			} else {
				sstr<<buffer[i];
			}
		}
		sstr<<std::dec;
	} catch(...) {
		sstr.str("q_hex_dump failed!");
	}

	return sstr.str();
}

static inline std::string q_replace(const std::string& ss, const std::string& search, const std::string& replace)
{
	std::string s=ss;
	size_t pos=0;
	while((pos=s.find(search, pos))!=std::string::npos) {
		s.replace(pos, search.length(), replace);
		pos+=replace.length();
	}
	return s;
}

static inline bool q_starts_with(const std::string& ss, const std::string& s)
{
	return (ss.substr(0, s.length())==s)?true:false;
}

static inline bool q_starts_with(const std::string& ss, char c)
{
	if(ss.length()) {
		return (ss.at(0)==c)?true:false;
	} else {
		return false;
	}
}

static inline bool q_ends_with(const std::string& ss, const std::string& s)
{
	if(ss.length()>=s.length()) {
		return (0==ss.compare(ss.length()-s.length(), s.length(), s));
	} else {
		return false;
	}
}

static inline bool q_ends_with(const std::string& ss, char c)
{
	if(ss.length()) {
		return (ss.at(ss.length()-1)==c);
	} else {
		return false;
	}
}

static inline bool q_contains(const std::string& ss, const std::string& s)
{
	return (ss.find(s)!=s.npos)?true:false;
}

static inline bool q_contains(const std::string& ss, char c)
{
	return (ss.find(c)!=ss.npos)?true:false;
}

static inline std::string q_to_lower(const std::string& ss)
{
	std::string s=ss;
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);
	return s;
}

static inline std::string q_to_upper(const std::string& ss)
{
	std::string s=ss;
	std::transform(s.begin(), s.end(), s.begin(), ::toupper);
	return s;
}

static inline std::vector<std::string> q_split(const std::string& ss, char sep)
{
	std::vector<std::string> elems;
	size_t start=0;
	size_t end;
	while((end=ss.find(sep, start))!=ss.npos) {
		elems.push_back(ss.substr(start, end-start));
		start=end+1;
	}
	elems.push_back(ss.substr(start));
	return elems;
}

static inline void q_split(const std::string& ss, char sep, std::vector<std::string>& elems)
{
	size_t start=0;
	size_t end;
	while((end=ss.find(sep, start))!=ss.npos) {
		elems.push_back(ss.substr(start, end-start));
		start=end+1;
	}
	elems.push_back(ss.substr(start));
}

static inline void q_split(const std::string& ss, char sep, std::list<std::string>& elems)
{
	size_t start=0;
	size_t end;
	while((end=ss.find(sep, start))!=ss.npos) {
		elems.push_back(ss.substr(start, end-start));
		start=end+1;
	}
	elems.push_back(ss.substr(start));
}

static inline std::vector<std::string> q_split(const std::string& ss, const std::string& sep)
{
	std::vector<std::string> elems;
	size_t start=0;
	size_t extra=0;
	size_t end;
	while((end=ss.find(sep, start+extra))!=ss.npos) {
		elems.push_back(ss.substr(start, end-start));
		start=end+sep.length();
		extra=(sep.length()==0?1:0);
	}
	elems.push_back(ss.substr(start));
	return elems;
}

static inline void q_split(const std::string& ss, const std::string& sep, std::vector<std::string>& elems)
{
	size_t start=0;
	size_t extra=0;
	size_t end;
	while((end=ss.find(sep, start+extra))!=ss.npos) {
		elems.push_back(ss.substr(start, end-start));
		start=end+sep.length();
		extra=(sep.length()==0?1:0);
	}
	elems.push_back(ss.substr(start));
}

static inline void q_split(const std::string& ss, const std::string& sep, std::list<std::string>& elems)
{
	size_t start=0;
	size_t extra=0;
	size_t end;
	while((end=ss.find(sep, start+extra))!=ss.npos) {
		elems.push_back(ss.substr(start, end-start));
		start=end+sep.length();
		extra=(sep.length()==0?1:0);
	}
	elems.push_back(ss.substr(start));
}

static inline std::vector<std::string> q_split_any(const std::string& ss, const std::string& sep)
{
	std::vector<std::string> elems;
	size_t start=0;
	size_t extra=0;
	size_t end;
	while((end=ss.find_first_of(sep, start+extra))!=ss.npos) {
		elems.push_back(ss.substr(start, end-start));
		start=end+(sep.length()==0?0:1);
		extra=(sep.length()==0?1:0);
	}
	elems.push_back(ss.substr(start));
	return elems;
}

static inline void q_split_any(const std::string& ss, const std::string& sep, std::vector<std::string>& elems)
{
	size_t start=0;
	size_t extra=0;
	size_t end;
	while((end=ss.find_first_of(sep, start+extra))!=ss.npos) {
		elems.push_back(ss.substr(start, end-start));
		start=end+(sep.length()==0?0:1);
		extra=(sep.length()==0?1:0);
	}
	elems.push_back(ss.substr(start));
}

static inline void q_split_any(const std::string& ss, const std::string& sep, std::list<std::string>& elems)
{
	size_t start=0;
	size_t extra=0;
	size_t end;
	while((end=ss.find_first_of(sep, start+extra))!=ss.npos) {
		elems.push_back(ss.substr(start, end-start));
		start=end+(sep.length()==0?0:1);
		extra=(sep.length()==0?1:0);
	}
	elems.push_back(ss.substr(start));
}

static inline std::string q_substr(const std::string& ss, const std::string& s1, const std::string& s2)
{
	std::string s;
	size_t pos=0;
	size_t pos1, pos2;
	if(((pos1=ss.find(s1, pos))!=ss.npos)&&((pos2=ss.find(s2, pos))!=ss.npos))
		s=ss.substr(pos1+s1.length(), pos2-pos1-s1.length());
	return s;
}

static inline std::vector<std::string> q_substr_all(const std::string& ss, const std::string& s1, const std::string& s2)
{
	std::vector<std::string> elems;
	size_t pos=0;
	size_t pos1, pos2;
	while(((pos1=ss.find(s1, pos))!=ss.npos)&&((pos2=ss.find(s2, pos))!=ss.npos)) {
		elems.push_back(ss.substr(pos1+s1.length(), pos2-pos1-s1.length()));
		pos=pos2+s2.length();
	}
	return elems;
}

static inline std::list<std::string> q_to_list(const std::vector<std::string>& vec)
{
	return std::list<std::string>(vec.begin(), vec.end());
}

static inline std::vector<std::string> q_to_vector(const std::list<std::string>& lst)
{
	return std::vector<std::string>(lst.begin(), lst.end());
}

static inline std::vector<std::string> q_sent_tokenize(const std::string& ss)
{
	std::string s1=q_replace(ss, "。", ".");
	s1=q_replace(s1, "？", "?");
	s1=q_replace(s1, "！", "!");

	const std::string seps="?!";
	std::vector<std::string> sents;
	std::string sent;

	size_t start=0;
	size_t end;
	while((end=s1.find_first_of(seps, start))!=ss.npos) {
		sent=s1.substr(start, end-start+1);
		sent=q_trim(sent);
		if(!sent.empty()) sents.push_back(sent);
		start=end+1;
	}
	if(start!=s1.length()) {
		sent=s1.substr(start);
		sent=q_trim(sent);
		if(!sent.empty()) sents.push_back(sent);
	}
	return sents;
}

static inline std::vector<std::string> q_line_tokenize(const std::string& ss)
{
	std::vector<std::string> sents;
	std::string sent;
	size_t start=0;
	size_t end;
	while((end=ss.find_first_of("\r\n", start))!=ss.npos) {
		if(start!=end) {
			sent=ss.substr(start, end-start);
			sent=q_trim(sent);
			if(sent.length()) sents.push_back(sent);
		}
		start=end+1;
	}
	if(start!=ss.length()) {
		sent=ss.substr(start);
		sent=q_trim(sent);
		if(sent.length()) sents.push_back(q_trim(sent));
	}
	return sents;
}

static inline std::string q_get_host(const std::string& url)
{
	uint32_t i=0;
	int32_t count=0;
	for(; i<url.size(); ++i)
		if(url[i]=='/' && ++count==3) break;
	return url.substr(0, i);
}

static inline std::string q_repair_url(const std::string& url, const std::string& host)
{
	std::string s("");
	std::string ss("");

	s=q_replace(url, "\\/", "/");

	if(q_starts_with(s, "http://")||q_starts_with(s, "https://")||q_starts_with(s, "ftp://")) {
		ss.append(s);
	} else if(q_starts_with(s, "//")) {
		ss.append("http:");
		ss.append(s);
	} else if(q_starts_with(s, "://")) {
		ss.append("http");
		ss.append(s);
	} else if(q_starts_with(s, '/')) {
		ss.append(host);
		ss.append(s);
	} else {
		ss.append(host);
		ss.append("/");
		ss.append(s);
	}
	return ss;
}

Q_END_NAMESPACE

#endif // __QFUNC_H_
