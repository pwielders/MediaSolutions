// ===========================================================================
//
// Filename:    sync.cpp
//
// Description: Implementation file for for the CriticalSection, 
//              CBinarySemahore, CountingSemaphore and the Event 
//              synchronisation classes.
//
// History
//
// Author        Reason                                             Date
// ---------------------------------------------------------------------------
// P. Wielders   Initial creation                                   2002/05/24
//
// ===========================================================================

#include "sync.h"

#ifdef __LINUX__
#include <sys/time.h>
#include <asm/errno.h>
#include <unistd.h>
#endif

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// GLOBAL INTERLOCKED METHODS
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

namespace Solutions { namespace Generics
{

CriticalSection	g_SystemLock;

uint32
	InterlockedIncrement (
		volatile uint32&	a_Number)
{
	uint32 l_Result;

	g_SystemLock.Lock ();

	l_Result  = a_Number + 1;
	a_Number = l_Result;

	g_SystemLock.Unlock ();

	return (l_Result);
}

uint32
	InterlockedDecrement (
		volatile uint32& a_Number)
{
	uint32 l_Result;

	g_SystemLock.Lock ();

	l_Result  = a_Number - 1;
	a_Number = l_Result;

	g_SystemLock.Unlock ();

	return (l_Result);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// CriticalSection class 
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// CONSTRUCTOR & DESTRUCTOR
//----------------------------------------------------------------------------
#ifdef __WIN32__
CriticalSection::CriticalSection ()
{
  TRACE_L5 ("Constructor CriticalSection <0x%X>", TRACE_POINTER (this));

  ::InitializeCriticalSection (&m_syncMutex);
}
#endif

#ifdef __LINUX__
CriticalSection::CriticalSection ()
{
  TRACE_L5 ("Constructor CriticalSection <0x%X>", TRACE_POINTER (this));

  pthread_mutexattr_t   structAttributes;
  structAttributes.__mutexkind = PTHREAD_MUTEX_RECURSIVE_NP;

  if (pthread_mutex_init  (&m_syncMutex, &structAttributes) != 0)
  {
    // That will be the day, if this fails...
    ASSERT (false);
  }
}
#endif

#ifdef __DEC__
CriticalSection::CriticalSection ()
{
  TRACE_L5 ("Constructor CriticalSection <0x%X>", TRACE_POINTER (this));

  pthread_mutexattr_t  structAttributes;

  // Create a recursive mutex for this process (no named version, use semaphore)
  if (pthread_mutexattr_init         (&structAttributes) != 0)
  {
    // That will be the day, if this fails...
    ASSERT (false);
  }
  else if (pthread_mutexattr_settype (&structAttributes, PTHREAD_MUTEX_RECURSIVE) != 0)
  {
    // That will be the day, if this fails...
    ASSERT (false);
  }
  else if (pthread_mutex_init        (&m_syncMutex, &structAttributes) != 0)
  {
    // That will be the day, if this fails...
    ASSERT (false);
  }
  else if (pthread_mutexattr_destroy (&structAttributes) != 0)
  {
    // That will be the day, if this fails...
    ASSERT (false);
  }
}
#endif

CriticalSection::~CriticalSection ()
{
  TRACE_L5 ("Destructor CriticalSection <0x%X>", TRACE_POINTER (this));

#ifdef __POSIX__
  if (pthread_mutex_destroy (&m_syncMutex) != 0)
  {
    TRACE_L1 ("Probably trying to delete a used CriticalSection.", 0);
  }
#endif
#ifdef __WIN32__
  ::DeleteCriticalSection (&m_syncMutex);
#endif
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// BinairySemaphore class 
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// CONSTRUCTOR & DESTRUCTOR
//----------------------------------------------------------------------------

// This constructor is to be compatible with the WIN32 CSemaphore class which
// sets the inital count an the maximum count. This way, on platform changes, 
// only the declaration/definition of the synchronisation object has to be defined
// as being Binairy, not the coding.
BinairySemaphore::BinairySemaphore (unsigned int nInitialCount, unsigned int nMaxCount)
{
#if _TRACE_LEVEL > 0
  ASSERT (nMaxCount == 1);
#else
  nMaxCount = nMaxCount;
#endif

  ASSERT ( (nInitialCount == 0) || (nInitialCount == 1) );

  TRACE_L5 ("Constructor BinairySemaphore (int, int)  <0x%X>", TRACE_POINTER (this));

#ifdef __POSIX__
  m_blLocked = (nInitialCount == 0);

  if (pthread_mutex_init (&m_syncAdminLock, NULL) != 0)
  {
    // That will be the day, if this fails...
    ASSERT (false);
  }
  else if (pthread_cond_init (&m_syncCondition, NULL) != 0)
  {
    // That will be the day, if this fails...
    ASSERT (false);
  }
#endif

#ifdef __WIN32__
  m_syncMutex = ::CreateMutex (NULL, (nInitialCount == 0), NULL);

  ASSERT (m_syncMutex != NULL);
#endif
}

BinairySemaphore::BinairySemaphore (bool blLocked)
#ifdef __POSIX__
  : m_blLocked     (blLocked)
#endif
{
  TRACE_L5 ("Constructor BinairySemaphore <0x%X>", TRACE_POINTER (this));

#ifdef __POSIX__
  if (pthread_mutex_init (&m_syncAdminLock, NULL) != 0)
  {
    // That will be the day, if this fails...
    ASSERT (false);
  }
  else if (pthread_cond_init (&m_syncCondition, NULL) != 0)
  {
    // That will be the day, if this fails...
    ASSERT (false);
  }
#endif

#ifdef __WIN32__
  m_syncMutex = ::CreateMutex (NULL, blLocked, NULL);

  ASSERT (m_syncMutex != NULL);
#endif
}

BinairySemaphore::~BinairySemaphore ()
{
  TRACE_L5 ("Destructor BinairySemaphore <0x%X>", TRACE_POINTER (this));

#ifdef __POSIX__
  // If we really create it, we really have to destroy it.
  pthread_mutex_destroy (&m_syncAdminLock);
  pthread_cond_destroy  (&m_syncCondition);
#endif

#ifdef __WIN32__
  ::CloseHandle (m_syncMutex);
#endif
}

//----------------------------------------------------------------------------
// PUBLIC METHODS
//----------------------------------------------------------------------------

int
BinairySemaphore::Lock ()
{
  int nResult = 0;

#ifdef __POSIX__
  // See if we can check the state.
  pthread_mutex_lock (&m_syncAdminLock);

  // We are not busy Setting the flag, so we can check it.
  if (m_blLocked != false)
  {
    do
    {
      // Oops it seems that we are not allowed to pass.
      nResult = pthread_cond_wait (&m_syncCondition, &m_syncAdminLock);

      if (nResult != 0)
      {
        // Something went wrong, so assume...
        TRACE_L5 ("Error waiting for event <%d>.", nResult);
      }

      // For some reason the documentation says that we have to double check on
      // the condition variable to see if we are allowed to fall through, so we
      // do (Guide to DEC threads, March 1996 ,page pthread-56, paragraph 4)
    } while ( (m_blLocked == true) && (nResult == 0) );
  }

  // Seems like we have the token, So the object is locked now.
  m_blLocked = true;

  // Done with the internals of the binairy semphore, everyone can access it again.
  pthread_mutex_unlock (&m_syncAdminLock);
#endif

#ifdef __WIN32__
  nResult = ::WaitForSingleObjectEx (m_syncMutex, INFINITE, FALSE);

  if (nResult == WAIT_OBJECT_0)
  {
	  nResult = OK;
  }
#endif

  // Wait forever so...
  return (nResult);
}

int
BinairySemaphore::Lock (unsigned int nTime)
{
  int nResult = 0;

#ifdef __POSIX__
  if (nTime == INFINITE)
  {
    nResult = Lock ();
  }
  else
  {
    // See if we can check the state.
    pthread_mutex_lock (&m_syncAdminLock);

    // We are not busy Setting the flag, so we can check it.
    if (m_blLocked != false)
    {
      struct timespec structTime;

#ifdef __LINUX__

      struct timeval structDelta;
      gettimeofday(&structDelta, NULL);
      structTime.tv_sec  = structDelta.tv_sec + nTime; 
      structTime.tv_nsec = 0;
#endif

#ifdef __DEC__

      struct timespec structDelta;

      // Convert the given time to a delta time.
      structDelta.tv_sec  = nTime;
      structDelta.tv_nsec = 0;
      pthread_get_expiration_np (&structDelta, &structTime);

#endif
    
      do
      {
        // Oops it seems that we are not allowed to pass.
        nResult = pthread_cond_timedwait (&m_syncCondition, &m_syncAdminLock, &structTime);

        if (nResult == ETIMEDOUT)
        {
          // Something went wrong, so assume...
          TRACE_L5 ("Timed out waiting for event <%d>.", nTime);
        }
        else if (nResult != 0)
        {
          // Something went wrong, so assume...
          TRACE_L5 ("Waiting on semaphore failed. Error code <%d>", nResult);        
        }
     
        // For some reason the documentation says that we have to double check on
        // the condition variable to see if we are allowed to fall through, so we
        // do (Guide to DEC threads, March 1996 ,page pthread-56, paragraph 4)
      } while ( (m_blLocked == true) && (nResult == 0) );
    }
    

    // Seems like we have the token, So the object is locked now.
    m_blLocked = true;

    // Done with the internals of the binairy semphore, everyone can access it again.
    pthread_mutex_unlock (&m_syncAdminLock);
  }
#endif

#ifdef __WIN32__
  nResult = ::WaitForSingleObjectEx (m_syncMutex, nTime, FALSE);

  if (nResult == WAIT_OBJECT_0)
  {
	  nResult = OK;
  }
#endif

  // Timed out or did we get the token ?
  return (nResult);
}

void
BinairySemaphore::Unlock ()
{

#ifdef __POSIX__
  // See if we can get access to the data members of this object.
  pthread_mutex_lock (&m_syncAdminLock);

  // Yep, that's it we are no longer locked. Signal the change.
  m_blLocked = false;

  // O.K. that is arranged, Now we should at least signal the first 
  // waiting process that is waiting for this condition to occur.
  pthread_cond_signal (&m_syncCondition);

  // Now that we are done with the variablegive other threads access 
  // to the object again.
  pthread_mutex_unlock (&m_syncAdminLock);
#endif

#ifdef __WIN32__
  ::ReleaseMutex (m_syncMutex);
#endif
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// CountingSemaphore class 
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// CONSTRUCTOR & DESTRUCTOR
//----------------------------------------------------------------------------

CountingSemaphore::CountingSemaphore (
  unsigned int             nInitialCount,
  unsigned int             nMaxCount) 
#ifdef __POSIX__
  : m_syncMinLimit	       (false),
    m_syncMaxLimit         (false),
    m_nCounter             (nInitialCount),
    m_nMaxCount            (nMaxCount)  
#endif
{
  TRACE_L5 ("Constructor CountingSemaphore <0x%X>", TRACE_POINTER (this));

#ifdef __POSIX__

  if (pthread_mutex_init (&m_syncAdminLock, NULL) != 0)
  {
    // That will be the day, if this fails...
    ASSERT (false);
  }

  // Well that is it, see if one of the Limit locks should be taken ?
  if (m_nCounter == 0)
  {
    // This should be possible since we created them Not Locked.
    m_syncMinLimit.Lock ();
  }

  // Or maybe we are at the upper limit ?
  if (m_nCounter == m_nMaxCount)
  {
    // This should be possible since we created them Not Locked.
    m_syncMaxLimit.Lock ();
  }
#endif

#ifdef __WIN32__
  m_syncSemaphore = ::CreateSemaphore (NULL, nInitialCount, nMaxCount, NULL);

  ASSERT (m_syncSemaphore != NULL);
#endif
}
    
CountingSemaphore::~CountingSemaphore ()
{
  TRACE_L5 ("Destructor CountingSemaphore <0x%X>", TRACE_POINTER (this));

#ifdef __POSIX__
  // O.K. Destroy all the semaphores used by this class.
  pthread_mutex_destroy (&m_syncAdminLock); 
#endif

#ifdef __WIN32__
  ::CloseHandle (m_syncSemaphore);
#endif
}

//----------------------------------------------------------------------------
// PUBLIC METHODS
//----------------------------------------------------------------------------

int
  CountingSemaphore::Lock ()
{

#ifdef __POSIX__
  // First see if we could still decrease the count..
  int nResult = m_syncMinLimit.Lock ();

  // See that we were able to get the semaphore.
  if (nResult == 0)
  {
    // If we have this semaphore, no Other lock can take place 
    // now make sure that the counter is handled atomic. Get the 
    // administration lock (unlock can still access it).
    pthread_mutex_lock (&m_syncAdminLock);

    // Now we are in the clear, Lock cannot access this (blocked 
    // on MinLimit) and unlock cannot access the counter (blocked on 
    // m_syncAdminLock). Work the Semaphore counter. It's safe.

    // If we leave the absolute max position, make sure we release
    // the MaxLimit synchronisation. 
    if (m_nCounter == m_nMaxCount) m_syncMaxLimit.Unlock ();
  
    // Now update the counter.
    m_nCounter--;

    // See if the counter can still be decreased.
    if (m_nCounter != 0) m_syncMinLimit.Unlock ();

    // Now we are completely done with the counter and it's logic. Free all 
    // waiting threads for this resource.
    pthread_mutex_unlock (&m_syncAdminLock);
  }
#endif

#ifdef __WIN32__
  int nResult = ::WaitForSingleObjectEx (m_syncSemaphore, INFINITE, FALSE);

  if (nResult == WAIT_OBJECT_0)
  {
	  nResult = OK;
  }
#endif

  return (nResult);
}

int
  CountingSemaphore::Lock (unsigned int nSeconds)
{
#ifdef __POSIX__
  // First see if we could still decrease the count..
  int nResult = m_syncMinLimit.Lock (nSeconds);

  // See that we were able to get the semaphore.
  if (nResult == 0)
  {
    // If we have this semaphore, no Other lock can take place 
    // now make sure that the counter is handled atomic. Get the 
    // administration lock (unlock can still access it).
    pthread_mutex_lock (&m_syncAdminLock);

    // Now we are in the clear, Lock cannot access this (blocked 
    // on MinLimit) and unlock cannot access the counter (blocked on 
    // m_syncAdminLock). Work the Semaphore counter. It's safe.

    // If we leave the absolute max position, make sure we release
    // the MaxLimit synchronisation. 
    if (m_nCounter == m_nMaxCount) m_syncMaxLimit.Unlock ();
  
    // Now update the counter.
    m_nCounter--;

    // See if the counter can still be decreased.
    if (m_nCounter != 0) m_syncMinLimit.Unlock ();

    // Now we are completely done with the counter and it's logic. Free all 
    // waiting threads for this resource.
    pthread_mutex_unlock (&m_syncAdminLock);
  }
#endif

#ifdef __WIN32__
  int nResult = ::WaitForSingleObjectEx (m_syncSemaphore, nSeconds, FALSE);

  if (nResult == WAIT_OBJECT_0)
  {
	  nResult = OK;
  }
#endif

  return (nResult);
}

int
  CountingSemaphore::Unlock (unsigned int nCount)
{
  ASSERT (nCount != 0);

#ifdef __POSIX__
  // First see if we could still increase the count..
  int nResult = m_syncMaxLimit.Lock (INFINITE);

  // See that we were able to get the semaphore.
  if (nResult == 0)
  {
    // If we have this semaphore, no Other lock can take place 
    // now make sure that the counter is handled atomic. Get the 
    // administration lock (unlock can still access it).
    pthread_mutex_lock (&m_syncAdminLock);

    // Now we are in the clear, Unlock cannot access this (blocked on
    // MaxLimit) and Lock cannot access the counter (blocked on 
    // m_syncAdminLock). Work the Semaphore counter. It's safe.

    // If we leave the absolute min position (0), make sure we signal
    // the Lock proCess, give the MinLimit synchronisation free.
    if (m_nCounter == 0) m_syncMinLimit.Unlock ();

    // See if the given count 
    m_nCounter += nCount;

    // See if we reached or overshot the max ?
    if (m_nCounter > m_nMaxCount)
    {
      // Release the Admin Semephore so the Lock on the max limit
      //  can proceed.
      pthread_mutex_unlock (&m_syncAdminLock);

      // Seems like we added more than allowed, so wait till the Max
      // mutex get's unlocked by the Lock process.
      m_syncMaxLimit.Lock (INFINITE);

      // Before we continue processing, Get the administrative lock
      // again.
      pthread_mutex_lock (&m_syncAdminLock);
    }

    // See if we are still allowed to increase the counter.
    if (m_nCounter != m_nMaxCount) m_syncMaxLimit.Unlock ();

    // Now we are completely done with the counter and it's logic. Free all 
    // waiting threads for this resource.
    pthread_mutex_unlock (&m_syncAdminLock);
  }
#endif

#ifdef __WIN32__
  int nResult = OK;

  if (::ReleaseSemaphore (m_syncSemaphore, nCount, NULL) == FALSE)
  {
	  // Could not give all tokens.
	  ASSERT (false);
  }
#endif

  return (nResult);
}

#ifdef __POSIX__
int CountingSemaphore::TryUnlock (unsigned int nSeconds)
#else
int CountingSemaphore::TryUnlock (unsigned int /* nSeconds */)
#endif
{
#ifdef __POSIX__
  // First see if we could still increase the count..
  int nResult = m_syncMaxLimit.Lock (nSeconds);

  // See that we were able to get the semaphore.
  if (nResult == 0)
  {
    // If we have this semaphore, no Other lock can take place
    // now make sure that the counter is handled atomic. Get the
    // administration lock (unlock can still access it).
    pthread_mutex_lock (&m_syncAdminLock);

    // Now we are in the clear, Unlock cannot access this (blocked on
    // MaxLimit) and Lock cannot access the counter (blocked on
    // m_syncAdminLock). Work the Semaphore counter. It's safe.

    // If we leave the absolute min position (0), make sure we signal
    // the Lock process, give the MinLimit synchronisation free.
    if (m_nCounter == 0) m_syncMinLimit.Unlock ();

    // Now update the counter.
    m_nCounter++;

    // See if we are still allowed to increase the counter.
    if (m_nCounter != m_nMaxCount) m_syncMaxLimit.Unlock ();

    // Now we are completely done with the counter and it's logic. Free all
    // waiting threads for this resource.
    pthread_mutex_unlock (&m_syncAdminLock);
  }
#endif

#ifdef __WIN32__
  int nResult = OK;

  if (::ReleaseSemaphore (m_syncSemaphore, 1, NULL) == FALSE)
  {
	// Wait for the given time to see if we can "give" the lock.
    // To be Coded.
	ASSERT (false);
  }
  
#endif

  return (nResult);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Event class (AVAILABLE WITHIN PROCESS SPACE)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// CONSTRUCTOR & DESTRUCTOR
//----------------------------------------------------------------------------

Event::Event         (bool blSet, bool blManualReset) :
  m_blManualReset      (blManualReset)
#ifdef __POSIX__
  ,m_blCondition       (blSet)
#endif
{
  TRACE_L5 ("Constructor Event <0x%X>", TRACE_POINTER (this));

#ifdef __POSIX__
  if (pthread_mutex_init (&m_syncAdminLock, NULL) != 0)
  {
    // That will be the day, if this fails...
    ASSERT (false);
  }
  else if (pthread_cond_init (&m_syncCondition, NULL) != 0)
  {
    // That will be the day, if this fails...
    ASSERT (false);
  }
#endif 

#ifdef __WIN32__
  m_syncEvent = ::CreateEvent (NULL, blManualReset, blSet, NULL);

  ASSERT (m_syncEvent != NULL);
#endif
}

Event::~Event ()
{

#ifdef __POSIX__
	TRACE_L5 ("Destructor Event <0x%X>", TRACE_POINTER (this));

  // If we really create it, we really have to destroy it.
  pthread_mutex_destroy (&m_syncAdminLock);
  pthread_cond_destroy  (&m_syncCondition);
#endif

#ifdef __WIN32__
  ::CloseHandle (m_syncEvent);
#endif
}

//----------------------------------------------------------------------------
// PUBLIC METHODS
//----------------------------------------------------------------------------

int
Event::Lock ()
{
  int nResult = 0;

#ifdef __POSIX__
  // See if we can check the state.
  pthread_mutex_lock (&m_syncAdminLock);

  // We are not busy Setting the flag, so we can check it.
  if (m_blCondition == false)
  {
    do
    {
      // Oops it seems that we are not allowed to pass.
      nResult = pthread_cond_wait (&m_syncCondition, &m_syncAdminLock);

      if (nResult != 0)
      {
        // Something went wrong, so assume...
        TRACE_L5 ("Error waiting for event <%d>.", nResult);
      }

      // For some reason the documentation says that we have to double check on
      // the condition variable to see if we are allowed to fall through, so we
      // do (Guide to DEC threads, March 1996 ,page pthread-56, paragraph 4)
    } while ( (m_blCondition == false) && (nResult == 0) );
  }

  // Seems that the event is triggered, lets continue. but
  // do not forget to give back the flag..
  pthread_mutex_unlock (&m_syncAdminLock);
#endif

#ifdef __WIN32__
  nResult = ::WaitForSingleObjectEx (m_syncEvent, INFINITE, FALSE);

  if (nResult == WAIT_OBJECT_0)
  {
	  nResult = OK;
  }
#endif
  // Wait forever so...
  return (nResult);
}

int
Event::Lock (unsigned int nTime)
{
  int nResult = 0;

#ifdef __POSIX__
  if (nTime == INFINITE)
  {
    nResult = Lock ();
  }
  else
  {
    // See if we can check the state.
    pthread_mutex_lock (&m_syncAdminLock);

    // We are not busy Setting the flag, so we can check it.
    if (m_blCondition == false)
    {
      struct timespec structTime;

#ifdef __LINUX__
      struct timeval    structDelta;
      gettimeofday(&structDelta, NULL);
      structTime.tv_sec  = structDelta.tv_sec + nTime; 
      structTime.tv_nsec = 0;
#endif

#ifdef __DEC__
      struct timespec structDelta;

      // Convert the given time to a delta time.
      structDelta.tv_sec  = nTime;
      structDelta.tv_nsec = 0;
      pthread_get_expiration_np (&structDelta, &structTime);
#endif

      do
      {
        // Oops it seems that we are not allowed to pass.
        nResult = pthread_cond_timedwait (&m_syncCondition, &m_syncAdminLock, &structTime);

        if (nResult != 0)
        {
          // Something went wrong, so assume...
          TRACE_L5 ("Timed out waiting for event <%d>!", nResult);
        }
     
        // For some reason the documentation says that we have to double check on
        // the condition variable to see if we are allowed to fall through, so we
        // do (Guide to DEC threads, March 1996 ,page pthread-56, paragraph 4)
      } while ( (m_blCondition == false) && (nResult == 0) );
    }
    
    // Seems that the event is triggered, lets continue. but
    // do not forget to give back the flag..
    pthread_mutex_unlock (&m_syncAdminLock);
  }
#endif

#ifdef __WIN32__
  nResult = ::WaitForSingleObjectEx (m_syncEvent, nTime, FALSE);

  if (nResult == WAIT_OBJECT_0)
  {
	  nResult = OK;
  }
#endif

  return (nResult);
}

int
Event::Unlock ()
{
  int nResult = 0;

#ifdef __POSIX__
  // See if we can get access to the data members of this object.
  pthread_mutex_lock (&m_syncAdminLock);

  // Yep, that's it we are no longer locked. Signal the change.
  m_blCondition = true;

  // O.K. that is arranged, Now we should at least signal the first
  // waiting process that is waiting for this condition to occur.
  pthread_cond_signal (&m_syncCondition);

  // Now that we are done with the variablegive other threads access
  // to the object again.
  pthread_mutex_unlock (&m_syncAdminLock);
#endif

#ifdef __WIN32__
  if (m_blManualReset)
  {
	  ::SetEvent (m_syncEvent);
  }
  else
  {
	  ::PulseEvent (m_syncEvent);
  }
#endif

  return (nResult);
}

void
Event::ResetEvent ()
{
#ifdef __POSIX__
  // See if we can check the state.
  pthread_mutex_lock (&m_syncAdminLock);

  // We are the onlyones who can access the data, time to update it.
  m_blCondition = false;

  // Done changing the data, free other threads so the can use this
  // object again 
  pthread_mutex_unlock (&m_syncAdminLock);
#endif

#ifdef __WIN32__
  ::ResetEvent (m_syncEvent);
#endif
}

void
Event::SetEvent ()
{
#ifdef __POSIX__
  // See if we can get access to the data members of this object.
  pthread_mutex_lock (&m_syncAdminLock);

  // Yep, that's it we are signalled, Broadcast the change.
  m_blCondition = true;

  // O.K. that is arranged, Now we should at least signal waiting
  // process that the event has occured.
  pthread_cond_broadcast (&m_syncCondition);

  // All waiting threads are now in the running mode again. See 
  // if the event should be cleared manually again.
  if (m_blManualReset == false)
  {
    // Make sure all threads are in running mode, place our request
    // for sync at the end of the FIFO-queue for syncConditionMutex.
    pthread_mutex_unlock (&m_syncAdminLock);
    ::SleepMs (0);
    pthread_mutex_lock   (&m_syncAdminLock);

    // They all had a change to continue so, now it is over, we can
    // not wait forever......
    m_blCondition = false;
  }

  // Now that we are done with the variablegive other threads access 
  // to the object again.
  pthread_mutex_unlock (&m_syncAdminLock);
#endif

#ifdef __WIN32__
  ::SetEvent (m_syncEvent);
#endif
}

void
Event::PulseEvent ()
{
#ifdef __POSIX__
  // See if we can get access to the data members of this object.
  pthread_mutex_lock (&m_syncAdminLock);

  // Yep, that's it we are signalled, Broadcast the change.
  m_blCondition = true;

  // O.K. that is arranged, Now we should at least signal waiting
  // process that the event has occured.
  pthread_cond_broadcast (&m_syncCondition);

  // Make sure all threads are in running mode, place our request
  // for sync at the end of the FIFO-queue for syncConditionMutex.
  pthread_mutex_unlock (&m_syncAdminLock);
  ::SleepMs (0);
  pthread_mutex_lock   (&m_syncAdminLock);

  // They all had a change to continue so, now it is over, we can
  // not wait forever......
  m_blCondition = false;

  // Now that we are done with the variablegive other threads access
  // to the object again.
  pthread_mutex_unlock (&m_syncAdminLock);
#endif

#ifdef __WIN32__
  ::PulseEvent (m_syncEvent);
#endif
}

} } // namespace Solutions::Generics
