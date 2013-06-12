#ifndef __SESSIONFACTORY_H
#define __SESSIONFACTORY_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "Session.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace Sessions
{
	class EXTERNAL SessionStore : public Generics::SingletonType<SessionStore>
	{
		private:
			typedef std::list<Session*>			SessionInstances;

		private:
			SessionStore (const SessionStore&);
			SessionStore& operator= (const SessionStore&);

		public:
			SessionStore ();
			virtual ~SessionStore ();

		public:
			Session*	FindSessionBySessionId (const Generics::TextFragment& sessionId);

			void		Clear();

		private:
			friend class Session;

			void		Register(Session* sessionInstance);
			void		Unregister(Session* sessionInstance);

		private:
			SessionInstances				m_SessionInstances;
	};

} } // namespace Solutions::Sessions

#endif // __SESSIONFACTORY_H
