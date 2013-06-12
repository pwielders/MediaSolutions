#include "SinkFactory.h"

namespace Solutions { namespace Sink
{

Factory::Iterator::Iterator() :
	m_Selected(NULL),
	m_Type(NULL),
	m_Container(NULL),
	m_Iterator()
{
}

Factory::Iterator::Iterator(const std::list<ISinkFactory*>& container, const Source::ISource* type) :
	m_Selected(NULL),
	m_Type(type),
	m_Container(&container),
	m_Iterator(container.begin())
{
}

Factory::Iterator::Iterator(const Factory::Iterator& copy) :
	m_Selected(copy.m_Selected),
	m_Type(copy.m_Type),
	m_Container(copy.m_Container),
	m_Iterator(copy.m_Iterator)
{
}

Factory::Iterator::~Iterator()
{
}

bool Factory::Iterator::IsValid() const
{
	return (m_Selected != NULL);
}

bool Factory::Iterator::Next ()
{
	if ( (m_Container != NULL) && (m_Iterator != m_Container->end()))
	{
		Generics::OptionalType<Generics::TextFragment> result;

		if (m_Selected != NULL)
		{
			m_Iterator++;
		}

		// Check if we are on a valid pos, if not move on..
		while ((m_Iterator != m_Container->end()) && ((*m_Iterator)->Supported(m_Type) == false))
		{
			m_Iterator++;
		}

		if (m_Iterator != m_Container->end())
		{
			m_Selected = (*m_Iterator);
		}
		else
		{
			m_Selected = NULL;
		}
	}

	return (m_Selected != NULL);
}

Factory::Factory() :
	m_FactoryDefinitions ()
{
}

Factory::~Factory()
{
	// Loose all registrations
	std::list<ISinkFactory*>::iterator index = m_FactoryDefinitions.begin();

	while (index != m_FactoryDefinitions.end())
	{
		delete (*index);

		++index;
	}
	
	m_FactoryDefinitions.clear();

}

void Factory::Announce(ISinkFactory* factory)
{
	std::list<ISinkFactory*>::iterator index = std::find(m_FactoryDefinitions.begin(), m_FactoryDefinitions.end(), factory);

	if (index == m_FactoryDefinitions.end())
	{
		m_FactoryDefinitions.push_back(factory);
	}
}

ISink* Factory::Create (const Sink::EnumSinkType type, const Source::ISource* source, Generics::NodeId& linkPoint)
{
	ISink* result = NULL;

	Factory::Iterator iterator (m_FactoryDefinitions,source);

	while ( (iterator.Next() == true)  && (result == NULL) )
	{
		result = iterator.Factory()->Create(type, source, linkPoint);
	}

	return (result);
}

Factory::Iterator Factory::Sinks(const Source::ISource* type) const
{
	return (Factory::Iterator(m_FactoryDefinitions,type));
}

} } // namespace Solutions::Sink

