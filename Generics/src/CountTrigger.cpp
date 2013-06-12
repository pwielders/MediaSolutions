// CountTrigger.cpp: implementation of the CountTrigger class.
//
//////////////////////////////////////////////////////////////////////

#include "CountTrigger.h"

namespace Solutions { namespace Generics
{
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CountTrigger::CountTrigger () :
  m_AdminSync                (),
  m_Refresh                  (0, 1),
  m_Count                    (0),
  m_UnlockOnCount            (false),
  m_LockTrigger              (true, false)
{
}

CountTrigger::~CountTrigger()
{
}

void
  CountTrigger::Lock () const
{
  m_AdminSync.Lock   ();

  // Inform the UnlockCountOn that it should re-evaluate.
  m_Refresh.Unlock ();

  // Announce the fact that we are waiting as well.
  m_Count++;

  m_AdminSync.Unlock ();

  // Wait till we can contineue. The UnlockOnCount has
  // reached it's set value.
  m_LockTrigger.Lock ();

  // Announce the fact that we are no longer waiting.
  m_Count--;
}

void 
  CountTrigger::UnlockOnCount  (unsigned int a_Count) const
{
  m_AdminSync.Lock   ();

  // See if we are allowed to do this. Only one thread at a 
  // time is allowed to call this.
  ASSERT (m_UnlockOnCount == false);

  m_UnlockOnCount = true;

  // As long as the requested count is not reached, we wait.
  while (m_Count < a_Count)
  {
    m_AdminSync.Unlock ();

    // If there is an update/change, trigger us.
    m_Refresh.Lock ();

    // Make sure no new updates arrive.
    m_AdminSync.Lock   ();
  }

  // We reached the requested amounted of threads, Release them.
  m_LockTrigger.SetEvent ();

  // Wait till we reach 0, this means that all the process 
  // that should have been released by the UnlockOnCount are
  // released.
  while (m_Count != 0)
  {
    ::SleepMs (0);
  }

  // The next bunch of processes has to wait for the next LockOnCount.
  m_LockTrigger.ResetEvent ();

  m_UnlockOnCount = false;

  // All right new requests cabn come in.
  m_AdminSync.Unlock ();
}

} } // namespace Solutions::Generics

