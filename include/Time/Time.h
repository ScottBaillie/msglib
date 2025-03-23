//////////////////////////////////////////////////////////////////////////////

#ifndef MSGLIB_TIME_H
#define MSGLIB_TIME_H

//////////////////////////////////////////////////////////////////////////////

#include <string>

#include <time.h>

//////////////////////////////////////////////////////////////////////////////

namespace msglib {

//////////////////////////////////////////////////////////////////////////////

class Time
{
public:
	Time(const bool now = false, bool realtime = true, bool course = false);

	void now(bool realtime = true, bool course = false);

	time_t get() const;
	double getDouble() const;
	const struct timespec & getTimespec() const;
	struct timeval getTimeval() const;
	int64_t getNanoseconds() const;

	std::string getString(const bool utc = false) const;

	void set(time_t time);
	void set(const struct timespec & ts);
	void set(const struct timeval & tv);
	bool set(const std::string & dateString, const bool utc = false);

	operator bool () const
	{
		if ((m_time.tv_sec==0) && (m_time.tv_nsec==0)) return false;
		return true;
	}

	bool operator == (const Time & time) const
	{
		if (m_time.tv_sec != time.m_time.tv_sec) return false;
		if (m_time.tv_nsec != time.m_time.tv_nsec) return false;
		return true;
	}

	bool operator < (const Time & time) const
	{
		if (m_time.tv_sec < time.m_time.tv_sec) return true;
		if (m_time.tv_nsec < time.m_time.tv_nsec) return true;
		return false;
	}

	bool operator <= (const Time & time) const { return((operator<(time)) || (operator==(time)));}

	bool operator != (const Time & time) const { return ! operator==(time);}

	bool operator >= (const Time & time) const { return ! operator<(time);}

	bool operator > (const Time & time) const { return ! operator<=(time);}

private:
	struct timespec		m_time;
};

//////////////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////////////

template<>
struct std::hash<msglib::Time>
{
	std::size_t operator()(const msglib::Time & t) const noexcept
	{
		std::size_t h1 = std::hash<size_t>{}(t.get());
		std::size_t h2 = std::hash<size_t>{}(t.getNanoseconds());
		return h1 ^ h2;
	}
};

//////////////////////////////////////////////////////////////////////////////

#endif
