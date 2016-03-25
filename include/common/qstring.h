/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qstring.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/08/19
**
*********************************************************************************************/

#ifndef __QSTRING_H_
#define __QSTRING_H_

#include "qglobal.h"
#include "qtextcodec.h"

Q_BEGIN_NAMESPACE

class QChar {
	public:
		enum SpecialCharacter {
			Null = 0x0000,
			Tabulation = 0x0009,
			LineFeed = 0x000a,
			CarriageReturn = 0x000d,
			Space = 0x0020,
			Nbsp = 0x00a0,
			SoftHyphen = 0x00ad,
			ReplacementCharacter = 0xfffd,
			ObjectReplacementCharacter = 0xfffc,
			ByteOrderMark = 0xfeff,
			ByteOrderSwapped = 0xfffe,
			ParagraphSeparator = 0x2029,
			LineSeparator = 0x2028,
			LastValidCodePoint = 0x10ffff
		};

		QChar() :
			ucs(0)
		{}

		QChar(uint16_t rc) :
			ucs(rc)
		{}

		QChar(int16_t rc) :
			ucs(uint16_t(rc))
		{}

		QChar(int32_t rc) :
			ucs(uint16_t(rc&0xffff))
		{}

		QChar(uint32_t rc) :
			ucs(uint16_t(rc&0xffff))
		{}

		QChar(SpecialCharacter s) :
			ucs(uint16_t(s))
		{}

		QChar(char c) :
			ucs(uint8_t(c))
		{}

		QChar(uint8_t c) :
			ucs(c)
		{}

		inline char toLatin1() const
		{return ucs>0xff ? '\0' : char(ucs);}

		inline uint16_t unicode() const
		{return ucs;}

		inline bool isNull() const
		{return ucs==0;}

		inline bool isSpace() const
		{return (ucs==0x20||(ucs<=0x0d&&ucs>=0x09));}

		inline bool isLetter() const
		{return ((ucs>='A'&&ucs<='Z')||(ucs>='a'&&ucs<='z'));}

		inline bool isNumber() const
		{return (ucs<='9'&&ucs>='0');}

		inline bool isLetterOrNumber() const
		{return ((ucs>='A'&&ucs<='Z')||(ucs>='a'&&ucs<='z')||(ucs<='9'&&ucs>='0'));}

		inline bool isLower() const
		{return (ucs>='a'&&ucs<='z');}

		inline bool isUpper() const
		{return (ucs>='A'&&ucs<='Z');}

		bool operator==(const QChar& qch)
		{return ucs==qch.ucs;}

		bool operator!=(const QChar& qch)
		{return !(*this==qch);}

		bool operator==(QChar& qch)
		{return ucs==qch.ucs;}

		bool operator!=(QChar& qch)
		{return !(*this==qch);}

	private:
		uint16_t ucs;
};

class QString {
	public:
		inline QString(int32_t size=DEFAULT_QSTRING_SIZE) :
			iLength(0),
			iMaxSize(size)
		{
			qch=q_new_array<QChar>(iMaxSize);
			Q_ASSERT(qch!=NULL, "QString: qch is null, alloc error!");
		}

		inline QString(const char* str, int32_t len=-1)
		{
			if(len==-1) len=strlen(str);
			iMaxSize=len+1;

			qch=q_new_array<QChar>(iMaxSize);
			Q_ASSERT(qch!=NULL, "QString: qch is null, alloc error!");

#ifdef Q_CODE_GBK
			int32_t rc=QTextCodec::gbk2unicode((char*)str, len, (char*)qch, sizeof(QChar)*iMaxSize);
#else
			int32_t rc=QTextCodec::utf82unicode((char*)str, len, (char*)qch, sizeof(QChar)*iMaxSize);
#endif
			Q_ASSERT(rc>0, "QString: rc = (%d)", rc);

			rc=cancelByteOrderMark((char*)qch, rc);
			iLength=rc/2;
		}

		inline QString(const QChar* qchar, int32_t len)
		{
			Q_ASSERT(len>0, "QString: len error, len = (%d)", len);
			iLength=iMaxSize=len;
			qch=q_new_array<QChar>(iMaxSize);
			Q_ASSERT(qch!=NULL, "QString: qch is null, alloc error!");
			memcpy((char*)qch, (char*)qchar, 2*len);
		}

		inline QString(const QString& object)
		{
			iLength=iMaxSize=object.iLength;
			qch=q_new_array<QChar>(iMaxSize);
			Q_ASSERT(qch!=NULL, "QString: qch is null!, alloc error!");
			memcpy((char*)qch, (char*)object.qch, object.iLength*2);
		}

