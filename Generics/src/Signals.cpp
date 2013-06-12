// Signals.cpp: interface for the SocketPort class.
//
//////////////////////////////////////////////////////////////////////

#include "Portability.h"

#ifndef __WIN32__

#include "signals.h"

#include <errno.h>

#define SIGNALMAP(index)   (1<<(index-1))

namespace Solutions { namespace Generics
{

// Make a singleton entry. Linux has some problems killing it is placed inside a class scope.
Signals g_SignalHandler;

/* static */ Signals&
  Signals::Instance ()
{
  return ( g_SignalHandler );
}

/* static */ void
  Signals::SignalHandler (
    int         a_Signal) 
{
  if (Signals::Instance ().Notify (a_Signal) == false)
  {
    if ((Signals::Instance ()).m_OldHandlers[a_Signal-1].sa_handler != NULL)
    {
      // Seems like the signal is not handled, call the base.
      (Signals::Instance ()).m_OldHandlers[a_Signal-1].sa_handler (a_Signal);
    }
  }
}

Signals::Signals   () :
  m_Subscribers    (),
  m_AdminLock      (),
  m_SignalSet      (0),
  m_OldHandlers    ()
{
  TRACE_L5 ("Constructor Signals<0x%X>", TRACE_POINTER (this));
}

Signals::~Signals ()
{
  TRACE_L5 ("Destructor Signals<0x%X>", TRACE_POINTER (this));

  // Clear all activated handlers.
  for (unsigned int l_Teller = 1; l_Teller <= MAX_SIGNALS; l_Teller++)
  {
    if ((m_SignalSet & SIGNALMAP(l_Teller)) != 0) 
    {
      Deactivate (l_Teller);
    }
  }
}

void
  Signals::Signal (const CThreadControlBlock& a_Destination, unsigned int a_Signal)
{
  ::pthread_kill (reinterpret_cast <pthread_t>(a_Destination.ID ()), a_Signal);
}

bool 
  Signals::Subscribe (
    unsigned int     a_Signal,
    SignalHandler&  a_Handler)
{
  // Make sure there is no precondition violation
  ASSERT ( (a_Signal > 0) && (a_Signal <= MAX_SIGNALS) );

  // About to change some administrational stuff. Make it atomic.
  m_AdminLock.Lock ();

  // Add this handler to the set of interested handlers.
  m_Subscribers[a_Signal-1].Add (&a_Handler);

  // Are we already waiting for this signal ?
  if ((SIGNALMAP(a_Signal) & m_SignalSet) == 0)
  {
    // Install the signal handler for this signal.
    Activate (a_Signal);
  }

  // Done changing. Release the admin lock.
  m_AdminLock.Unlock ();

  return (true);
}

bool 
  Signals::Unsubscribe (
    unsigned int     a_Signal,
    SignalHandler&  a_Handler)
{
  bool l_Result;

  // Make sure there is no precondition violation
  ASSERT ( (a_Signal > 0) && (a_Signal <= MAX_SIGNALS) );
  ASSERT ( (SIGNALMAP(a_Signal) & m_SignalSet) != 0 );

  // About to change some administrational stuff. Make it atomic.
  m_AdminLock.Lock ();

  // Remove this handler to the set of interested handlers.
  l_Result = m_Subscribers[a_Signal-1].Remove (&a_Handler);

  // Done changing. Release the admin lock.
  m_AdminLock.Unlock ();

  return (l_Result);
}

void
  Signals::Activate (
    unsigned int     a_Signal)
{
  struct sigaction  l_Handler;

  ASSERT ( (a_Signal > 0) && (a_Signal <= MAX_SIGNALS) );
  ASSERT ( (m_SignalSet & SIGNALMAP(a_Signal)) == 0 );

  sigemptyset(&l_Handler.sa_mask);
  l_Handler.sa_flags   = 0;
  l_Handler.sa_handler = Signals::SignalHandler;

  if ( sigaction (a_Signal, &l_Handler, &m_OldHandlers[a_Signal-1]) == -1 )
  {
    // It seems like we could not install a handler. Reason for an assert.
    TRACE_L5 ("Error seting handler: %d", errno);
  }
  else
  {
    // Add this as a trigger to the mask.
    m_SignalSet |= SIGNALMAP(a_Signal);   
  }
}

void
  Signals::Deactivate (
    unsigned int    a_Signal)
{
  struct sigaction  l_Handler;

  ASSERT ( (a_Signal > 0) && (a_Signal <= MAX_SIGNALS) );
  ASSERT ( (m_SignalSet & SIGNALMAP(a_Signal)) != 0 );

  if ( sigaction (a_Signal, &m_OldHandlers[a_Signal-1], &l_Handler) == -1 )
  {
    TRACE_L5 ("Error restoring handler: %d", errno);
  }
  else
  {
    // Remove this as a trigger from the mask.
    m_SignalSet ^= SIGNALMAP(a_Signal);   
  }
}

bool
  Signals::Notify (unsigned int a_Signal) const
{
  unsigned int            l_Max;
  bool                    l_Result = false;
  const CSubscriberList&  l_List   = m_Subscribers[a_Signal-1];
  TCHAR                   l_Info[40]; 

  // About to change some administrational stuff. Make it atomic.
  m_AdminLock.Lock ();

  // Optimize for speed....
  l_Max = l_List.Count ();

  for (unsigned int l_Teller = 0; l_Teller < l_Max; l_Teller++)
  {
    ASSERT (l_List[l_Teller] != NULL);

    l_Result |= l_List[l_Teller]->SignalHandler ();
  }

  // Done changing. Release the admin lock.
  m_AdminLock.Unlock ();

  // Return the fact if it handled somthing or not.
  return (l_Result);
}

} } // namespace Solutions::Generics

#endif // __WIN32__