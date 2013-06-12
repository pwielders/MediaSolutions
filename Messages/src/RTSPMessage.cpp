#include "RTSPMessage.h"

namespace Solutions { 

ENUM_CONVERSION_TABLE(RTSP::Message::enumRequestType)
{
	{ RTSP::Message::OPTIONS,       _TXT("OPTIONS")       },
	{ RTSP::Message::DESCRIBE,      _TXT("DESCRIBE")      },
	{ RTSP::Message::ANNOUNCE,      _TXT("ANNOUNCE")      },
	{ RTSP::Message::SETUP,         _TXT("SETUP")         },
	{ RTSP::Message::PLAY,          _TXT("PLAY")          },
	{ RTSP::Message::PAUSE,         _TXT("PAUSE")         },
	{ RTSP::Message::TEARDOWN,      _TXT("TEARDOWN")      },
	{ RTSP::Message::GET_PARAMETER, _TXT("GET_PARAMETER") },
	{ RTSP::Message::SET_PARAMETER, _TXT("SET_PARAMETER") },
	{ RTSP::Message::REDIRECT,      _TXT("REDIRECT")      },
	{ RTSP::Message::RECORD,        _TXT("RECORD")        },
};

ENUM_CONVERSION_TABLE(RTSP::Message::enumHeaderType)
{
	{ RTSP::Message::CACHE_CONTROL,			_TXT("Cache-Control")		},		// g
	{ RTSP::Message::CONNECTION,			_TXT("Connection")			},		// g
	{ RTSP::Message::CSEQ,					_TXT("CSeq")				},		// g
	{ RTSP::Message::DATE,					_TXT("Date")				},		// g
	{ RTSP::Message::VIA,					_TXT("Via")					},		// g
	{ RTSP::Message::ACCEPT,				_TXT("Accept")				},		// R
	{ RTSP::Message::ACCEPT_ENCODING,		_TXT("Accept-Encoding")		},		// R
	{ RTSP::Message::ACCEPT_LANGUAGE,		_TXT("Accept-Language")		},		// R
	{ RTSP::Message::AUTHORIZATION,			_TXT("Authorization")		},		// R
	{ RTSP::Message::BANDWIDTH,				_TXT("Bandwidth")			},		// R
	{ RTSP::Message::BLOCKSIZE,				_TXT("Blocksize")			},		// R
	{ RTSP::Message::CONFERENCE,			_TXT("Conference")			},		// R
	{ RTSP::Message::FROM,					_TXT("From")				},		// R
	{ RTSP::Message::IF_MODIFIED_SINCE,		_TXT("If-Modified-Since")	},		// R
	{ RTSP::Message::PROXY_REQUIRE,			_TXT("Proxy-Require")		},		// R
	{ RTSP::Message::REFERER,				_TXT("Referer")				},		// R
	{ RTSP::Message::REQUIRE,				_TXT("Require")				},		// R
	{ RTSP::Message::USER_AGENT,			_TXT("User-Agent")			},		// R
	{ RTSP::Message::RANGE,					_TXT("Range")				},		// Rr
	{ RTSP::Message::SCALE,					_TXT("Scale")				},		// Rr
	{ RTSP::Message::SESSION,				_TXT("Session")				},		// Rr
	{ RTSP::Message::SPEED,					_TXT("Speed")				},		// Rr
	{ RTSP::Message::TRANSPORT,				_TXT("Transport")			},		// Rr
	{ RTSP::Message::ALLOW,					_TXT("Allow")				},		// r
	{ RTSP::Message::CONTENT_TYPE,			_TXT("Content-Type")		},		// re
	{ RTSP::Message::PUBLIC,				_TXT("Public")				},		// r
	{ RTSP::Message::RETRY_AFTER,			_TXT("Retry-After")			},		// r
	{ RTSP::Message::RTP_INFO,				_TXT("RTP-Info")			},		// r
	{ RTSP::Message::SERVER,				_TXT("Server")				},		// r
	{ RTSP::Message::UNSUPPORTED,			_TXT("Unsupported")			},		// r
	{ RTSP::Message::WWW_AUTHENTICATE,		_TXT("WWW-Authenticate")	},		// r
	{ RTSP::Message::CONTENT_BASE,			_TXT("Content-Base")		},		// e
	{ RTSP::Message::CONTENT_ENCODING,		_TXT("Content-Encoding")	},		// e
	{ RTSP::Message::CONTENT_LANGUAGE,		_TXT("Content-Language")	},		// e
	{ RTSP::Message::CONTENT_LENGTH,		_TXT("Content-Length")		},		// e
	{ RTSP::Message::CONTENT_LOCATION,		_TXT("Content-Location")	},		// e
	{ RTSP::Message::EXPIRES,				_TXT("Expires")				},		// e
	{ RTSP::Message::LAST_MODIFIED,			_TXT("Last-Modified")		},		// e
	{ RTSP::Message::PROXY_AUTHENTICATION,	_TXT("Proxy-Authenticate")	}		// e
};

ENUM_CONVERSION_TABLE(RTSP::MessageOptions::enumKeyWords)
{
	{ RTSP::MessageOptions::OPTIONS,		 _TXT("OPTIONS")       },
	{ RTSP::MessageOptions::DESCRIBE,		 _TXT("DESCRIBE")      },
	{ RTSP::MessageOptions::ANNOUNCE,		 _TXT("ANNOUNCE")      },
	{ RTSP::MessageOptions::SETUP,			 _TXT("SETUP")         },
	{ RTSP::MessageOptions::PLAY,			 _TXT("PLAY")          },
	{ RTSP::MessageOptions::PAUSE,			 _TXT("PAUSE")         },
	{ RTSP::MessageOptions::TEARDOWN,		 _TXT("TEARDOWN")      },
	{ RTSP::MessageOptions::GET_PARAMETER,	 _TXT("GET_PARAMETER") },
	{ RTSP::MessageOptions::SET_PARAMETER,	 _TXT("SET_PARAMETER") },
	{ RTSP::MessageOptions::REDIRECT,		 _TXT("REDIRECT")      },
	{ RTSP::MessageOptions::RECORD,			 _TXT("RECORD")        },
	{ RTSP::MessageOptions::RTPAVP_TYPE,	 _TXT("RTP/AVP")       },
	{ RTSP::MessageOptions::DESTINATION,	 _TXT("destination")   },
	{ RTSP::MessageOptions::SOURCE,			 _TXT("source")        },
	{ RTSP::MessageOptions::CLIENTPORT,		 _TXT("client_port")   },
	{ RTSP::MessageOptions::SERVERPORT,		 _TXT("server_port")   },
	{ RTSP::MessageOptions::UNICAST,		 _TXT("unicast")       },
	{ RTSP::MessageOptions::MULTICAST,		 _TXT("multicast")     },
	{ RTSP::MessageOptions::URLTEXT,		 _TXT("url")           },
	{ RTSP::MessageOptions::SEQUENCE,		 _TXT("seq")           },
	{ RTSP::MessageOptions::RTPTIME,		 _TXT("rtptime")       },
	{ RTSP::MessageOptions::NPT,		     _TXT("npt")           }
};

namespace RTSP
{

const String ProtocolVersion			(_T("RTSP/1.0 "));
const String RTPS_EOL					(_T("\r\n"));
#define DateHeader						(_T("%a, %b %d %Y %H:%M:%S GMT"))


/* static */ Generics::ProxyType<Message> Message::Create (const Generics::TextFragment& message)
{
	uint32 length = message.Length();
	Generics::ProxyType<Message>	request (Generics::ProxyType<Message>::CreateEx((length + 1) * sizeof(TCHAR)));

	TCHAR* buffer = reinterpret_cast<TCHAR*> (request.Buffer());

	// Copy the memory into the buffer
	::memcpy (buffer, message.Data(), (length * sizeof(TCHAR)));

	buffer[length] = '\0';

	request->Parse (Generics::TextFragment(buffer, length));

	return (request);
}


Message::Message () :
	m_Sequence(),
	m_Request(),
	m_Major(),
	m_Minor(),
	m_Status(),
	m_URI(),
	m_UserAgent()
{
}

Message::Message (const TCHAR lines[]) :
	m_Sequence(),
	m_Request(),
	m_Major(),
	m_Minor(),
	m_Status(),
	m_URI(),
	m_UserAgent()
{
	Parse (Generics::TextFragment(lines));
}

Message::~Message()
{
}

String Message::Data() const
{
	String message;

	if (m_Request.IsSet())
	{
		// Seems like we are processing a Request !!
		message = Generics::EnumerateType<enumRequestType>::Name(m_Request.Value());

		message += RTPS_EOL;
	}
	else if (m_Status.IsSet ())
	{
		// Seems like we are processing a response !!
		message = ProtocolVersion + Generics::NumberType<uint16, false>(m_Status.Value()).Text();

		if (m_StatusDescription.IsSet())
		{
			message.push_back (' ');
			message += m_StatusDescription.Value().Text();
		}

		message += RTPS_EOL;
	}

	if (m_Sequence.IsSet())
	{
		message += Generics::EnumerateType<enumHeaderType>::Name(CSEQ);
		message.push_back (':'); message.push_back (' ');
		message += Generics::NumberType<uint32,false>(m_Sequence.Value()).Text();
		message += RTPS_EOL;
	}

	if (m_DateTime.IsSet())
	{
		message += Generics::EnumerateType<enumHeaderType>::Name(DATE);
		message.push_back (':'); message.push_back (' ');
		message += m_DateTime.Value().Format(DateHeader);
		message += RTPS_EOL;
	}

	if (m_Transport.IsSet())
	{
		message += Generics::EnumerateType<enumHeaderType>::Name(TRANSPORT);
		message.push_back (':'); message.push_back (' ');
		message += m_Transport.Value().Text();
		message += RTPS_EOL;
	}

	if (m_Public.IsSet())
	{
		message += Generics::EnumerateType<enumHeaderType>::Name(PUBLIC);
		message.push_back (':'); message.push_back (' ');
		message += m_Public.Value().Text();
		message += RTPS_EOL;
	}

	if (m_Session.IsSet())
	{
		message += Generics::EnumerateType<enumHeaderType>::Name(SESSION);
		message.push_back (':'); message.push_back (' ');
		message += m_Session.Value().Text();
		message += RTPS_EOL;
	}

	if (m_Range.IsSet())
	{
		message += Generics::EnumerateType<enumHeaderType>::Name(RANGE);
		message.push_back (':'); message.push_back (' ');
		message += m_Range.Value().Text();
		message += RTPS_EOL;
	}

	if (m_RTPInfo.IsSet())
	{
		message += Generics::EnumerateType<enumHeaderType>::Name(RTP_INFO);
		message.push_back (':'); message.push_back (' ');
		message += m_RTPInfo.Value().Text();
		message += RTPS_EOL;
	}

	if (m_ContentBase.IsSet())
	{
		message += Generics::EnumerateType<enumHeaderType>::Name(CONTENT_BASE);
		message.push_back (':'); message.push_back (' ');
		message += m_ContentBase.Value().Text();
		message += RTPS_EOL;
	}

	if (m_ContentType.IsSet())
	{
		message += Generics::EnumerateType<enumHeaderType>::Name(CONTENT_TYPE);
		message.push_back (':'); message.push_back (' ');
		message += m_ContentType.Value().Text();
		message += RTPS_EOL;
	}

	if (m_Content.IsSet())
	{
		message += Generics::EnumerateType<enumHeaderType>::Name(CONTENT_LENGTH);
		message.push_back (':'); message.push_back (' ');
		message += Generics::NumberType<uint32, false>(m_Content.Value().Length()).Text();
		message += RTPS_EOL;
		message += RTPS_EOL;
		message += m_Content.Value().Text();
	}

	return (message);
}

void Message::Parse (const Generics::TextFragment& lines)
{
	Generics::TextParser parser(lines);

	// Find the first word, that must be a request..
	parser.ReadEnumerate<enumRequestType,true> (m_Request, _T("\n\r\t "));

	if (m_Request.IsSet())
	{
		// Do request parsing, this is a request !!
		// Request-Line = Method SP Request-URI SP RTSP-Version CRLF
		parser.ReadText(m_URI, _T("\n\r\t "));
	}

	if ( (parser.Validate<true>(Generics::TextFragment(_TXT("RTSP"))) == true) && (parser.OnMarker(_T("/"))) )
	{
		parser.Skip(1);

		// the next part is the protocol version
		parser.ReadNumber<uint8, false>(m_Major);

		if (parser.OnMarker(_T(".")))
		{
			parser.Skip(1);
			parser.ReadNumber<uint8, false>(m_Minor);
		}
	}

	if (m_Request.IsSet() == false)
	{
		// Now we should parse the status
		parser.ReadNumber<uint16, false>(m_Status);
	}

	// Now Skip past the CR/LF
	parser.SkipLine(); 

	while (parser.Length() != 0)
	{
		Generics::OptionalType<enumHeaderType> headerKeyWord;
		parser.ReadEnumerate<enumHeaderType, true> (headerKeyWord, _T("\n\r\t :"));

		if (headerKeyWord.IsSet())
		{
			// If we are on the colon, skip it.
			parser.Skip(_T(":"));
	
			switch (headerKeyWord.Value())
			{
				case CACHE_CONTROL:			// g
					break;
				case CONNECTION:			// g
					break;
				case CSEQ:					// g
					parser.ReadNumber<uint32, false> (m_Sequence);
					break;
				case DATE:					// g
					break;
				case VIA:					// g
					break;
				case ACCEPT:				// R
					parser.ReadText(m_Accept, _T("\n\r"));
					break;
				case ACCEPT_ENCODING:		// R
					break;
				case ACCEPT_LANGUAGE:		// R
					break;
				case AUTHORIZATION:			// R
					break;
				case BANDWIDTH:				// R
					break;
				case BLOCKSIZE:				// R
					break;
				case CONFERENCE:			// R
					break;
				case FROM:					// R
					break;
				case IF_MODIFIED_SINCE:		// R
					break;
				case PROXY_REQUIRE:			// R
					break;
				case REFERER:				// R
					break;
				case REQUIRE:				// R
					break;
				case USER_AGENT:			// R
					parser.ReadText(m_UserAgent, _T("\n\r"));
					break;
				case RANGE:					// Rr
					parser.ReadText(m_Range, _T("\n\r"));
					break;
				case SCALE:					// Rr
					break;
				case SESSION:				// Rr
					parser.ReadText(m_Session, _T("\n\r"));
					break;
				case SPEED:					// Rr
					break;
				case TRANSPORT:				// Rr
					parser.ReadText(m_Transport, _T("\n\r"));
					break;
				case ALLOW:					// r
					break;
				case CONTENT_TYPE:			// re
					break;
				case PUBLIC:				// r
					parser.ReadText(m_Public, _T("\n\r"));
					break;
				case RETRY_AFTER:			// r
					break;
				case RTP_INFO:				// r
					parser.ReadText(m_RTPInfo, _T("\n\r"));
					break;
				case SERVER:				// r
					break;
				case UNSUPPORTED:			// r
					break;
				case WWW_AUTHENTICATE:		// r
					break;
				case CONTENT_BASE:			// e
					break;
				case CONTENT_ENCODING:		// e
					break;
				case CONTENT_LANGUAGE:		// e
					break;
				case CONTENT_LENGTH:		// e
					break;
				case CONTENT_LOCATION:		// e
					break;
				case EXPIRES:				// e
					break;
				case LAST_MODIFIED:			// e
					break;
				case PROXY_AUTHENTICATION:	// e
					break;
			}
		}

		// Now Skip past the CR/LF
		parser.SkipLine(); 
	}
}

Generics::OptionalType<Generics::TextFragment> Message::ChannelId () const
{
	Generics::OptionalType<Generics::TextFragment> result;

	if (m_URI.IsSet())
	{
		Generics::TextParser parser(m_URI.Value());

		// Find the last '/ or \ from the back, after the drive
		uint32 index = parser.ReverseFind(_T("\\/"));

		if (index != NUMBER_MAX_UNSIGNED(uint32))
		{
			// Yip this seems to be a ChannelId
			result = Generics::TextFragment(parser, index + 1, NUMBER_MAX_UNSIGNED(uint32));
		}
	}

	return (result);
}

Generics::OptionalType<Generics::TextFragment> Message::BaseURI () const
{
	Generics::OptionalType<Generics::TextFragment> result;
	Generics::OptionalType<Generics::TextFragment> channelId (Message::ChannelId());

	if (channelId.IsSet())
	{
		result = Generics::TextFragment(m_URI.Value(), 0, m_URI.Value().Length() - channelId.Value().Length() - 1);
	}
	else
	{
		result = m_URI.Value();
	}

	return (result);
}

MessageOptions::MessageOptions()
{
}

MessageOptions::MessageOptions(const Generics::TextFragment& text)
{
	ParseOptionLine(text);
}

MessageOptions::~MessageOptions()
{
}

String MessageOptions::Message() const
{
	String message;

	if (m_ProtocolType.IsSet())
	{
		switch (m_ProtocolType.Value())
		{
		case MessageOptions::RTPAVP:	message = Generics::EnumerateType<enumKeyWords>::Name(RTPAVP_TYPE);
											break;
		}
	}

	if (m_Unicast.IsSet())
	{
		if (message.empty() == false) { message.push_back(';'); }

		if (m_Unicast.Value() == true)
		{
			message += Generics::EnumerateType<enumKeyWords>::Name(UNICAST);
		}
		else
		{
			message += Generics::EnumerateType<enumKeyWords>::Name(MULTICAST);
		}
	}

	if (m_Destination.IsSet())
	{
		if (message.empty() == false) { message.push_back(';'); }

		message += Generics::EnumerateType<enumKeyWords>::Name(DESTINATION);

		message.push_back('=');

		message += m_Destination.Value().Text();
	}

	if (m_Source.IsSet())
	{
		if (message.empty() == false) { message.push_back(';'); }

		message += Generics::EnumerateType<enumKeyWords>::Name(SOURCE);

		message.push_back('=');

		message += m_Source.Value().Text();
	}

	if (m_ClientPortRange.IsSet())
	{
		if (message.empty() == false) { message.push_back(';'); }

		message += Generics::EnumerateType<enumKeyWords>::Name(CLIENTPORT);

		message.push_back('=');

		message += Generics::NumberType<uint16, false>(m_ClientPortRange.Value().Minimum()).Text();

		message.push_back('-');

		message += Generics::NumberType<uint16, false>(m_ClientPortRange.Value().Maximum()).Text();
	}

	if (m_ServerPortRange.IsSet())
	{
		if (message.empty() == false) { message.push_back(';'); }

		message += Generics::EnumerateType<enumKeyWords>::Name(SERVERPORT);

		message.push_back('=');

		message += Generics::NumberType<uint16, false>(m_ServerPortRange.Value().Minimum()).Text();

		message.push_back('-');

		message += Generics::NumberType<uint16, false>(m_ServerPortRange.Value().Maximum()).Text();
	}

	if (m_RequestsAllowed.IsSet())
	{
		if (message.empty() == false) { message.push_back(';'); }

		uint32 value = m_RequestsAllowed.Value();
		uint32 runCount = 0;
		bool commaNeeded = false;

		while (value != 0)
		{
			if ((value & 0x01) == 0x01)
			{
				if (commaNeeded) { message.push_back(','); }

				message += Generics::EnumerateType<enumKeyWords>(OPTIONS + runCount);

				commaNeeded = true;
			}

			value = (value >> 1);
			runCount++;
		}
	}

	if (m_URL.IsSet())
	{
		if (message.empty() == false) { message.push_back(';'); }

		message += Generics::EnumerateType<enumKeyWords>::Name(URLTEXT);

		message.push_back('=');

		message += m_URL.Value().Text();
	}

	if (m_Sequence.IsSet())
	{
		if (message.empty() == false) { message.push_back(';'); }

		message += Generics::EnumerateType<enumKeyWords>::Name(SEQUENCE);

		message.push_back('=');

		message += Generics::NumberType<uint32,false>(m_Sequence.Value()).Text();
	}

	if (m_RTPTime.IsSet())
	{
		if (message.empty() == false) { message.push_back(';'); }

		message += Generics::EnumerateType<enumKeyWords>::Name(RTPTIME);

		message.push_back('=');

		message += Generics::NumberType<uint32,false>(m_RTPTime.Value()).Text();
	}

	if (m_NPTRange.IsSet())
	{
		if (message.empty() == false) { message.push_back(';'); }

		message += Generics::EnumerateType<enumKeyWords>::Name(NPT);

		message.push_back('=');

		message += m_NPTRange.Value().Minimum().Text(3);
		message.push_back('-');
		message += m_NPTRange.Value().Maximum().Text(3);
	}

	return (message);
}

void MessageOptions::ParseOptionLine (const Generics::TextFragment& text)
{
	Generics::TextParser	parser(text);

	while (parser.Length() != 0)
	{	
		Generics::OptionalType<enumKeyWords> keyWord;

		// Skip any pending white space and any delimters left, its an empty line
		// if we run into delimiters.....
		parser.SkipWhiteSpace(); parser.Skip(_T(";,="));

		// Read the keyword, this is the key
		parser.ReadEnumerate<enumKeyWords,true> (keyWord, _T(";,=\n\r\0"));

		if (parser.OnMarker(_T("=")) == true)
		{
			parser.Skip(1);
		}

		if (keyWord.IsSet())
		{
			switch (keyWord.Value())
			{
			case MessageOptions::OPTIONS:
				{
					uint32 value = (m_RequestsAllowed.IsSet() ? m_RequestsAllowed.Value() : 0);
					m_RequestsAllowed = value | (1 << Message::OPTIONS);
					break;
				}
			case MessageOptions::DESCRIBE:
				{
					uint32 value = (m_RequestsAllowed.IsSet() ? m_RequestsAllowed.Value() : 0);
					m_RequestsAllowed = value | (1 << Message::OPTIONS);
					break;
				}
			case MessageOptions::ANNOUNCE:
				{
					uint32 value = (m_RequestsAllowed.IsSet() ? m_RequestsAllowed.Value() : 0);
					m_RequestsAllowed = value | (1 << Message::OPTIONS);
					break;
				}
			case MessageOptions::SETUP:
				{
					uint32 value = (m_RequestsAllowed.IsSet() ? m_RequestsAllowed.Value() : 0);
					m_RequestsAllowed = value | (1 << Message::OPTIONS);
					break;
				}
			case MessageOptions::PLAY:
				{
					uint32 value = (m_RequestsAllowed.IsSet() ? m_RequestsAllowed.Value() : 0);
					m_RequestsAllowed = value | (1 << Message::PLAY);
					break;
				}
			case MessageOptions::PAUSE:
				{
					uint32 value = (m_RequestsAllowed.IsSet() ? m_RequestsAllowed.Value() : 0);
					m_RequestsAllowed = value | (1 << Message::PAUSE);
					break;
				}
			case MessageOptions::TEARDOWN:
				{
					uint32 value = (m_RequestsAllowed.IsSet() ? m_RequestsAllowed.Value() : 0);
					m_RequestsAllowed = value | (1 << Message::TEARDOWN);
					break;
				}
			case MessageOptions::GET_PARAMETER:
				{
					uint32 value = (m_RequestsAllowed.IsSet() ? m_RequestsAllowed.Value() : 0);
					m_RequestsAllowed = value | (1 << Message::GET_PARAMETER);
					break;
				}
			case MessageOptions::SET_PARAMETER:
				{
					uint32 value = (m_RequestsAllowed.IsSet() ? m_RequestsAllowed.Value() : 0);
					m_RequestsAllowed = value | (1 << Message::SET_PARAMETER);
					break;
				}
			case MessageOptions::REDIRECT:
				{
					uint32 value = (m_RequestsAllowed.IsSet() ? m_RequestsAllowed.Value() : 0);
					m_RequestsAllowed = value | (1 << Message::REDIRECT);
					break;
				}
			case MessageOptions::RECORD:
				{
					uint32 value = (m_RequestsAllowed.IsSet() ? m_RequestsAllowed.Value() : 0);
					m_RequestsAllowed = value | (1 << Message::RECORD);
					break;
				}
			case MessageOptions::CLIENTPORT:
				{
					parser.ReadRange<'-',uint16,false,true,false>(m_ClientPortRange);
					break;
				}
			case MessageOptions::DESTINATION:
				{
					parser.ReadText(m_Destination, _T(";,\n\r\0"));
					break;
				}
			case MessageOptions::MULTICAST:
				{
					m_Unicast = false;
					break;
				}
			case MessageOptions::UNICAST:
				{
					m_Unicast = true;
					break;
				}
			case MessageOptions::RTPAVP_TYPE:
				{
					m_ProtocolType = MessageOptions::RTPAVP;
					break;
				}
			case MessageOptions::SERVERPORT:
				{
					parser.ReadRange<'-',uint16,false,true,false>(m_ServerPortRange);
					break;
				}
			case MessageOptions::SOURCE:
				{
					parser.ReadText(m_Source, _T(";,\n\r\0"));
					break;
				}
			case MessageOptions::URLTEXT:
				{
					parser.ReadText(m_URL, _T(";,\n\r\0"));
					break;
				}
			case MessageOptions::SEQUENCE:
				{
					parser.ReadNumber<uint32,false>(m_Sequence);
					break;
				}
			case MessageOptions::RTPTIME:
				{
					parser.ReadNumber<uint32,false>(m_RTPTime);
					break;
				}
			case MessageOptions::NPT:
				{
					parser.ReadRange<'-',Generics::Fractional,true,true,false>(m_NPTRange);
					break;
				}
			}

			parser.Find(_T(";,\n\r\0"));

			// Clear the current Keyword, it is handled...
			keyWord = Generics::OptionalType<enumKeyWords>();
		}
	}
}

} } // namespace Solutions::RTSP