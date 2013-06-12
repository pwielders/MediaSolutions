// Signals.h: interface for the SocketPort class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __SIGNALS_H
#define __SIGNALS_H

#ifndef __WIN32__

#include <list>
#include <signal.h>
#include <collection.h>

#include "Portability.h"
#include "thread.h"

#define MAX_SIGNALS  32

namespace Solutions { namespace Generics
{
	interface SignalHandler
	{
		virtual bool SignalHandler () = 0;
	};

	class Signals 
	{
	  typedef List<SignalHandler*>  SubscriberList;

	  // -------------------------------------------------------------------------
	  // This object should not be copied, assigned or created with a default 
	  // constructor. Prevent them from being used, generatoed by the compiler.
	  // define them but do not implement them. Compile error and/or link error.
	  // -------------------------------------------------------------------------
	  private:
		Signals(const Signals& a_RHS);
		Signals& operator= (const Signals& a_RHS);

	  // This class is a singleton to the complete programm. There is only one 
	  // handler per process space possible. To prevent several instantiations
	  // of this object, the one and only constructor, to be implemented of this
	  // clas will be private. It can be called via the static Instance method
	  // on this class.
	  public:
		Signals();
		~Signals();
		static Signals&          Instance        ();
		void                     Signal          (const CThreadControlBlock&  a_Destination,
												 unsigned int                 a_Signal); 

		bool                     Subscribe       (unsigned int a_Signal, SignalHandler& a_Handler);
		bool                     Unsubscribe     (unsigned int a_Signal, SignalHandler& a_Handler);

	  private:
		static void              SignalHandler   (int a_Signal);
		bool                     Notify          (unsigned int a_Signal) const;
		void                     Activate        (unsigned int a_Signal);
		void                     Deactivate      (unsigned int a_Signal);

	  private:
		SubscriberList           m_Subscribers[MAX_SIGNALS];
		struct sigaction         m_OldHandlers[MAX_SIGNALS];
		mutable CriticalSection  m_AdminLock;
		unsigned long            m_SignalSet;
	};

} } // namespace Solutions::Generics

#endif // __WIN32__

#endif // __SIGNALS_H
