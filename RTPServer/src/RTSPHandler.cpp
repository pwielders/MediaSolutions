#include "RTSPHandler.h"
#include "RTSPServer.h"
#include "TraceCategories.h"

#define RTSPEOL							"\r\n"
namespace Solutions { 
	
//------------------------------------------------------------------------------------------------------------
// Definition section:
//
// Here are all the definitions of the seperate generic entities (not linked to RTSP/SDP) mapped to specific
// RTSP/SDP text settings.
//------------------------------------------------------------------------------------------------------------

const String ContentType				(_T("application/sdp"));
const String Status_OK					(_T("OK"));
const String Status_Unauthorized		(_T("Unauthorized"));
const String Status_ContentFailure		(_T("Content not found/reached or incorrect format"));
const String Status_TransportFailure	(_T("Transport type cvan not be realized"));

ENUM_CONVERSION_TABLE(Source::EnumStreamType) 
{
	{ Source::AUDIO, _TXT("audio") },
	{ Source::VIDEO, _TXT("video") }
};

ENUM_CONVERSION_TABLE(Source::EnumSourceType)
{
	{ Source::UNKNOWN,      _TXT("Unknown context")   },
	{ Source::MPEG_STREAM,  _TXT("MPEG1/2 describer") },
	{ Source::AVI_MEDIA,    _TXT("AVI describer")     },
};

typedef struct 
{
	Source::EnumSourceType			SourceType;
	Source::EnumStreamType			StreamType;
	Sink::EnumSinkType			SinkType;
	const TCHAR*					Definition;
	uint8							Length;

}	SDPDefinitionType;

static SDPDefinitionType g_SDPDefinitionTable[] = 
{
	{ Source::UNKNOWN,	    Source::TEXT,	 Sink::UNKNOWN,  _TXT("") },
	{ Source::MPEG_STREAM,	Source::AUDIO,	 Sink::RTP_UDP,  _TXT("RTP/AVP 14") },
	{ Source::MPEG_STREAM,	Source::VIDEO,	 Sink::RTP_TCP,  _TXT("RTP/AVP 33") },
	{ Source::AVI_MEDIA,    Source::AUDIO,   Sink::RTP_UDP,  _TXT("RTP/AVP 16") },
	{ Source::AVI_MEDIA,    Source::VIDEO,   Sink::RTP_UDP,  _TXT("RTP/AVP 16") },
	{ Source::AVI_MEDIA,    Source::AUDIO,   Sink::RTP_TCP,  _TXT("RTP/AVP 16") },
	{ Source::AVI_MEDIA,    Source::VIDEO,   Sink::RTP_TCP,  _TXT("RTP/AVP 16") },
};

//------------------------------------------------------------------------------------------------------------
// End definition section
//------------------------------------------------------------------------------------------------------------

static Generics::TextFragment GetSDPDefinition (const Source::EnumSourceType sourceType, Source::EnumStreamType streamType, Sink::EnumSinkType sinkType)
{
	Generics::TextFragment result;
	uint16 index = 0;

	while ((result.IsEmpty() == true) && (index < (sizeof(g_SDPDefinitionTable)/sizeof(SDPDefinitionType))))
	{
		if ( (g_SDPDefinitionTable[index].SinkType   == sinkType) &&
			 (g_SDPDefinitionTable[index].SourceType == sourceType) &&
			 (g_SDPDefinitionTable[index].StreamType == streamType) )
		{
			result = Generics::TextFragment(g_SDPDefinitionTable[index].Definition, g_SDPDefinitionTable[index].Length);
		}
		else
		{
			index++;
		}
	}

	return (result);
}



class RTSPProcessor : public Generics::SingletonType<RTSPProcessor>
{
	friend class Generics::SingletonType<RTSPProcessor>;

