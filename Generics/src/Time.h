#ifndef __TIME_H
#define __TIME_H

#include "Portability.h"

#define TICKS_PER_MILLISECONDS			10000
#define TICKS_PER_MICROSECONDS			10

namespace Solutions { namespace Generics
{
	class EXTERNAL Time
	{
		public:
			Time (const uint64 time);
			Time (const FILETIME& time);
			inline Time () : m_Time()
			{
			}
			inline Time (const SYSTEMTIME& time) : m_Time(time)
			{
			}
			inline Time (const Time& copy) : m_Time(copy.m_Time)
			{
			}
			inline ~Time ()
			{
			}

			inline Time& operator= (const Time& RHS)
			{
				m_Time = RHS.m_Time;

				return (*this);
			}

		public:
			inline uint32 MilliSeconds () const
			{
				return (m_Time.wMilliseconds);
			}
			inline uint8 Seconds () const
			{
				return (static_cast<uint8> (m_Time.wSecond));
			}
			inline uint8 Minutes () const
			{
				return (static_cast<uint8> (m_Time.wMinute));
			}
			inline uint8 Hours () const
			{
				return (static_cast<uint8> (m_Time.wHour));
			}
			inline uint8 Day () const
			{
				return (static_cast<uint8> (m_Time.wDay));
			}
			inline uint8 Month () const
			{
				return (static_cast<uint8> (m_Time.wMonth));
			}
			inline uint32 Year () const
			{
				return (m_Time.wYear);
			}

			uint64 NTPTime () const;
			uint64 Ticks () const;

			static Time Now ();

			String Format (const TCHAR* formatter) const;

		private:
			SYSTEMTIME			m_Time;
	};

} } // namespace Solutions::Generics

#endif // __TIME_H
