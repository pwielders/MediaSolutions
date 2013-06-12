// ===========================================================================
//
// Filename:    multiplelock.h
//
// Description: Header file for for the CCriticalSection, CBinarySemahore,
//              CCountingSemaphore and the CEvent synchronisation classes.
//
// History
//
// Author        Reason                                             Date
// ---------------------------------------------------------------------------
// P. Wielders   Initial creation                                   2002/05/24
//
// ===========================================================================

#ifndef __MULTIPLELOCK_H
#define __MULTIPLELOCK_H

#include "Module.h"

#ifdef __WIN32__
#include <process.h>
#endif 

// ===========================================================================
// class MultipleLock
// ===========================================================================
namespace Solutions { namespace Generics
{

	template <int count>
	class MultipleLock
	{
	  private:
		MultipleLock (const MultipleLock& a_Copy);
		MultipleLock& operator= (const MultipleLock& a_RHS);

	  public:    // Methods
		MultipleLock ();
		~MultipleLock ();

		bool                         Lock                 (unsigned int a_Mask);
		void                         Unlock               (unsigned int a_Mask);
		void                         Cancel               ();

	  private:
		bool                         Aquire               (unsigned int a_Mask);
		void                         Release              (unsigned int a_Mask);
	    
	  private: // Members
		typedef struct
		{
		   unsigned long		     m_TaskId;
		   unsigned int              m_Count;

		}  struct_lock;
		struct_lock                  m_Locks[count];    
		unsigned short               m_InterestCount;
		unsigned short               m_InterestCaptured;
		Event                        m_Changed;
		CriticalSection              m_AdminLock;
		bool                         m_Cancel;
	};

	template <int count>
	MultipleLock<count>::MultipleLock () :
	  m_InterestCount                   (0),
	  m_InterestCaptured                (0),
	  m_Changed                         (true, false),
	  m_AdminLock                       (),
	  m_Cancel                          (false)
	{
	  // Clear the lock table, All locks available.
	  for (unsigned int l_Teller = 0; l_Teller < count; l_Teller++)
	  {
		m_Locks[l_Teller].m_TaskId = 0L;
		m_Locks[l_Teller].m_Count  = 0;
	  }
	}

	template <int count>
	MultipleLock<count>::~MultipleLock ()
	{
	}

	template <int count>
	bool
	MultipleLock<count>::Lock (unsigned int a_Mask)
	{
	  bool l_Result;
	  bool l_Canceled = false;

	  // This has to be attomic.
	  m_AdminLock.Lock ();

	  // Can we get the locks right away ?
	  while ( (l_Canceled == false) && ((l_Result = Aquire (a_Mask)) == false) )
	  {
		// We are interested in changes.
		m_InterestCount++;

		m_AdminLock.Unlock ();

		// Wait for a change.
		m_Changed.Lock ();

		// Maybe they are trying to cancel this call
		l_Canceled = m_Cancel;

		// OK we have woken up to continue.
		m_InterestCaptured++;

		// Make sure we are attomic again.
		m_AdminLock.Lock ();
	  }

	  // O.K. we are done, continue....
	  m_AdminLock.Unlock ();

	  return (l_Result);
	}

	template <int count>
	void
	MultipleLock<count>::Unlock (unsigned int a_Mask)
	{
	  // This has to be atomic.
	  m_AdminLock.Lock ();

	  // Release all locks.
	  Release (a_Mask);

	  // Signal waiting processes that something changed.
	  if (m_InterestCount > 0)
	  {
		m_InterestCaptured = 0;

		m_Changed.SetEvent ();

		// Wait for all subscribed (m_InterestCount)
		// process to take theit token. So long, we
		// Hold the admin token, so they can continue
		// till the admin token.
		while (m_InterestCaptured != m_InterestCount)
		{
		  // Well let's relinguish the CPU.
		  ::SleepMs (0);
		}

		// Everyone who subscribed is through, Clear
		// the flag again.
		m_Changed.ResetEvent ();
	  }

	  // Done with the atomic action. 
	  m_AdminLock.Unlock ();
	}

	template <int count>
	void
	MultipleLock<count>::Cancel ()
	{
	  // This has to be atomic.
	  m_AdminLock.Lock ();

	  // Signal waiting processes that something changed.
	  if (m_InterestCount > 0)
	  {
		m_InterestCaptured = 0;
		m_Cancel           = true;

		m_Changed.SetEvent ();

		// Wait for all subscribed (m_InterestCount)
		// process to take theit token. So long, we
		// Hold the admin token, so they can continue
		// till the admin token.
		while (m_InterestCaptured != m_InterestCount)
		{
		  // Well let's relinguish the CPU.
		  ::SleepMs (0);
		}

		// Everyone who subscribed is through, Clear
		// the flag again.
		m_Changed.ResetEvent ();

		m_Cancel = false;
	  }

	  // Done with the atomic action.
	  m_AdminLock.Unlock ();
	}

	template <int count>
	bool
	MultipleLock<count>::Aquire (unsigned int a_Mask)
	{
	  bool           l_Result = true;
	  unsigned int   l_Mask   = a_Mask;
	  unsigned int   l_Index  = 0;
	#ifdef __WIN32__
	  unsigned long  l_ID     = ::_getpid ();
	#endif
	#ifdef __POSIX__
	  unsigned long  l_ID     = reinterpret_cast <unsigned long> (pthread_self ());
	#endif

	  // Check if we can Lock all locks......
	  while ( (l_Mask != 0) && (l_Result != false) )
	  {
		l_Result = ( (l_Mask & 0x01) == 0) || (m_Locks[l_Index].m_Count == 0) || (m_Locks[l_Index].m_TaskId == l_ID);

		// Move on to the next mask bit.
		l_Index++;
		l_Mask = l_Mask >> 1;
	  }

	  // And can we lock all locks ?
	  if (l_Result != false)
	  {
		l_Mask  = a_Mask;
		l_Index = 0;

		// Yes we can so, lock them.
		while (l_Mask != 0)
		{
		  if ( (l_Mask & 0x01) != 0 )
		  {
			m_Locks[l_Index].m_Count++;
			m_Locks[l_Index].m_TaskId = l_ID;
		  }

		  // Move on to the next mask bit.
		  l_Index++;
		  l_Mask = l_Mask >> 1;

		}
	  }

	  return (l_Result);
	}

	template <int count>
	void
	MultipleLock<count>::Release (unsigned int a_Mask)
	{
	  unsigned int   l_Index  = 0;

	  // Free all specified locks....
	  while (a_Mask != 0)
	  {
		if ( (a_Mask & 0x01) != 0 )
		{
		  // Is it an unbalanced lock ?
		  if (m_Locks[l_Index].m_Count == 0)
		  {
			TRACE_L1 ("Semaphore more unlocked then Locked", 0);
		  }
		  else
		  {
			// Do we own this semaphore ?
			ASSERT (m_Locks[l_Index].m_TaskId == CThreadControlBlock::Instance ().ID ());

			// Release this semaphore at least once.
			m_Locks[l_Index].m_Count--;
		  }
		}

		// Move on to the next mask bit.
		l_Index++;
		a_Mask = a_Mask >> 1;
	  }
	}

} } // namespace Solutions::Generics

#endif // __MULTIPLELOCK_H