	private:
		class Work : public Generics::ThreadContextType<Generics::ProxyType<RTSPHandler>>
		{
			public:
				inline Work () : Generics::ThreadContextType<Generics::ProxyType<RTSPHandler>> (), m_Info()
				{
				}
				inline Work (const Generics::ProxyType<RTSPHandler>& session) : 
					Generics::ThreadContextType<Generics::ProxyType<RTSPHandler>> (session), m_Info()
				{
				}
				inline Work (const Generics::ProxyType<RTSPHandler>& session, Generics::ProxyType<RTSP::Message>& message) : 
					Generics::ThreadContextType<Generics::ProxyType<RTSPHandler>> (session), m_Info(message)
				{
				}
				inline Work(const Work& copy) : Generics::ThreadContextType<Generics::ProxyType<RTSPHandler>> (copy), m_Info(copy.m_Info)
				{
				}
				inline ~Work ()
				{
				}
				Work& operator= (Work& RHS)
				{
					Generics::ThreadContextType<Generics::ProxyType<RTSPHandler>>::operator= (RHS);

					m_Info = RHS.m_Info;

					return (*this);
				}
				inline void Process()
				{
					Generics::ThreadContextType<Generics::ProxyType<RTSPHandler>>::Context()->Process(*m_Info);
				}

			private:
				Generics::ProxyType<RTSP::Message>	m_Info;
		};


	protected:
		RTSPProcessor () :
			SingletonType<RTSPProcessor>(),
			m_Processor(_T("RTSPProcessor"))
		{
		}
		~RTSPProcessor()
		{
			// Stop handling any new requests..
			m_Processor.Block ();
			m_Processor.Wait (Generics::Thread::BLOCKED, INFINITE);
		}

	public:
		inline void ScheduleWork(const Generics::ProxyType<RTSPHandler>& entry, Generics::ProxyType<RTSP::Message> information)
		{
			Work newJob (entry, information);
			m_Processor.Submit(newJob, INFINITE);
		}
		inline void Revoke(const Generics::ProxyType<RTSPHandler>& entry)
		{
			Work newJob (entry);
			m_Processor.Revoke(newJob);
		}