		virtual ~QString()
		{q_delete_array<QChar>(qch);}

		inline int32_t length() const
		{return iLength;}

		inline int32_t size() const
		{return iLength;}

		inline int32_t max_size() const
		{return iMaxSize;}

		inline bool empty() const
		{return iLength==0;}

		inline void clear()
		{iLength=0;}

		const QChar at(int32_t i) const
		{
			Q_ASSERT(i>=0&&i<iLength, "QString: out of range error, i = (%d)", i);
			return qch[i];
		}

		const QChar operator[](int32_t i) const
		{
			Q_ASSERT(i>=0&&i<iLength, "QString: out of range error, i = (%d)", i);
			return qch[i];
		}

		char* c_str()
		{return (char*)qch;}

		const char* c_str() const
		{return (char*)qch;}

		QString& append(const char* str, int32_t len=-1)
		{return append(QString(str, len));}

		QString& append(const QString& object)
		{
			if(iLength+object.iLength<=iMaxSize) {
				memcpy((char*)(qch+iLength), (char*)object.qch, object.iLength*2);
				iLength+=object.iLength;
			} else {
				iMaxSize=iLength+object.iLength;
				QChar* temp=qch;
				qch=q_new_array<QChar>(iMaxSize);
				Q_ASSERT(qch!=NULL, "QString: qch is null, alloc error!");
				memcpy((char*)qch, (char*)temp, iLength*2);
				memcpy((char*)(qch+iLength), (char*)object.qch, object.iLength*2);
				iLength+=object.iLength;
				q_delete_array<QChar>(temp);
			}
			return *this;
		}

		QString left(int32_t len)
		{return mid(0, len);}

		QString right(int32_t len)
		{return mid(iLength-len, len);}

		QString mid(int32_t pos, int32_t len=-1)
		{
			if(len<0) len=iLength-pos;
			return QString(qch+pos, len);
		}

		int32_t indexOf(const char* str, int32_t len=-1, int32_t pos=0)
		{return indexOf(QString(str, len), pos);}

		int32_t indexOf(const QString& pat, int32_t pos=0)
		{
			if(pat.empty())
				return -1;
			int32_t i, j;
			for(i=pos; i<=iLength-pat.length(); ++i) {
				for(j=0; j<pat.length(); ++j)
					if(qch[i+j]!=pat[j]) break;
				if(j==pat.length())
					return i;
			}
			return -1;
		}

		int32_t indexFirstOf(const char* str, int32_t len=-1, int32_t pos=0)
		{return indexFirstOf(QString(str, len), pos);}

		int32_t indexFirstOf(const QString& pat, int32_t pos=0)
		{
			if(pat.empty())
				return -1;
			for(int32_t i=pos; i!=iLength; ++i) {
				for(int32_t j=0; j!=pat.length(); ++j)
					if(qch[i]==pat[j])
						return i;
			}
			return -1;
		}

		std::list<QString> line_tokenize()
		{
			std::list<QString> sents;
			QString sent;
			int32_t start=0;
			int32_t end;
			while((end=indexFirstOf("\r\n", 2, start))!=-1) {
				if(start!=end) {
					sent=mid(start, end-start);
					sent=sent.trimmed();
					if(sent.length()) sents.push_back(sent);
				}
				start=end+1;
			}
			if(start!=iLength) {
				sent=mid(start);
				sent=sent.trimmed();
				if(sent.length()) sents.push_back(sent);
			}
			return sents;
		}

		QString& readAll(const char* pFileName)
		{
			std::ifstream infile(pFileName, std::ios::in|std::ios::binary);
			Q_ASSERT(infile.good(), "QString: unable to open file (%s) error", pFileName);
			std::string s;
			infile.seekg(0, std::ios::end);
			s.resize(infile.tellg(), '\t');
			infile.seekg(0, std::ios::beg);
			infile.read(&s[0], s.size());
			infile.close();
			infile.clear();
			s.erase(0, s.find_first_not_of(" \t\r\n\v\f"));
			s.erase(s.find_last_not_of(" \t\r\n\v\f")+1);
			*this=s.c_str();
			return *this;
		}

		bool startsWith(const char* str, int32_t len=-1)
		{return startsWith(QString(str, len));}

		bool startsWith(const QString& pat)
		{return (left(pat.length())==pat)?true:false;}

		bool endsWith(const char* str, int32_t len=-1)
		{return endsWith(QString(str, len));}

		bool endsWith(const QString& pat)
		{return (right(pat.length())==pat)?true:false;}

