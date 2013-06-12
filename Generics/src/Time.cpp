#include <Time.h>
#include "Time.h"

namespace Solutions { namespace Generics
{

Time::Time (const uint64 time) : m_Time()
{
	FILETIME fileTime;
	_ULARGE_INTEGER result;

	result.QuadPart = time;

	fileTime.dwLowDateTime = result.LowPart;
	fileTime.dwHighDateTime = result.HighPart;

	::FileTimeToSystemTime(&fileTime, &m_Time);
}

Time::Time (const FILETIME& time) : m_Time()
{
	::FileTimeToSystemTime(&time, &m_Time);
}

uint64 Time::Ticks() const
{
	FILETIME fileTime;
	::SystemTimeToFileTime (&m_Time, &fileTime);
	_ULARGE_INTEGER result;

	result.LowPart = fileTime.dwLowDateTime;
	result.HighPart = fileTime.dwHighDateTime;

	return (result.QuadPart);
}

uint64 Time::NTPTime() const
{
	/* FILETIME of Jan 1 1900 00:00:00. */
	static const unsigned __int64 epoch = 94354848000000000LL;

	FILETIME fileTime;
	::SystemTimeToFileTime (&m_Time, &fileTime);
	_ULARGE_INTEGER result;

	result.LowPart = fileTime.dwLowDateTime;
	result.HighPart = fileTime.dwHighDateTime;

	// FileTime starts on January 1, 1601, so correct to 1900
	uint64 seconds = ((result.QuadPart - epoch) / 10000000L) << 32;
	seconds += static_cast<uint32> (m_Time.wMilliseconds * 4294967.296); // 2^32/10^6

	return (seconds);
}

String Time::Format (const TCHAR* formatter) const
{
	TCHAR buffer[200];

	struct tm convertedTime;
	::memset(&convertedTime, 0, sizeof(convertedTime));

	convertedTime.tm_year = m_Time.wYear - 1900;
	convertedTime.tm_mon = m_Time.wMonth - 1;
	convertedTime.tm_mday = m_Time.wDay;
	convertedTime.tm_hour = m_Time.wHour;
	convertedTime.tm_min = m_Time.wMinute;
	convertedTime.tm_sec = m_Time.wSecond;

	_tcsftime (buffer, sizeof (buffer), formatter, &convertedTime);

	return (String(buffer));
}

/* static */ Time Time::Now ()
{
	SYSTEMTIME systemTime;

	::GetSystemTime(&systemTime);
	
	return (systemTime);
}

} } // namespace Solutions::Generics
