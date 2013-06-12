#include "SessionStore.h"

namespace Solutions { namespace Sessions
{

SessionStore::SessionStore () :
	m_SessionInstances ()
{
}

/* virtual */ SessionStore::~SessionStore ()
{
}

Session* SessionStore::FindSessionBySessionId (const Generics::TextFragment& sessionId)
{
	Session* result = NULL;

	// Yip we have, first see if we have a session we can use..
	SessionInstances::iterator instanceIndex = m_SessionInstances.begin();

	while ( (result == NULL) && (instanceIndex != m_SessionInstances.end()) )
	{
		if (sessionId == (*instanceIndex)->SessionId())
		{
			result = *instanceIndex;
		}
		else
		{
			instanceIndex++;
		}
	}

	return (result);
}

void SessionStore::Register(Session* sessionInstance)
{
	m_SessionInstances.push_back (sessionInstance);
}

void SessionStore::Unregister(Session* sessionInstance)
{
	m_SessionInstances.remove (sessionInstance);
}

void SessionStore::Clear()
{
	while ( m_SessionInstances.empty() == false )
	{
		// Deleting the session will unregister it...
		delete m_SessionInstances.front();
	}
}

} } // namespace Solutions::Sessions

