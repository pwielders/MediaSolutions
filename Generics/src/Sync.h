// ===========================================================================
//
// Filename:    sync.h
//
// Description: Header file for for the CCriticalSection, CBinarySemahore,
//              CCountingSemaphore and the Event synchronisation classes.
//
// History
//
// Author        Reason                                             Date
// ---------------------------------------------------------------------------
// P. Wielders   Initial creation                                   2002/05/24
//
// ===========================================================================

#ifndef __SYNC_H
#define __SYNC_H

#include "Module.h"

#include <list>

#ifdef __LINUX__
#include <pthread.h>
#endif

namespace Solutions { namespace Generics
{
	class EXTERNAL SyncHandle
	{
	  public:
		SyncHandle     () :
		  m_SyncHandle (reinterpret_cast<SYSTEM_SYNC_HANDLE>(~0))
		{
		}
		SyncHandle     (SYSTEM_SYNC_HANDLE a_Handle) :
		  m_SyncHandle (a_Handle)
		{
		}
		SyncHandle     (const SyncHandle& a_Copy) :
		  m_SyncHandle (a_Copy.m_SyncHandle)
		{
		}
		~SyncHandle () 
		{
		}

		inline SyncHandle& operator= (const SyncHandle& a_RHS)
		{
		  // Copy my own members
		  return (operator=(a_RHS.m_SyncHandle));
		}

		SyncHandle&    operator=     (SYSTEM_SYNC_HANDLE a_SyncHandle)
		{
		  //  This does not change the base, so...
		  m_SyncHandle = a_SyncHandle;

		  return (*this);
		}

	  public:
		operator SYSTEM_SYNC_HANDLE () const
		{ 
		  return (m_SyncHandle);   
		}

	  private:
		SYSTEM_SYNC_HANDLE             m_SyncHandle;
	};

	// ===========================================================================
	// class CCriticalSection
	// ===========================================================================

	class EXTERNAL CriticalSection
	{
	  public:    // Methods
		CriticalSection ();
		~CriticalSection ();

		inline void Lock ()
		{
			#ifdef __POSIX__
			if (pthread_mutex_lock (&m_syncMutex) != 0)
			{
			  TRACE_L1 ("Probably creating a deadlock situation.", 0);
			}
			#endif

			#ifdef __WIN32__
			::EnterCriticalSection (&m_syncMutex);
			#endif
		}

		inline void Unlock ()
		{
			#ifdef __POSIX__
			if (pthread_mutex_unlock (&m_syncMutex) != 0)
			{
			  TRACE_L1 ("Probably does the calling thread not own this CCriticalSection.", 0);
			}
			#endif

			#ifdef __WIN32__
			::LeaveCriticalSection (&m_syncMutex);
			#endif
		}

	  protected: // Members
	#ifdef __POSIX__
		pthread_mutex_t              m_syncMutex;
	#endif
	#ifdef __WIN32__
		CRITICAL_SECTION 			 m_syncMutex;
	#endif
	};

	// ===========================================================================
	// class CBinairySemaphore
	// ===========================================================================

	class EXTERNAL BinairySemaphore
	{
	  public:    // Methods
		BinairySemaphore (unsigned int nInitialCount, unsigned int nMaxCount);
		BinairySemaphore (bool blLocked = true);
		~BinairySemaphore ();

		int                          Lock                 ();
		int                          Lock                 (unsigned int nSeconds);
		void                         Unlock               ();
		bool                         Locked               () const;

	#ifdef __WIN32__
		inline operator              SyncHandle           () { return (SyncHandle(m_syncMutex)); }
	#endif

	  protected: // Members
	#ifdef __POSIX__
		pthread_mutex_t              m_syncAdminLock;
		pthread_cond_t               m_syncCondition;
		volatile bool                m_blLocked;
	#endif

	#ifdef __WIN32__
		HANDLE						 m_syncMutex;
	#endif
	};

	// ===========================================================================
	// class CCountingSemaphore
	// ===========================================================================

	// For Windows platform compatibility
	#define Semaphore              CountingSemaphore

	class EXTERNAL CountingSemaphore
	{
	  public:    // Methods
		CountingSemaphore (unsigned int nInitialCount = 1, unsigned int nMaxCount = 1);
		~CountingSemaphore ();

		int                          Lock                 ();
		int                          Lock                 (unsigned int nSeconds);
		int                          Unlock               (unsigned int nCount = 1);
		int                          TryUnlock            (unsigned int nSeconds);

	#ifdef __WIN32__
		inline operator              SyncHandle           () { return (SyncHandle(m_syncSemaphore)); }
	#endif

	  protected: // Members
	#ifdef __POSIX__
		pthread_mutex_t              m_syncAdminLock;
		BinairySemaphore             m_syncMinLimit;
		BinairySemaphore             m_syncMaxLimit;
		unsigned int                 m_nCounter;
		unsigned int                 m_nMaxCount;
	#endif

	#ifdef __WIN32__
		HANDLE						 m_syncSemaphore;
	#endif
	};

	// ===========================================================================
	// class Event
	// ===========================================================================

	class EXTERNAL Event
	{
	  public:    // Methods
		Event  (bool blSet, bool blManualReset);
		~Event ();

		int                          Lock                 ();
		int                          Unlock               ();
		int                          Lock                 (unsigned int nTime);
		void                         ResetEvent           ();
		void                         SetEvent             ();
		void                         PulseEvent           ();

	#ifdef __WIN32__
		inline operator              SyncHandle           () { return (SyncHandle(m_syncEvent)); }
	#endif

	  protected: // Members
		bool                         m_blManualReset;

	#ifdef __POSIX__
		volatile bool                m_blCondition;
		pthread_mutex_t              m_syncAdminLock;
		pthread_cond_t               m_syncCondition;
	#endif

	#ifdef __WIN32__
		HANDLE						 m_syncEvent;
	#endif
	};

	EXTERNAL uint32 InterlockedIncrement (volatile uint32& a_Number);
	EXTERNAL uint32 InterlockedDecrement (volatile uint32& a_Number);

} } // namespace Solutions::Generics

#endif // __SYNC_H
