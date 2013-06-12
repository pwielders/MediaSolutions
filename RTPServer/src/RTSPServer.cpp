#include "RTSPServer.h"

namespace Solutions 
{

RTSPServer::RTSPServer (const Generics::NodeId& node, const TCHAR* description, const uint32 characteristics) : 
	Generics::SocketServerType<RTSPHandler> (node), 
	m_Description(description), 
	m_Characteristics (characteristics),
	m_SourceTypes(),
	m_SinkTypes()
{
}

/* virtual */ RTSPServer::~RTSPServer ()
{
}

/* virtual */ bool RTSPServer::Autheticate ()
{
	return (true);
}

const TCHAR* RTSPServer::Description () const
{
	return (m_Description.c_str());
}

bool RTSPServer::AgentSetup () const
{
	return ((m_Characteristics & AGENT_SETUP_ALLOWED) == AGENT_SETUP_ALLOWED);
}

void RTSPServer::Register (Source::ISourceFactory* source)
{
	m_SourceTypes.Announce(source);
}

void RTSPServer::Register (Sink::ISinkFactory* sink)
{
	m_SinkTypes.Announce(sink);
}

Generics::ProxyType<Source::ISources> RTSPServer::Source (const Generics::TextFragment& identifier)
{
	Generics::ProxyType<Source::ISources> result;
	Generics::URL locator (identifier);

	// Check if the data is a valid URL, if not reject and move on!!!
	if (locator.IsValid())
	{
		// if it is a rtsp request from this host, it get's turned into a file. Create the URI accordingly
		if ( (locator.Type() == Generics::URL::SCHEME_RTSP) &&
			 ( (locator.Host().IsSet () == false) || 
			 (Generics::NodeId(locator.Host().Value()).IsLocal() == true)))
		{
			// Update the URL...
			Generics::URL newLocator(Generics::URL::SCHEME_FILE);

			newLocator.Path(locator.Path());

			locator = newLocator;
		}

		result = m_SourceTypes.Create(locator);
	}

	return (result);
}

Sink::Factory& RTSPServer::Factory ()
{
	return (m_SinkTypes);
}

} // namespace Solutions

