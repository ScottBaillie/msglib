//////////////////////////////////////////////////////////////////////////////

#include <Time/Time.h>

using namespace msglib;

#include <iostream>

//////////////////////////////////////////////////////////////////////////////

Time::Time(const bool now, bool realtime, bool course)
{
	if (now) this->now(realtime,course);
}

//////////////////////////////////////////////////////////////////////////////

void
Time::now(bool realtime, bool course)
{
	clockid_t clockid;

	if (realtime) {
		if (course) clockid = CLOCK_REALTIME_COARSE; else clockid = CLOCK_REALTIME;
	} else {
		if (course) clockid = CLOCK_MONOTONIC_COARSE; else clockid = CLOCK_MONOTONIC;
	}

	int res = ::clock_gettime(clockid, &m_time);
	if (res != 0 ) std::cout << "Time::now : Error from clock_gettime\n";
}

//////////////////////////////////////////////////////////////////////////////

time_t
Time::get() const
{
	return m_time.tv_sec;
}

//////////////////////////////////////////////////////////////////////////////

double
Time::getDouble() const
{
	double ret = m_time.tv_sec + (m_time.tv_nsec/1000000000.0);
	return ret;
}

//////////////////////////////////////////////////////////////////////////////

const struct timespec &
Time::getTimespec() const
{
	return m_time;
}

//////////////////////////////////////////////////////////////////////////////

struct timeval
Time::getTimeval() const
{
	struct timeval ret;
	ret.tv_sec = m_time.tv_sec;
	ret.tv_usec = m_time.tv_nsec/1000;
	return ret;
}

//////////////////////////////////////////////////////////////////////////////

int64_t
Time::getNanoseconds() const
{
	return m_time.tv_nsec;
}

//////////////////////////////////////////////////////////////////////////////

std::string
Time::getString(const bool utc) const
{
	std::string ret;
	char buf[32];

	if (utc) {
		struct tm tm1;
		struct tm * ptm = ::gmtime_r(&m_time.tv_sec, &tm1);
		if (ptm==0) return("");
		char * p = ::asctime_r(&tm1, buf);
		if (p) ret = buf;
	} else {
		char * p = ::ctime_r(&m_time.tv_sec, buf);
		if (p) ret = buf;
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////////

void
Time::set(time_t time)
{
	m_time.tv_sec = time;
	m_time.tv_nsec = 0;
}

void
Time::set(const struct timespec & ts)
{
	m_time = ts;
}

void
Time::set(const struct timeval & tv)
{
	m_time.tv_sec = tv.tv_sec;
	m_time.tv_nsec = 1000*tv.tv_usec;
}

//////////////////////////////////////////////////////////////////////////////

bool
Time::set(const std::string & dateString, const bool utc)
{
	struct tm tm1;

//	char * format = "%Y-%m-%d %H:%M:%S";
	const char * format = "%a %b %d %H:%M:%S %Y";

	char * p = ::strptime(dateString.c_str(), format, &tm1);

	if (!p) return false;

	time_t t;
	if (utc) {
		t = ::timegm(&tm1);
	} else {
		t = ::timelocal(&tm1);
	}

	m_time.tv_sec = t;
	m_time.tv_nsec = 0;

	return true;
}

//////////////////////////////////////////////////////////////////////////////