		QString& trimmed()
		{
			QChar* beg_pos=qch;
			QChar* end_pos=qch+iLength-1;

			while(beg_pos<=end_pos&&(*beg_pos==0x0009||*beg_pos==0x000d||*beg_pos==0x000a||*beg_pos==0x0020))
				beg_pos++;
			if(beg_pos>end_pos) {
				iLength=0;
				return *this;
			}

			while(beg_pos<=end_pos&&(*end_pos==0x0009||*end_pos==0x000d||*end_pos==0x000a||*end_pos==0x0020))
				end_pos--;
			if(beg_pos>end_pos) {
				iLength=0;
				return *this;
			}

			int32_t offset=beg_pos-qch;
			iLength=end_pos-beg_pos+1;
			if(offset) {
				for(int32_t i=0; i!=iLength; ++i)
					qch[i]=qch[i+offset];
			}
			return *this;
		}

		QString& operator=(const char* str)
		{
			int32_t len=strlen(str);
			if(iMaxSize<len+1) {
				delete [] qch;
				iMaxSize=len;
				qch=q_new_array<QChar>(iMaxSize);
				Q_ASSERT(qch!=NULL, "QString: qch is null!, alloc error");
			}

#ifdef Q_CODE_GBK
			int32_t rc=QTextCodec::gbk2unicode((char*)str, len, (char*)qch, sizeof(QChar)*iMaxSize);
#else
			int32_t rc=QTextCodec::utf82unicode((char*)str, len, (char*)qch, sizeof(QChar)*iMaxSize);
#endif
			Q_ASSERT(rc>0, "QString: rc = (%d)", rc);

			rc=cancelByteOrderMark((char*)qch, rc);
			iLength=rc/2;
			return *this;
		}

		QString& operator=(const QString& object)
		{
			if(&object!=this) {
				if(iMaxSize>=object.iLength) {
					iLength=object.iLength;
					memcpy((char*)qch, (char*)object.qch, object.iLength*2);
				} else {
					q_delete_array<QChar>(qch);
					qch=q_new_array<QChar>(object.iMaxSize);
					Q_ASSERT(qch!=NULL, "QString: alloc error, qch is null!");
					iMaxSize=object.iMaxSize;
					iLength=object.iLength;
					memcpy((char*)qch, (char*)object.qch, object.iLength*2);
				}
			}
			return *this;
		}

		int32_t operator==(const char* str)
		{return (*this==QString(str));}

		int32_t operator==(const QString& object)
		{return ((iLength==object.iLength)&&(memcmp((char*)qch, (char*)object.qch, iLength)==0));}

		int32_t operator!=(const char* str)
		{return !(*this==QString(str));}

		int32_t operator!=(const QString& object)
		{return !((iLength==object.iLength)&&(memcmp((char*)qch, (char*)object.qch, iLength)==0));}

		friend std::istream& operator>>(std::istream& in, QString& object)
		{
			char temp[BUFSIZ_1K]={0};
			in>>temp;
			object=temp;
			return in;
		}

		friend std::ostream& operator<<(std::ostream& out, const QString& object)
		{
			if(object.iLength==0) {
				out<<"null!";
				return out;
			} else {
				char* pszBuffer=q_new_array<char>(6*object.iLength);
				memset(pszBuffer, 0, 6*object.iLength);
#ifdef Q_CODE_GBK
				int32_t rc=QTextCodec::unicode2gbk((char*)(object.qch), sizeof(QChar)*(object.iLength), pszBuffer, 6*(object.iLength));
#else
				int32_t rc=QTextCodec::unicode2utf8((char*)(object.qch), sizeof(QChar)*(object.iLength), pszBuffer, 6*(object.iLength));
#endif
				Q_ASSERT(rc>0, "QString: rc = (%d)", rc);

				out<<pszBuffer;
				q_delete_array<char>(pszBuffer);
				return out;
			}
		}

	private:
		int32_t cancelByteOrderMark(char* pszBuffer, int32_t iLength)
		{
			if(pszBuffer==NULL||iLength<=0)
				return 0;
			if((iLength>=2)&&(*(uint16_t *)pszBuffer==0xfeff)) {
				for(int32_t i=0; i!=iLength-2; ++i)
					pszBuffer[i]=pszBuffer[i+2];
				iLength-=2;
			}
			return iLength;
		}

	protected:
		enum {DEFAULT_QSTRING_SIZE=128};
		QChar* qch;
		int32_t iLength;
		int32_t iMaxSize;
};

Q_END_NAMESPACE

#endif // __QSTRING_H_
