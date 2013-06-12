#include "Thread.h"
#include "Proxy.h"

#ifdef __WIN32__
#include <process.h>
#endif

//-----------------------------------------------------------------------------------------------
// CLASS: Thread
//-----------------------------------------------------------------------------------------------

namespace Solutions { namespace Generics
{

Thread::Thread(
  const bool              blReleased,
  const TCHAR*			  threadName) :
    m_hThreadInstance    (),
    m_syncAdmin          (),
    m_enumState          (BLOCKED),
    m_sigExit            (false, true)
{
  TRACE_L5 ("Constructor Thread <0x%X>", TRACE_POINTER (this));

  // Create a worker that can do actions in parallel
#ifdef __WIN32__

  m_hThreadInstance = ::CreateThread (NULL, 
                    0, 
                    (LPTHREAD_START_ROUTINE) Thread::StartThread, 
                    (LPVOID) this,
                    0,
                    &m_ThreadId);

        // If there is no thread, the "new" thread can also not free the destructor,
        // then it is up to us.
        if (m_hThreadInstance == NULL)
#endif

#ifdef __POSIX__
        // If there is no thread, the "new" thread can also not free the destructor,
        // then it is up to us.
        if (pthread_create (&m_hThreadInstance, NULL, (void *(*)(void *)) Thread::StartThread, this) == static_cast <unsigned long> (-1))
#endif
        {
                // Creation failed, O.K. We will signal the inactive state our selves.
			m_sigExit.SetEvent ();
        }
  else if (blReleased != false)
  {
    State (RUNNING);
  }

	if (threadName != NULL)
	{
		ThreadName (threadName);
	}
}

Thread::~Thread()
{
  TRACE_L5 ("Destructor Thread <0x%X>", TRACE_POINTER (this));

  Terminate ();
}

#ifdef __WIN32__
void Thread::StartThread(Thread* cClassPointer)
#endif

#ifdef __POSIX__
void*  Thread::StartThread(Thread* cClassPointer)
#endif
{
  CriticalSection& adminLock = cClassPointer->m_syncAdmin;
  StateTrigger<thread_state>& stateObject = cClassPointer->m_enumState;

  // O.K. befor using the state, lock it.
  adminLock.Lock();

  // Do the work that is requested.
  while ( stateObject != STOPPED )
  {
    unsigned int delayed = INFINITE;

	if (stateObject == RUNNING)
    {
      // O.K. befor using the state, lock it.
      adminLock.Unlock ();

      delayed = cClassPointer->Worker ();

      // Change the state, we are done with it.
      adminLock.Lock ();
    }
    
	// Hold the thread untill the right state has been reached !!
	while ((stateObject != RUNNING) && (stateObject != STOPPED))
	{
      // O.K. befor using the state, lock it.
      adminLock.Unlock ();

      // Wait till we reach a runnable state
	  stateObject.WaitState (RUNNING|STOPPED|DEACTIVATE, delayed);

      // Change the state, we are done with it.
      adminLock.Lock ();

	  // Check in which state we reached the criteria for the WaitState !!
      if (stateObject == DEACTIVATE)
	  {
		cClassPointer->State (BLOCKED);
	  }
	  else if (stateObject == DELAYED)
	  {
        cClassPointer->State (RUNNING);
	  }

	  // We can only delay once, given by the worker. The next requires an active 
	  // trigger to wake up.
      delayed = INFINITE;
    }
  }

  // O.K. befor using the state, lock it.
  adminLock.Unlock ();

  // Report that the worker is done by releasing the Signal sync mechanism.
  cClassPointer->m_sigExit.SetEvent ();

#ifdef __POSIX__
  ::pthread_exit (NULL);
  return (NULL);
#endif

#ifdef __WIN32__
  ::ExitThread (0);
#endif
}

void Thread::Terminate ()
{
  // Make sure the thread knows that it should stop.
  m_syncAdmin.Lock ();

  State (STOPPED);

  m_syncAdmin.Unlock ();

#ifdef __WIN32__
  // Resume the thread, it might be suspended.
  ::ResumeThread (m_hThreadInstance);
#endif

  // We have to wait till the thread is completely stopped. If we 
  // would continue here, the memory allocated for this instance of
  // this class would be freed. If our worker thread would still 
  // be running, it will still reference to the data stored in the
  // allocated memory for the instance of this class. In the mean 
  // time this data might have been taken by another thread/procces
  // and thay might have changed the data with all consequences, so
  // let's wait until we are triggered that the thread is dead.
  // We do not want to be busy waiting so do it via a synchronisation
  // mechanism, e.g. the semaphore created during construction
  // time.

  m_sigExit.Lock (INFINITE);

#ifdef __POSIX__
  void* l_Dummy;
  ::pthread_join (m_hThreadInstance, &l_Dummy);
#endif
}

void Thread::Suspend()
{
#ifdef  __POSIX__
  Block ();
#else
  State (SUSPENDED);
#endif
}

void
Thread::Block ()
{
  m_syncAdmin.Lock ();

  State (DEACTIVATE);

  m_syncAdmin.Unlock ();
}

void
Thread::Delay ()
{
  m_syncAdmin.Lock ();

  State (DELAYED);

  m_syncAdmin.Unlock ();
}

void
Thread::Run ()
{
  m_syncAdmin.Lock ();

  State (RUNNING);

  m_syncAdmin.Unlock ();
}

bool Thread::Wait (const unsigned int enumState, unsigned int nTime) const
{
  return (m_enumState.WaitState (enumState, nTime));
}

Thread::thread_state Thread::State () const
{
  // Return the current status of the worker thread.
  return (m_enumState);
}

bool 
Thread::State (Thread::thread_state enumNewState)
{
  bool blOK = false;

  switch (m_enumState)
  {
    case SUSPENDED:  blOK = ( (enumNewState == RUNNING)    ||
                              (enumNewState == STOPPED)    );
                     break;
    case RUNNING:    blOK = ( (enumNewState == SUSPENDED)  ||
                              (enumNewState == BLOCKED)    ||
                              (enumNewState == DEACTIVATE) ||
                              (enumNewState == STOPPED)    ||
							  (enumNewState == DELAYED)    );
                     break;
    case DEACTIVATE: blOK = ( (enumNewState == SUSPENDED)  ||
                              (enumNewState == BLOCKED)    ||
                              (enumNewState == RUNNING)    ||
                              (enumNewState == STOPPED));
                     break;
    case BLOCKED:    blOK = ( (enumNewState == SUSPENDED)  ||
                              (enumNewState == RUNNING)    ||
                              (enumNewState == STOPPED)    );
                     break;
    case DELAYED:    blOK = ( (enumNewState == SUSPENDED)  ||
                              (enumNewState == RUNNING)    ||
                              (enumNewState == STOPPED)    ||
							  (enumNewState == DEACTIVATE) );
                     break;
    case STOPPED:    // The STOPPED state is the end,
                     // No changes possible anymore.
                     blOK = (enumNewState == STOPPED);
                     break;
  }

  if (blOK)
  {
#ifndef __POSIX__
    if (enumNewState == SUSPENDED)
    {
      m_enumSuspendedState = m_enumState;
      m_enumState.SetState (SUSPENDED);

      // O.K. Suspend this thread.
      ::SuspendThread (m_hThreadInstance);      
    }
    else if (m_enumState == SUSPENDED)
    {
      m_enumState.SetState (m_enumSuspendedState);

      // Done in the suspended state, resume.
      ::ResumeThread (m_hThreadInstance);
    }
    else
#endif
    {
      m_enumState.SetState (enumNewState);
    }
  }

  return (blOK);
}

void Thread::ThreadName (const TCHAR* threadName)
{
#ifdef __WIN32__

#pragma pack(push,8)

	struct tagTHREADNAME_INFO
	{
		DWORD dwType; // Must be 0x1000.
		LPCSTR szName; // Pointer to name (in user addr space).
		DWORD dwThreadID; // Thread ID (-1=caller thread).
		DWORD dwFlags; // Reserved for future use, must be zero.
	} info;

#pragma pack(pop)

   info.dwType = 0x1000;
   info.szName = threadName;
   info.dwThreadID = m_ThreadId;
   info.dwFlags = 0;

   __try
   {
      RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
   }
   __except(EXCEPTION_EXECUTE_HANDLER)
   {
   }

#endif // __WIN32__
}

} } // namespace Solutions::Generics
