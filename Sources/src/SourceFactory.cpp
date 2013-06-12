#include "SourceFactory.h"

namespace Solutions { namespace Source
{

Factory::Factory() :
	m_Admin(),
	m_CleanupTask(_T("SourceInstanceCleaner")),
	m_FactoryDefinitions(),
	m_AvailableSources()
{
	m_CleanupTask.Schedule(Generics::Time::Now().Ticks() + (PERIODIC_CLEANUP_TIME * 1000 * TICKS_PER_MILLISECONDS), this);
}

Factory::~Factory()
{
	// Loose all registrations
	std::list<ISourceFactory*>::iterator index = m_FactoryDefinitions.begin();

	while (index != m_FactoryDefinitions.end())
	{
		delete (*index);

		++index;
	}
	
	m_FactoryDefinitions.clear();

}

void Factory::Announce(ISourceFactory* factory)
{
	std::list<ISourceFactory*>::iterator index = std::find(m_FactoryDefinitions.begin(), m_FactoryDefinitions.end(), factory);

	if (index == m_FactoryDefinitions.end())
	{
		m_FactoryDefinitions.push_back(factory);
	}
}

Generics::ProxyType<ISources> Factory::Create (const Generics::URL& identfier)
{
	Generics::ProxyType<ISources> result;
	Generics::String sourceIdentifier (identfier.Text().Text());

	m_Admin.Lock ();

	std::map<String, Generics::ProxyType<ISources>>::iterator	cached = m_AvailableSources.find(sourceIdentifier);

	// See if we have this identifier in cache..
	// Do we have a template for these type of identifications..
	if (cached != m_AvailableSources.end())
	{
		result = cached->second;
	}

	// If we did not have it in chache, see if we can create it..
	if (result.IsValid() == false)
	{
		// Check if this server supports this builder..
		std::list<ISourceFactory*>::iterator index = m_FactoryDefinitions.begin();

		while ( (result.IsValid() == false) && (index != m_FactoryDefinitions.end()) )
		{
			result = (*index)->Create(identfier);

			index++;
		}

		if (result.IsValid() == true)
		{
			// Store this for the next session..
			m_AvailableSources.insert(std::pair<String, Generics::ProxyType<ISources>>(sourceIdentifier, result));
		}
	}

	m_Admin.Unlock ();

	return (result);
}

void Factory::Timed()
{
	m_Admin.Lock ();

	std::map<String, Generics::ProxyType<ISources>>::iterator	cached = m_AvailableSources.begin();

	// Check if from all these resources, we hold the last..
	while  (cached != m_AvailableSources.end())
	{
		// See if we hold the last resource, if so, we release it..
		if (cached->second.LastRef())
		{
			cached->second.Release();

			//TODO: Take it out of the list...
		}

		cached++;
	}

	m_Admin.Unlock ();

	// Make sure we are retriggerd !!
	m_CleanupTask.Schedule(Generics::Time::Now().Ticks() + (PERIODIC_CLEANUP_TIME * 1000 * TICKS_PER_MILLISECONDS), this);
}

} } // namespace Solutions::Source

