#include "TraceUnit.h"
#include "TraceCategories.h"

namespace Solutions { namespace Trace
{

/* static */ Generics::CriticalSection	TraceUnit::m_Admin;
/* static */ TraceMedia*				TraceUnit::m_OutputChannel = NULL;

void TraceUnit::Announce (ITraceControl& Category)
{
	m_Admin.Lock();

	m_Categories.push_back(&Category);

	m_Admin.Unlock();
}

void TraceUnit::Output (const Generics::NodeId& host)
{
	m_Admin.Lock();

	m_OutputChannel = new TraceMedia (host);

	m_Admin.Unlock();
}

TraceUnit::Iterator TraceUnit::GetCategories()
{
	return (Iterator(m_Categories));
}

uint32 TraceUnit::SetCategories (const bool enable, const char* module, const char* category)
{
	uint32 modifications = 0;

	TraceControlList::iterator index (m_Categories.begin());

	while (index != m_Categories.end())
	{
		const char* thisModule =  (*index)->GetModuleName();
		const char* thisCategory = (*index)->GetCategoryName();

		if ( ((*module == '\0')   || (::strcmp(module,   thisModule)   == 0)) &&
             ((*category == '\0') || (::strcmp(category, thisCategory) == 0)) )
		{

			modifications++;
			(*index)->Enabled(enable);
		}
		index++;
	}

	return (modifications);
}

/* static */ void
	TraceUnit::Trace(
		const char			a_FileName[],
		const unsigned int	a_LineNumber,
		const char			a_ClassName[],
		const ITrace* const a_Information)
{
	m_Admin.Lock();

	if (m_OutputChannel != NULL)
	{
		m_OutputChannel->SendTraceLine(a_FileName, a_LineNumber, a_ClassName, a_Information);
	}

	m_Admin.Unlock();
}

} } // namespace Solutions::Trace

