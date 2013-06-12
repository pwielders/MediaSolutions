#ifndef __TIMER_H
#define __TIMER_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "Time.h"

#include <list>
#include <iostream>

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----
#pragma warning( disable : 4355 )

// ---- Class Definition ----

//
// Description: Helper class to use pointers or proxies (if lifetime management needs to be automated)
//              as a carrier to be executed by the threadpooltype.
//
namespace Solutions { namespace Generics
{
	template<typename CONTEXT>
	class TimerContextType
	{
		private:
			TimerContextType();

		public:
			inline TimerContextType (const CONTEXT& content) : m_Content(content)
			{
			}
			inline TimerContextType(const TimerContextType<CONTEXT>& copy) : m_Content (copy.m_Content)
			{
			}
			inline ~TimerContextType ()
			{
			}
			inline TimerContextType<CONTEXT>& operator= (const TimerContextType<CONTEXT>& RHS)
			{
				m_Content = RHS.m_Content;

				return (*this);
			}
			CONTEXT& Context()
			{
				return (m_Content);
			}
			inline void Timed()
			{
				m_Content->Timed();
			}
			inline bool operator== (const TimerContextType<CONTEXT>& RHS) const
			{
				return (m_Content == RHS.m_Content);
			}
			inline bool operator!= (const TimerContextType<CONTEXT>& RHS) const
			{
				return !(operator==(RHS));
			}

		private:
			CONTEXT	m_Content;
	};

	template<typename CONTENT> 
	class TimerType 
	{
		private:
			TimerType(const TimerType&);
			TimerType& operator= (const TimerType&);

		private:
			template<typename CONTENT> 
			class TimedInfo
			{
				public:
					inline TimedInfo() :
						m_ScheduleTime(0),
						m_Info()
					{
					}

					inline TimedInfo(const uint64 time, const CONTENT& contents) :
						m_ScheduleTime(time),
						m_Info(contents)
					{
					}

					inline TimedInfo(const TimedInfo& copy) :
						m_ScheduleTime(copy.m_ScheduleTime),
						m_Info(copy.m_Info)
					{
					}

					inline ~TimedInfo()
					{
					}

					inline TimedInfo& operator= (const TimedInfo& RHS)
					{
						m_ScheduleTime = RHS.m_ScheduleTime;
						m_Info = RHS.m_Info;

						return (*this);
					}

					inline uint64 ScheduleTime() const
					{
						return (m_ScheduleTime);
					}

					inline CONTENT& Content()
					{
						return (m_Info);
					}

				private:
					uint64		m_ScheduleTime;
					CONTENT		m_Info;
			};

			class TimeWorker : public Thread
			{
				private:
					TimeWorker();
					TimeWorker(const TimeWorker&);
					TimeWorker& operator= (const TimeWorker&);

				public:
					inline TimeWorker(TimerType& parent, const TCHAR* timerName) : Thread (false, timerName), m_Parent(parent)
					{
						// Everything is initialized, go...
						Run();
					}
					inline ~TimeWorker()
					{
						// This is a blocking call that waits untill the associated thread died!!!
						// Make sure you do not do Harakiri, as this will deadlock !!!!
						Terminate ();
					}

					virtual uint32 Worker ()
					{
						return (m_Parent.Process());
					}

				private:
					TimerType<CONTENT>&	m_Parent;
			};

			typedef std::list<TimedInfo<CONTENT>> SubscriberList;

		public:

			TimerType(const TCHAR* timerName) : m_PendingQueue(), m_TimerThread(*this, timerName), m_Admin(), m_NextTrigger(NUMBER_MAX_UNSIGNED(uint64))
			{
			}

			inline void Schedule (const Time& time, const CONTENT& info)
			{
				Schedule (time.Ticks(), info);
			}

			void Schedule (const uint64& time, const CONTENT& info)
			{
				m_Admin.Lock();

				SubscriberList::iterator index = m_PendingQueue.begin();

				while ((index != m_PendingQueue.end()) && (time >= (*index).ScheduleTime()))
				{
					++index;
				}

				if (index == m_PendingQueue.begin())
				{
					m_PendingQueue.push_front (TimedInfo<CONTENT>(time, info));

					// If we added the new time up front, retrigger the scheduler.
					m_TimerThread.Run ();
				}
				else if (index == m_PendingQueue.end ())
				{
					m_PendingQueue.push_back (TimedInfo<CONTENT>(time, info));
				}
				else
				{
					m_PendingQueue.insert (index, TimedInfo<CONTENT>(time, info));
				}

				m_Admin.Unlock();
			}


			void Revoke (const CONTENT& info)
			{
				m_Admin.Lock();

				SubscriberList::iterator index = m_PendingQueue.begin();

				while ((index != m_PendingQueue.end()) && (index->Content() != info))
				{
					++index;
				}

				if (index != m_PendingQueue.end())
				{
					// Found it, remove it.
					m_PendingQueue.erase (index);

					// If we added the new time up front, retrigger the scheduler.
					m_TimerThread.Run ();
				}
				else
				{
					// Since we have the admin lock, we are pretty sure that there is not any
					// context running, so we can be pretty sure that if it was scheduled, it 
					// is gone !!!
				}

				m_Admin.Unlock();
			}

			uint64 NextTrigger () const
			{
				return (m_NextTrigger);
			}

		protected:
			uint32 Process()
			{
				uint32 delayTime = INFINITE;
				uint64 now = Time::Now().Ticks();

				m_Admin.Lock();

				// Move to the delay state. We would like to have some delay afterwards..
				// Ranging from 0-INFINITE
				m_TimerThread.Delay();

				while ((m_PendingQueue.empty() == false) && (m_PendingQueue.front().ScheduleTime() <= now))
				{
					CONTENT info (m_PendingQueue.front().Content());

					// Make sure we loose the current one before we do the call, that one might add ;-)
					m_PendingQueue.pop_front();

					info.Timed();
				}

				// Calculate the delay...
				if (m_PendingQueue.empty() == true)
				{
					m_NextTrigger = NUMBER_MAX_UNSIGNED(uint64);
				}
				else
				{
					// Refresh the time, just to be on the safe side...
					uint64 delta = Time::Now().Ticks();
					
					if (delta >= m_PendingQueue.front().ScheduleTime())
					{
						m_NextTrigger = delta;
						delayTime = 0;
					}
					else
					{
						// The windows counter is in 100ns intervals dus we mmoeten even delen door  1000 (us) * 10 ns = 10.000
						// om de waarde in ms te krijgen.
						m_NextTrigger = m_PendingQueue.front().ScheduleTime();
						delayTime = static_cast<uint32> ((m_NextTrigger - delta) / TICKS_PER_MILLISECONDS);
					}
				}

				m_Admin.Unlock();

				return (delayTime);
			}

		private:
			SubscriberList		m_PendingQueue;
			TimeWorker			m_TimerThread;
			CriticalSection		m_Admin;
			uint64				m_NextTrigger;
	};

} } // namespace Solutions::Generics

#pragma warning( default : 4355 )

#endif // __TIMER_H