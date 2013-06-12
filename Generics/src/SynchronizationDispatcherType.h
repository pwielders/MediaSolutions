// SocketDispatcher.h: implementation for SocketDispatcher class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __SYNCHRONIZATIONDISPATCHER_H
#define __SYNCHRONIZATIONDISPATCHER_H

#include <vector>
#include <algorithm>

#include "Thread.h"
#include "StlIterator.h"

#pragma warning(disable: 4355)

namespace Solutions { namespace Generics
{
	//
	// Description: Helper class to use pointers or proxies (if lifetime management needs to be automated)
	//              as a carrier to be executed by the threadpooltype.
	//
	template<typename CONTEXT>
	class SynchronisationContextType
	{
		private:
			SynchronisationContextType();

		public:
			inline SynchronisationContextType (const CONTEXT& content) : m_Content(content)
			{
			}
			inline SynchronisationContextType(const SynchronisationContextType<CONTEXT>& copy) : m_Content (copy.m_Content)
			{
			}
			inline ~SynchronisationContextType ()
			{
			}
			inline SynchronisationContextType<CONTEXT>& operator= (const SynchronisationContextType<CONTEXT>& RHS)
			{
				m_Content = RHS.m_Content;

				return (*this);
			}
			inline void Process()
			{
				m_Content->Process();
			}
			CONTEXT& Context()
			{
				return (m_Content);
			}
			inline SYSTEM_SYNC_HANDLE SynchronisationHandle()
			{
				return (m_Content->SynchronisationHandle());
			}
			inline bool operator== (const SynchronisationContextType<CONTEXT>& RHS) const
			{
				return (m_Content == RHS.m_Content);
			}

			inline bool operator!= (const SynchronisationContextType<CONTEXT>& RHS) const
			{
				return !(operator==(RHS));
			}
			
		private:
			CONTEXT	m_Content;
	};

	template <typename CONTEXT>
	class SynchronizationDispatcherType
	{
		private:
			typedef LockingContainer<std::vector<CONTEXT>>	ContextList;

		public:
			typedef LockingIterator<ContextList, CONTEXT>	ContextIterator;

		private:
			class DispatcherThread : Thread
			{
				private:
					DispatcherThread (const DispatcherThread&);
					DispatcherThread& operator= (const DispatcherThread&);

				public:
					DispatcherThread		(SynchronizationDispatcherType<CONTEXT>& parent, const TCHAR* threadName) :
						Thread				(false, threadName),
						m_Parent			(parent),
						m_Subscribers		(),
						m_SubscribedHandles	(NULL),
						m_HandleArraySize	(10),
						m_UpdateLock        (false, true)
					{
						m_SubscribedHandles = new SYSTEM_SYNC_HANDLE[m_HandleArraySize];
					}
					~DispatcherThread ()
					{
						m_Subscribers.Lock ();
						
						delete [] m_SubscribedHandles;

						m_SubscribedHandles = NULL;

						m_UpdateLock.SetEvent();

						m_Subscribers.Unlock ();

						Block ();

						Wait (BLOCKED|STOPPED, INFINITE);
					}

				public:
					inline public ContextIterator GetSubscribers ()
					{
						return (ContextIterator (m_Subscribers));
					}

					virtual uint32 Worker ()
					{
						m_Subscribers.Lock ();
						
						while ((IsRunning()) && (m_SubscribedHandles != NULL))
						{
							// Make sure the tread is stopped.
							m_UpdateLock.ResetEvent();

							m_Subscribers.Unlock ();

							// Wait till something happens !!!
							int index = WaitForMultipleObjects(m_Subscribers.size()+1, m_SubscribedHandles, FALSE, INFINITE);

							m_Subscribers.Lock ();

							// Make sure it is not a restart request...
							if ((index > static_cast<int>(WAIT_OBJECT_0)) && (index <= (static_cast<int>(m_Subscribers.size() + WAIT_OBJECT_0))))
							{
								// Report the initially changed item, the other (if triggered) will follow in the next loop.
								m_Subscribers[index - WAIT_OBJECT_0 - 1].Process();
							}
						}

						m_Subscribers.Unlock ();

						return (INFINITE);
					}

					void AddPort (CONTEXT& context)
					{
						m_Subscribers.Lock ();

						std::vector<CONTEXT>::iterator result = std::find(m_Subscribers.begin(), m_Subscribers.end(), context);

						if (result == m_Subscribers.end())
						{
							m_Subscribers.push_back(context);

							UpdateSynchronizationList();

							Run();
						}

						m_Subscribers.Unlock();
					}

					void RemovePort (const CONTEXT& context)
					{
						m_Subscribers.Lock ();

						std::vector<CONTEXT>::const_iterator result = std::find(m_Subscribers.begin(), m_Subscribers.end(), context);

						if (result != m_Subscribers.end())
						{
							m_Subscribers.erase(result);

							UpdateSynchronizationList();

							if (m_Subscribers.size () == 0)
							{
								Block();
							}
						}

						m_Subscribers.Unlock();
					}

					void Clear ()
					{
						m_Subscribers.Lock ();

						// Make sure the tread is stopped.
						m_UpdateLock.SetEvent();

						Block ();

						m_Subscribers.clear ();

						m_Subscribers.Unlock();
					}

				private:
					void UpdateSynchronizationList ()
					{
						// Make sure the tread is stopped.
						m_UpdateLock.SetEvent();

						if (m_Subscribers.size() >= m_HandleArraySize)
						{
							delete [] m_SubscribedHandles;
							m_HandleArraySize *= 2;
							m_SubscribedHandles =	new SYSTEM_SYNC_HANDLE[m_HandleArraySize];
						}

						int count = 1;
						std::vector<CONTEXT>::iterator index = m_Subscribers.begin();

						while (index != m_Subscribers.end())
						{
							m_SubscribedHandles[count] = (*index).SynchronisationHandle();
							count++;
							index++;
						}

						m_SubscribedHandles[0] = m_UpdateLock.operator SyncHandle().operator SYSTEM_SYNC_HANDLE();
					}

				private:
					SynchronizationDispatcherType<CONTEXT>&	m_Parent;
					#pragma warning(disable: 4251) 
					ContextList			m_Subscribers;
					#pragma warning(default: 4251) 
					SYSTEM_SYNC_HANDLE*	m_SubscribedHandles;
					unsigned int		m_HandleArraySize;
					Event				m_UpdateLock;
			};

		public:
			SynchronizationDispatcherType(const TCHAR* threadName) :
			  m_Worker (*this, threadName)
			{
			}
			~SynchronizationDispatcherType()
			{
			}

		public:
			inline void Clear ()
			{
				m_Worker.Clear();
			}
			inline void AddSynchronization (CONTEXT& element)
			{
				m_Worker.AddPort(element);
			}

			inline void RemoveSynchronization (const CONTEXT& context)
			{
				m_Worker.RemovePort(context);
			}

			inline ContextIterator GetSynchronisationIterator ()
			{
				return (m_Worker.GetSubscribers());
			}

		private:
			DispatcherThread		m_Worker;
	};

	#pragma warning(default: 4355)

} } // namespace Solutions::Generics

#endif // __SYNCHRONIZATIONDISPATCHER_H
