#include "Source.h"
#include "AVIMediaSource.h"

namespace Solutions { namespace Source
{

AVIMedia::AVIMedia (const Generics::DataElement& dataObject) : 
	m_MainHeader(AVI::Main (dataObject))
{
}

/* virtual */ AVIMedia::~AVIMedia()
{
}


} } // namespace Solutions::Source

