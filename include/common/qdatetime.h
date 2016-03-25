/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qdatetime.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2014/10/03
**
*********************************************************************************************/

#ifndef __QDATETIME_H_
#define __QDATETIME_H_

#include "qglobal.h"

Q_BEGIN_NAMESPACE

static const int32_t FIRST_YEAR		= 1752;		// wrong for many countries
static const int32_t SECS_PER_DAY	= 86400;
static const int32_t MSECS_PER_DAY	= 86400000;
static const uint32_t SECS_PER_HOUR	= 3600;
static const uint32_t MSECS_PER_HOUR	= 3600000;
static const uint32_t SECS_PER_MIN	= 60;
static const uint32_t MSECS_PER_MIN	= 60000;
static const int16_t monthDays[]	= {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// QDate日期类
class QDate {
	public:
		inline QDate()
		{jd=nullJd();}

		inline QDate(int32_t y, int32_t m, int32_t d)
		{jd=0; setDate(y, m, d);}

		bool isNull() const
		{return !isValid();}

		bool isValid() const
		{return jd>=minJd()&&jd<=maxJd();}

		int32_t year() const
		{
			int32_t y, m, d;
			jul2greg(jd, y, m, d);
			return y;
		}

		int32_t month() const
		{
			int32_t y, m, d;
			jul2greg(jd, y, m, d);
			return m;
		}

		int32_t day() const
		{
			int32_t y, m, d;
			jul2greg(jd, y, m, d);
			return d;
		}

		int32_t dayOfWeek() const
		{return (((jd+1)%7)+6)%7+1;}

		int32_t dayOfYear() const
		{return jd-greg2jul(year(), 1, 1)+1;}

		int32_t daysInMonth() const
		{
			int32_t y, m, d;
			jul2greg(jd, y, m, d);
			if(m==2&&isLeapYear(y))
				return 29;
			else
				return monthDays[m];
		}

		int32_t daysInYear() const
		{
			int32_t y, m, d;
			jul2greg(jd, y, m, d);
			return isLeapYear(y)?366:365;
		}

		bool setDate(int32_t y, int32_t m, int32_t d)
		{
			if(!isValid(y, m, d)) {
				Q_DEBUG("QDate::setYMD: invalid date %04d/%02d/%02d", y, m, d);
				return false;
			}
			jd=greg2jul(y, m, d);
			return true;
		}

		void getDate(int32_t& y, int32_t& m, int32_t& d)
		{jul2greg(jd, y, m, d);}

		QDate addDays(int64_t days) const
		{
			QDate d;
			d.jd=jd+days;
			return d;
		}

		int64_t daysTo(const QDate& d) const
		{return d.jd-jd;}

		std::string to_string(int32_t mode=0) const
		{
			int32_t y, m, d;
			jul2greg(jd, y, m, d);
			char date[BUFSIZ_16];
			if(mode==1) {
				q_snprintf(date, sizeof(date), "%04d/%02d/%02d", y, m, d);
			} else {
				q_snprintf(date, sizeof(date), "%04d-%02d-%02d", y, m, d);
			}
			return date;
		}

		bool operator==(const QDate &other) const
		{return jd==other.jd;}

		bool operator!=(const QDate &other) const
		{return jd!=other.jd;}

		bool operator<(const QDate &other) const
		{return jd<other.jd;}

		bool operator<=(const QDate &other) const
		{return jd<=other.jd;}

		bool operator>(const QDate &other) const
		{return jd>other.jd;}

		bool operator>=(const QDate &other) const
		{return jd>=other.jd;}

		static QDate now()
		{
			time_t ltime;
			time(&ltime);
			tm *t=localtime(&ltime);
			return QDate(t->tm_year+1900, t->tm_mon+1, t->tm_mday);
		}

		static bool isValid(int32_t y, int32_t m, int32_t d)
		{
			if(y>=0&&y<=99)
				y+=1900;
			else if(y<FIRST_YEAR||(y==FIRST_YEAR&&(m<9||(m==9&&d<14))))
				return false;
			return (d>0&&m>0&&m<=12)&&(d<=monthDays[m]||(d==29&&m==2&&isLeapYear(y)));
		}

		static bool isLeapYear(int32_t y)
		{return (y%4==0&&y%100!=0)||(y%400==0);}

	private:
		static inline int64_t nullJd()
		{return __int64_c(-784350574879);}

		static inline int64_t minJd()
		{return __int64_c(-784350574879);}

		static inline int64_t maxJd()
		{return __int64_c(784354017364);}

		uint32_t greg2jul(int32_t y, int32_t m, int32_t d) const
		{
			uint32_t c, ya;
			if ( y <= 99 )
				y += 1900;
			if ( m > 2 ) {
				m -= 3;
			} else {
				m += 9;
				y--;
			}
			c = y;	 // NOTE: Sym C++ 6.0 bug
			c /= 100;
			ya = y - 100*c;
			return 1721119 + d + (146097*c)/4 + (1461*ya)/4 + (153*m+2)/5;
		}

		void jul2greg(uint32_t jd, int32_t& y, int32_t& m, int32_t& d) const
		{
			uint32_t x;
			uint32_t j = jd - 1721119;
			y = (j*4 - 1)/146097;
			j = j*4 - 146097*y - 1;
			x = j/4;
			j = (x*4 + 3) / 1461;
			y = 100*y + j;
			x = (x*4) + 3 - 1461*j;
			x = (x + 4)/4;
			m = (5*x - 3)/153;
			x = 5*x - 3 - 153*m;
			d = (x + 5)/5;
			if ( m < 10 ) {
				m += 3;
			} else {
				m -= 9;
				y++;
			}
		}

	protected:
		friend class QDateTime;
		int64_t jd;
};

// QTime时间类
class QTime {
	public:
		inline QTime() :
			mds(NullTime)
		{}

		inline QTime(int32_t h, int32_t m, int32_t s, int32_t ms=0)
		{setHMS(h, m, s, ms);}

		bool isNull() const
		{return mds==NullTime;}

		bool isValid() const
		{return mds<MSECS_PER_DAY;}

		int32_t hour() const
		{return mds/MSECS_PER_HOUR;}

		int32_t minute() const
		{return (mds%MSECS_PER_HOUR)/MSECS_PER_MIN;}

		int32_t second() const
		{return (mds/1000)%SECS_PER_MIN;}

		int32_t msec() const
		{return mds%1000;}

		std::string to_string() const
		{
			char buf[BUFSIZ_16]={0};
			q_snprintf(buf, sizeof(buf), "%.2d:%.2d:%.2d", hour(), minute(), second());
			return buf;
		}

		bool setHMS(int32_t h, int32_t m, int32_t s, int32_t ms=0)
		{
			if (!isValid(h, m, s, ms)) {
				Q_DEBUG("QTime::setHMS, invalid time %02d:%02d:%02d.%03d", h, m, s, ms);
				mds=MSECS_PER_DAY;	 // make this invalid
				return false;
			}
			mds=(h*SECS_PER_HOUR+m*SECS_PER_MIN+s)*1000+ms;
			return true;
		}

		QTime addSecs(int32_t secs) const
		{return addMSecs(secs*1000);}

		int32_t secsTo(const QTime &t) const
		{return (t.mds-mds)/1000;}

		QTime addMSecs(int32_t ms) const
		{
			QTime t;
			if(ms<0) {
				int32_t negdays=(MSECS_PER_DAY-ms)/MSECS_PER_DAY;
				t.mds=((int32_t)mds+ms+negdays*MSECS_PER_DAY)%MSECS_PER_DAY;
			} else {
				t.mds=((int32_t)mds+ms)%MSECS_PER_DAY;
			}
			return t;
		}

		int32_t msecsTo(const QTime &t) const
		{return t.mds-mds;}

		bool operator==(const QTime &other) const
		{return mds==other.mds;}

		bool operator!=(const QTime &other) const
		{return mds!=other.mds;}

		bool operator<(const QTime &other) const
		{return mds<other.mds;}

		bool operator<=(const QTime &other) const
		{return mds<=other.mds;}

		bool operator>(const QTime &other) const
		{return mds>other.mds;}

		bool operator>=(const QTime &other) const
		{return mds>=other.mds;}

		static QTime now()
		{
#ifdef WIN32
			time_t ltime;
			time(&ltime);
			tm *t=localtime(&ltime);
			return QTime(t->tm_hour, t->tm_min, t->tm_sec, 0);
#else
			struct timeval tv;
			gettimeofday(&tv, 0);
			time_t ltime=tv.tv_sec;
			tm *t=localtime(&ltime);
			return QTime(t->tm_hour, t->tm_min, t->tm_sec, tv.tv_usec/1000);
#endif
		}

		static bool isValid(int32_t h, int32_t m, int32_t s, int32_t ms=0)
		{return (uint32_t)h<24&&(uint32_t)m<60&&(uint32_t)s<60&&(uint32_t)ms<1000;}

		int32_t elapsed() const
		{
			QTime time=now();
			int32_t n=msecsTo(time);
			if(n<0) n+=86400*1000;
			return n;
		}

	private:
		friend class QDateTime;
		enum TimeFlag {NullTime=-1};
		int32_t mds;
};

// QDateTime日期时间类
class QDateTime {
	public:
		inline QDateTime()
		{}

		explicit QDateTime(const QDate& date) :
			d(date)
		{}

		QDateTime(const QDate& date, const QTime& time) :
			d(date),
			t(time)
		{}

		QDateTime(int32_t timestamp)
		{
			time_t ltime(timestamp);
			struct tm *ptm=localtime(&ltime);
			d.setDate(ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday);
			t.setHMS(ptm->tm_hour, ptm->tm_min, ptm->tm_sec, 0);
		}

		virtual ~QDateTime()
		{}

		QDate date() const
		{return d;}

		QTime time() const
		{return t;}

		void setDate(const QDate& date)
		{d=date;}

		void setTime(const QTime& time)
		{t=time;}

		std::string to_string(int32_t mode=0) const
		{return d.to_string(mode)+' '+t.to_string();}

		bool operator==(const QDateTime &other) const
		{return d==other.d&&t==other.t;}

		inline bool operator!=(const QDateTime &other) const
		{return !(*this==other);}

		bool operator<(const QDateTime &other) const
		{if(d<other.d) return true; return d==other.d?t<other.t:false;}

		inline bool operator<=(const QDateTime &other) const
		{return !(other<*this);}

		inline bool operator>(const QDateTime &other) const 
		{return other<*this;}

		inline bool operator>=(const QDateTime &other) const
		{return !(*this<other);}

		static QDateTime now()
		{return QDateTime(QDate::now(), QTime::now());}

		static int32_t timestamp()
		{return static_cast<int32_t>(::time(NULL));}

	protected:
		QDate d;
		QTime t;
};

Q_END_NAMESPACE

#endif // __QDATETIME_H_