	private:
		Generics::ThreadPoolType<Work, 1, ~0>		m_Processor;
};

RTSPHandler::RTSPHandler(Generics::SocketServerType<RTSPHandler>* parent, Generics::SocketStream* media) : 
	m_Parent(static_cast<RTSPServer&>(*parent)), 
	m_RTSPClient(media), 
	m_ReceiveText(), 
	m_Channel(media, media->BufferSize(true)), 
	m_LastIndex(0),
	m_RemoteNode(media->RemoteNode())
{
	// This is the producer/consumer pattern!!!
	// We received the IMedia interface for consumption.
	// We need to delete it if we are done with it !!!
	// so remember the media
	if (m_Channel.Receive(m_ReceiveText) == true)
	{
		// We received data right away, process it...
		HandleProtocol();
	}
}

RTSPHandler::~RTSPHandler()
{
	// This is the producer/consumer pattern!!!
	// We received the IMedia interface for consumption.
	// We need to delete it if we are done with it !!!
	delete m_RTSPClient;
	m_RTSPClient = NULL;
}

/* virtual */ bool RTSPHandler::Authenticate (const RTSP::Message& /* info */)
{
	return (true);
}

void RTSPHandler::HandleProtocol ()
{
	// See if we can send a response again, if not 
	// do not read the receive queue, This will throttle
	// incoming requests!!!
	if ( (m_Channel.Send() == true) && (m_Channel.Receive() == true))
	{
		// Seems like a line has been received...
		// Look for the end of the message: <CR><LF><CR><LF>
		if (m_ReceiveText.length() == m_LastIndex)
		{
			Generics::ProxyType<RTSP::Message> request (RTSP::Message::Create (Generics::TextFragment(m_ReceiveText)));

		if ((request.IsValid()) && (request->RequestType().IsSet() == true))
			{
				// Lets handle the RTSP requestSend it out !!! 
				RTSPProcessor::Instance().ScheduleWork(Generics::ProxyType<RTSPHandler> (*this), request);
			}
			m_ReceiveText.clear();
		}
		else
		{
			m_ReceiveText.push_back('\n');
		}
		m_LastIndex = m_ReceiveText.length();

		m_Channel.Receive(m_ReceiveText);
	}
}

void RTSPHandler::Process(const RTSP::Message& info)
{
	RTSP::Message response;
	TRACE(Trace::RTSPRequest, (info.Data()));

	switch (info.RequestType())
	{
	case RTSP::Message::OPTIONS:		Options(info, response);
										break;
	case RTSP::Message::DESCRIBE:		Describe(info, response);
										break;
	case RTSP::Message::SETUP:			Setup(info, response);
										break;
	case RTSP::Message::PLAY:			Play(info, response);
										break;
	case RTSP::Message::PAUSE:			Pause(info, response);
										break;
	case RTSP::Message::SET_PARAMETER:	SetParameter(info, response);
										break;
	case RTSP::Message::GET_PARAMETER:	GetParameter(info, response);
										break;
	case RTSP::Message::TEARDOWN:		Teardown(info, response);
										break;
	case RTSP::Message::ANNOUNCE:		Announce(info, response);
										break;
	case RTSP::Message::REDIRECT:		Redirect(info, response);
										break;
	case RTSP::Message::RECORD:			Record(info, response);
										break;
	}

	String message = response.Data();

	MSG_DROP ("\nRequest: %s\n", info.Data().data());
	MSG_DROP ("\nResponse: %s\n", message.data());

	TRACE(Trace::RTSPResponse, (message));
	m_Channel.Send(message);
}

void RTSPHandler::Options (const RTSP::Message& info, RTSP::Message& response)
{
	response.Sequence(info.Sequence());
	response.Date(Generics::Time::Now());
	response.Status(200);
	response.StatusDescription(Generics::TextFragment(Status_OK));

	RTSP::MessageOptions options;

	options.AllowedRequest(
		(1 << RTSP::Message::OPTIONS)       | 
		(1 << RTSP::Message::DESCRIBE)      |
		(1 << RTSP::Message::SETUP)         |
		(1 << RTSP::Message::TEARDOWN)      |
		(1 << RTSP::Message::PLAY)          |
		(1 << RTSP::Message::PAUSE)         |
		(1 << RTSP::Message::SET_PARAMETER) );

	String publicMessage = options.Message();

	response.Public(Generics::TextFragment(publicMessage));
}

void RTSPHandler::Describe (const RTSP::Message& info, RTSP::Message& response)
{
	String description;

	response.Sequence(info.Sequence());
	response.Date(Generics::Time::Now());

	if (Authenticate(info) == false)
	{
		response.Status(401);
		response.StatusDescription(Generics::TextFragment(Status_Unauthorized));
	}
	else if (info.URI().IsSet() == false)
	{  
		response.Status(404);
		response.StatusDescription(Generics::TextFragment(Status_ContentFailure));
	}
	else if (SDPDescription(info.URI().Value(), description) == false)
	{
		response.Status(404);
		response.StatusDescription(Generics::TextFragment(Status_ContentFailure));
	}
	else
	{
		String contentBase(info.URI().Value().Text() + '/');

		response.Status(200);
		response.StatusDescription(Generics::TextFragment(Status_OK));
		response.ContentBase(Generics::TextFragment(contentBase));
		response.ContentType(Generics::TextFragment(ContentType));
		response.Content(Generics::TextFragment(description));
	}
}

void RTSPHandler::Setup (const RTSP::Message& request, RTSP::Message& response)
{
	response.Sequence(request.Sequence());
	response.Date(Generics::Time::Now());

	if (request.URI().IsSet() && request.Transport().IsSet())
	{
		RTSP::MessageOptions options (request.Transport().Value());

		// Make sure the basic information is available !!!
		if (options.Protocol().IsSet() && options.ClientPorts().IsSet())
		{
			Generics::NodeId destinationAddress (m_RemoteNode);
					
			if (options.Destination().IsSet())
			{
				destinationAddress = Generics::NodeId (options.Destination(), 0); 
			}

			if ( (options.Destination().IsSet() == false) || (m_Parent.AgentSetup() == true) || (destinationAddress == m_RemoteNode) )
			{
				uint8 trackId = NUMBER_MAX_UNSIGNED(uint8);
				Generics::TextFragment resource (request.URI().Value());

				// We need to take off the track/control..
				uint32 index = resource.ReverseFind(_T("/\\"), 0);

				if (index != NUMBER_MAX_UNSIGNED(uint32))
				{
					resource = Generics::TextFragment (resource, 0, index);
					Generics::TextFragment trackText (Generics::TextFragment(resource, index, resource.Length() - index));

					// This also means that we have a track number to construct, determine the track number
					// It should contain Track"", so we start at position 5 with the number !!!
					trackText.Number<uint8, false>(trackId, 5);
				}

				// Create a source that is requested !!!
				Generics::ProxyType<Source::ISources>	mediaSource = m_Parent.Source(resource);
				Generics::URL resourceURL (resource);

				if ((mediaSource.IsValid()) && (trackId != NUMBER_MAX_UNSIGNED(uint8)))
				{
					Sessions::Session* session = NULL;

					if (request.Session().IsSet())
					{
						// Time to find a Session !!!
						session = Sessions::SessionStore::Instance().FindSessionBySessionId(request.Session().Value());
					}
					else
					{
						// Time to create a session...
						session = new Sessions::Session(mediaSource);
					}

					if (session != NULL)
					{
						Generics::TextFragment sessionId (request.Session().Value()); 

						options.Destination(Generics::OptionalType<Generics::TextFragment>(Generics::TextFragment(destinationAddress.HostAddress())));
						Generics::URL identifier(Generics::URL::SCHEME_RTP_UDP);
						RTSP::PortRange portsClient = options.ClientPorts().Value();
						RTSP::PortRange portsUsed;

						identifier.Port(portsClient.Minimum());
						identifier.Host(Generics::OptionalType<Generics::TextFragment>(Generics::TextFragment(destinationAddress.HostAddress())));

						uint16 portNumber;

						if ( (portNumber = AddChannel (*session, mediaSource, options, trackId, destinationAddress)) != 0)
						{
							response.Status(200);
							response.StatusDescription(Generics::TextFragment(Status_OK));
							options.Source(resourceURL.Host());
							options.ServerPorts(RTSP::PortRange(portNumber, portNumber+1));
							response.Transport(Generics::TextFragment(options.Message()));
							response.Session(Generics::TextFragment(session->SessionId()));
						}
					}
				}
			}
		}
	}

	if (response.Status().IsSet() == false)
	{
		// Create the faiure response, just in case we cannot reach the end.
		response.Status(401);
		response.StatusDescription(Generics::TextFragment(Status_ContentFailure));
	}
}

void RTSPHandler::Play(const RTSP::Message& info, RTSP::Message& response)
{
	response.Sequence(info.Sequence());
	response.Date(Generics::Time::Now());

	// See for which channel this is intended...
	Generics::OptionalType<Generics::TextFragment>	channelId (info.ChannelId());

	// Check if we can find a session..
	if (info.Session().IsSet())
	{
		Sessions::Session*	session = Sessions::SessionStore::Instance().FindSessionBySessionId(info.Session().Value());

		if (session != NULL)
		{
			uint64 startTick = NUMBER_MAX_UNSIGNED(uint64);

			if (info.Range().IsSet())
			{
				RTSP::MessageOptions options (info.Range().Value());
				
				if (options.NPTTime().IsSet())
				{
					startTick = options.NPTTime().Value().Minimum().Integer() * 1000 +
								options.NPTTime().Value().Minimum().Remainder();
				}
			}

			// Now create the options for all channels.
			String baseURI = info.BaseURI().Value().Text() +_T("/");

			// session->RTSPRespone(info, response);
			// FOR RTSP::PLAY
			// -------------------------------------------------------
			// Channels must be iterated to create a:
			// RTP-Info: url=rtsp://127.0.0.1/Flirt.mp3/track1;seq=28223;rtptime=2157515825
			Sessions::Channel::Iterator indexChannel (session->Channels());

			while (indexChannel.Next())
			{
				RTSP::MessageOptions options;
				options.URL(Generics::OptionalType<Generics::TextFragment>(Generics::TextFragment(baseURI + indexChannel.Identifier())));
				options.Sequence(Generics::OptionalType<uint32>(indexChannel.Sequence()));
				options.RTPTime(Generics::OptionalType<uint32>(indexChannel.SyncStamp()));
				response.RTPInfo(Generics::TextFragment(options.Message()));
			}

			// Start the session to play on the requested spot!!
			startTick = session->Play(channelId, startTick);

			// Create a new range that indicates where we are and how long it is..
			uint64 endTime = session->Duration();

			// Now create a range..
			RTSP::MessageOptions range;
			range.NPTTime (RTSP::TimeRange (Generics::Fractional(
										static_cast<sint32>(startTick/1000), 
										static_cast<uint32>(startTick%1000)), 
									  Generics::Fractional(
										static_cast<sint32>(endTime/1000), 
										static_cast<uint32>(endTime%1000))));
			response.Range(Generics::TextFragment(range.Message()));

			// report a succesfull play !!!
			response.Status(200);
			response.StatusDescription(Generics::TextFragment(Status_OK));
			response.Session(Generics::TextFragment(session->SessionId()));
		}
	}

	if (response.Status().IsSet() == false)
	{
		// Create the faiure response, just in case we cannot reach the end.
		response.Status(401);
		response.StatusDescription(Generics::TextFragment(Status_ContentFailure));
	}
}

void RTSPHandler::Pause(const RTSP::Message& info, RTSP::Message& response)
{
	response.Sequence(info.Sequence());
	response.Date(Generics::Time::Now());

	// See for which channel this is intended...
	Generics::OptionalType<Generics::TextFragment>	channelId (info.ChannelId());

	// Check if we can find a session..
	if (info.Session().IsSet())
	{
		Sessions::Session*	session = Sessions::SessionStore::Instance().FindSessionBySessionId(info.Session().Value());

		if (session != NULL)
		{
			// Pause the session !!
			session->Pause(channelId);

			// report a succesfull pause !!!
			response.Status(200);
			response.StatusDescription(Generics::TextFragment(Status_OK));
			response.Session(Generics::TextFragment(session->SessionId()));
		}
	}

	if (response.Status().IsSet() == false)
	{
		// Create the faiure response, just in case we cannot reach the end.
		response.Status(401);
		response.StatusDescription(Generics::TextFragment(Status_ContentFailure));
	}
}

void RTSPHandler::SetParameter(const RTSP::Message& /*info*/, RTSP::Message& /*response*/)
{
}

void RTSPHandler::GetParameter(const RTSP::Message& /*info*/, RTSP::Message& /*response*/)
{
}

void RTSPHandler::Teardown(const RTSP::Message& info, RTSP::Message& response)
{
	response.Sequence(info.Sequence());
	response.Date(Generics::Time::Now());

	// Check if we can find a session..
	if (info.Session().IsSet())
	{
		Sessions::Session*	session = Sessions::SessionStore::Instance().FindSessionBySessionId(info.Session().Value());

		if (session != NULL)
		{
			// Unregister this session. (It will remove any pending work and move to the init state)
			session->Deactivate();

			// Clean-out the session !!!
			delete session;

			// report a succesfull clear !!!
			response.Status(200);
			response.StatusDescription(Generics::TextFragment(Status_OK));
		}
	}

	if (response.Status().IsSet() == false)
	{
		// Create the faiure response, just in case we cannot reach the end.
		response.Status(401);
		response.StatusDescription(Generics::TextFragment(Status_ContentFailure));
	}

}

void RTSPHandler::Announce(const RTSP::Message& /*info*/, RTSP::Message& /*response*/)
{
}

void RTSPHandler::Redirect(const RTSP::Message& /*info*/, RTSP::Message& /*response*/)
{
}

void RTSPHandler::Record(const RTSP::Message& /*info*/, RTSP::Message& /*response*/)
{
}

bool RTSPHandler::SDPDescription(const Generics::TextFragment& identifier, String& result) 
{
	bool availableResource = false;

	// Create a source that is requested !!!
	Generics::ProxyType<Source::ISources>	mediaSource = m_Parent.Source(identifier);

	// Of we have a session, we can reply.
	if (mediaSource.IsValid())
	{
		String durationText;
		Generics::Time now (Generics::Time::Now());
		uint64 duration = mediaSource->Duration();
		String sourceDescription (mediaSource->Parser());
		String sourceIdentifier (mediaSource->Content().Value().Text().Text());

		if (duration != 0)
		{
			durationText = _T("\r\na=range:npt=0-") + 
					       Generics::NumberType<uint64, false>(duration / 1000).Text() + 
						   _T(".") + 
						   Generics::NumberType<uint64, false>(duration % 1000).Text();
		}

		result = _T("v=0\r\no=- ") + Generics::NumberType<uint64, false>(now.Ticks() / 10000).Text() + _T(" 1 IN IP4 ");
		result += Generics::NodeId::LocalId().HostAddress();
		result += _T("\r\ns=") + sourceDescription;
		result += _T("\r\ni=") + sourceIdentifier;
		result += _T("\r\nt=0 0");
		result += _T("\r\na=type:broadcast\r\na=control:*");
		result += durationText;
		result += _T("\r\na=x-qt-text-nam:") + sourceDescription;
		result += _T("\r\na=x-qt-text-inf:") + sourceIdentifier;
		result += _T("\r\na=tool:") + String(m_Parent.Description());
		result += _T("\r\nc=IN IP4 ") + m_Parent.ListeningNode().HostAddress();

		uint8 teller = 0;

		while (mediaSource->Next() == true)
		{
			Source::ISource* channel = mediaSource->Source();

			if (channel != NULL)
			{
				String streamType = String (Generics::EnumerateType<Source::EnumStreamType>::Name(channel->Properties()->StreamType()));
				String baseText  = _T("\r\nm=") + streamType + ' ' + Generics::NumberType<uint8, false>(teller).Text() + ' ';

				// It can not be that, depending on the source type, we need to find the sink description....
				// TODO: Findout what we should report here !!!
				Sink::Factory::Iterator index(m_Parent.Factory().Sinks(channel));

				while (index.Next())
				{
					Generics::TextFragment definition (GetSDPDefinition (channel->Properties()->SourceType(), channel->Properties()->StreamType(), index.Factory()->Type()));
						
					if (definition.IsEmpty() ==false)
					{
						// RTP/AVP 14
						result += baseText + definition.Data();
					}
				}

				result += _T("\r\nb=AS:") + Generics::NumberType<uint32,false>(channel->Properties()->BitRate()).Text() +
					_T("\r\na=control:") + Sessions::Channel::Name(teller);

				delete channel;
			}

			teller++;
		}

		availableResource = true;
	}

	return (availableResource);
}

uint16 RTSPHandler::AddChannel (Sessions::Session& session, Generics::ProxyType<Source::ISources>& sources, const RTSP::MessageOptions& identifier, const uint8 trackId, const Generics::NodeId& connectPoint)
{
	uint16 portNumber = 0;
	Sink::EnumSinkType sinkType = Sink::UNKNOWN;
	
	switch (identifier.Protocol())
	{
		case RTSP::MessageOptions::RTPAVP:		sinkType = Sink::RTP_UDP;
												break;
		default:								/* Do not change it, default is unknown, nothing happens !!!*/
												break;
	}

	if (sinkType != Sink::UNKNOWN)
	{
		bool succeeded = true;
		uint8 teller = trackId;

		// Start from the beginning..
		sources->Reset();

		// Find the correct Channel..
		while ( ((succeeded = sources->Next()) == true) && (teller != 0) )
		{
			// Move on to the next
			teller--;
		}

		Source::ISource* source = (succeeded == true ? sources->Source() : NULL);

		// If we have the correct Source
		if (source != NULL)
		{
			Generics::NodeId linkPoint (connectPoint);

			// Now get the requested sink
			Sink::ISink* sink = m_Parent.Factory().Create(sinkType, source, linkPoint);

			if (sink != NULL)
			{
				// Producer, consumer, the generated streamer by the package, must be deleted by the channel.
				session.AddChannel (source, sink);

				portNumber = linkPoint.PortNumber();
			}
			else
			{
				// Do not forget to delete the source, The channel wil not do it ;-)
				delete source;
			}
		}
	}

	return (portNumber);
}

} // namespace Solutions

