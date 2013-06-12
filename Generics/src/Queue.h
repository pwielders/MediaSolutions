// ===========================================================================
// Author        Reason                                             Date
// ---------------------------------------------------------------------------
// P. Wielders   Initial creation                                   2010/01/14
//
// ===========================================================================

#ifndef __QUEUE_H
#define __QUEUE_H

#include "Module.h"
#include "Sync.h"
#include "StateTrigger.h"

#include <queue>

namespace Solutions { namespace Generics
{
	template <typename CONTEXT>
	class QueueType  
	{
		private:
			// -------------------------------------------------------------------
			// This object should not be copied or assigned. Prevent the copy 
			// constructor and assignment constructor from being used. Compiler 
			// generated assignment and copy methods will be blocked by the 
			// following statments.
			// Define them but do not implement them, compile error/link error.
			// -------------------------------------------------------------------
			QueueType ();
			QueueType (const QueueType<CONTEXT>&);
			QueueType& operator= (const QueueType<CONTEXT>&);

		public:
			explicit QueueType	(
				const uint32		 a_HighWaterMark) :
					m_Queue			(),
					m_State			(EMPTY),
					m_MaxSlots		(a_HighWaterMark)
			{
				// A highwatermark of 0 is bullshit.
				ASSERT (m_MaxSlots != 0);

				TRACE_L5 ("Constructor QueueType <0x%X>", TRACE_POINTER (this));
			}

			~QueueType	()
			{
				TRACE_L5 ("Destructor QueueType <0x%X>", TRACE_POINTER (this));

				// Disable the queue and flush all entries.
				Disable ();
			}

			typedef enum
			{
				EMPTY	 = 0x0001,
				ENTRIES  = 0x0002,
				LIMITED	 = 0x0004,
				DISABLED = 0x0008

			}	enumQueueState;

			// -------------------------------------------------------------------
			// This queue enforces a Producer-Consumer pattern. It takes a 
			// pointer on the heap. This pointer is created by the caller 
			// (producer) of the Post method. It should be destructed by the 
			// receiver (consumer). The consumer is the one that calls the 
			// Receive method.
			// -------------------------------------------------------------------
			bool Remove (const CONTEXT&	a_Entry)
			{
				bool l_Removed = false;

				// This needs to be atomic. Make sure it is.
				m_Admin.Lock ();

				if (m_State != DISABLED)
				{
					std::list<CONTEXT>::const_iterator index = std::find(m_Queue.begin(), m_Queue.end(), a_Entry);

					if (index != m_Queue.end())
					{
						// Yep, we found it, remove it 
						l_Removed = true;
						m_Queue.erase (index);
					}

					// Determine the new state.
					m_State.SetState ( IsEmpty () ? EMPTY : ENTRIES );
				}

				// Done with the administration. Release the lock.
				m_Admin.Unlock ();

				return (l_Removed);
			}

			bool Post (CONTEXT&	a_Entry)
			{
				bool Result = false;

				// This needs to be atomic. Make sure it is.
				m_Admin.Lock ();

				if (m_State != DISABLED)
				{
					// Yep, let's fill it
					//lint -e{534}
					m_Queue.push_back (a_Entry);

					// Determine the new state.
					m_State.SetState ( IsFull () ? LIMITED : ENTRIES );

					Result = true;
				}

				// Done with the administration. Release the lock.
				m_Admin.Unlock ();

				return (Result);
			}

			bool Insert (CONTEXT& a_Entry, DWORD a_WaitTime)
			{
				bool	l_Posted	= false;
				bool	l_Triggered	= true;

				// This needs to be atomic. Make sure it is.
				m_Admin.Lock ();

				if (m_State != DISABLED)
				{
					do
					{
						// And is there a slot available to us ?
						if (m_State != LIMITED)
						{
							// We have posted it.
							l_Posted = true;

							// Yep, let's fill it
							m_Queue.push_back (a_Entry);

							// Determine the new state.
							m_State.SetState ( IsFull () ? LIMITED : ENTRIES );
						}
						else
						{
							// We are moving into a wait, release the lock.
							m_Admin.Unlock ();

							// Wait till the status of the queue changes.
							l_Triggered = m_State.WaitState (DISABLED|ENTRIES|EMPTY, a_WaitTime);

							// Seems something happend, lock the administration.
							m_Admin.Lock ();

							// If we were reset, that is assumed to be also a timeout
							l_Triggered = l_Triggered && (m_State != DISABLED);
						}

					} while ( (l_Posted == false) && (l_Triggered != false) );
				}

				// Done with the administration. Release the lock.
				m_Admin.Unlock ();

				return (l_Posted);
			}

			bool Extract (CONTEXT& a_Result, DWORD a_WaitTime)
			{
				bool	l_Received  = false;
				bool	l_Triggered = true;

				// This needs to be atomic. Make sure it is.
				m_Admin.Lock ();

				if (m_State != DISABLED)
				{
					do
					{
						// And is there a slot to read ?
						if (m_State != EMPTY)
						{
							l_Received = true;

							std::list<CONTEXT>::iterator index = m_Queue.begin();

							// Get the first entry from the first spot..
							a_Result =*index; m_Queue.erase(index);

							// Determine the new state.
							m_State.SetState ( IsEmpty () ? EMPTY : ENTRIES );
						}
						else
						{
							// We are moving into a wait, release the lock.
							m_Admin.Unlock ();

							// Wait till the status of the queue changes.
							l_Triggered = m_State.WaitState (DISABLED|ENTRIES|LIMITED, a_WaitTime);

							// Seems something happend, lock the administration.
							m_Admin.Lock ();

							// If we were reset, that is assumed to be also a timeout
							l_Triggered = l_Triggered && (m_State != DISABLED);
						}

					} while ( (l_Received == false) && (l_Triggered != false));
				}

				// Done with the administration. Release the lock.
				m_Admin.Unlock ();

				return (l_Received);
			}

			void Enable ()
			{
				// This needs to be atomic. Make sure it is.
				m_Admin.Lock ();

				if ( m_State == DISABLED )
				{
					m_State.SetState (EMPTY);
				}

				// Done with the administration. Release the lock.
				m_Admin.Unlock ();
			}

			void Disable ()
			{
				// This needs to be atomic. Make sure it is.
				m_Admin.Lock ();

				if (m_State != DISABLED)
				{
					// Change the state
					m_State.SetState (DISABLED);
				}

				// Done with the administration. Release the lock.
				m_Admin.Unlock ();
			}

			void Flush ()
			{
				// Clear is only possible in a "DISABLED" state !!
				ASSERT(m_State == DISABLED);

				// This needs to be atomic. Make sure it is.
				m_Admin.Lock ();

				// Clear all entries !!
				while (m_Queue.empty() == false)
				{
					m_Queue.erase(m_Queue.begin());
				}

				// Done with the administration. Release the lock.
				m_Admin.Unlock ();
			}

			inline void FreeSlot () const
			{
				m_State.WaitState (false, DISABLED|ENTRIES|EMPTY, INFINITE);
			}

			inline bool IsEmpty () const
			{
				return (m_Queue.empty());
			}

			inline bool IsFull () const
			{
				return (m_Queue.size() >=  m_MaxSlots);
			}

		private:
			std::list<CONTEXT>			 m_Queue;
			StateTrigger<enumQueueState> m_State;
			CriticalSection				 m_Admin;
			uint32			             m_MaxSlots;
	};

} } // namespace Solutions::Generics

#endif // __QUEUE_H
