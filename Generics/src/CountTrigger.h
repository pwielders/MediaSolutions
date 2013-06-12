// CountTrigger.h: interface for the CountTrigger class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __COUNTTRIGGER_H
#define __COUNTTRIGGER_H

#include "Module.h"
#include "sync.h"

namespace Solutions { namespace Generics
{
	class EXTERNAL CountTrigger
	{
	  private:
		CountTrigger (const CountTrigger& a_Copy);
		CountTrigger& operator= (const CountTrigger& a_RHS);

	  public:
		CountTrigger();
		virtual ~CountTrigger();

	  public:
		void               Lock           () const;
		void               UnlockOnCount  (unsigned int a_Count) const;

	  private:
		mutable Event             m_LockTrigger;
		mutable unsigned          m_Count;
		mutable bool              m_UnlockOnCount;
		mutable CriticalSection   m_AdminSync;
		mutable Semaphore         m_Refresh;
	};

} } // namespace Solutions::Generics

#endif // __COUNTTRIGGER_H
