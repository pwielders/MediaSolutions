#include "Portability.h"
#include "Singleton.h"

namespace Solutions { namespace Generics
{

/* static */ Singleton::SingletonList	Singleton::g_Singletons;

Singleton::SingletonList::SingletonList()
{
}

/* virtual */ Singleton::SingletonList::~SingletonList()
{
	Dispose();
}

void Singleton::SingletonList::Dispose()
{
	// Iterate over all singletons and delete...
	while (m_Singletons.empty() == false)
	{
		Singleton* result = *(m_Singletons.begin());

		m_Singletons.pop_front();

		delete result;
	}
}

void Singleton::SingletonList::Register (Singleton* singleton)
{
	m_Singletons.push_front(singleton);
}

void Singleton::SingletonList::Unregister (Singleton* singleton)
{
	m_Singletons.remove(singleton);
}

} } // namespace Solutions::Generics
