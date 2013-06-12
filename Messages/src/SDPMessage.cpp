
#include "SDPMessage.h"

namespace Solutions { namespace SDP
{

static Message::Attributes::AttributeName    g_AttributeNameTable[] =
{
	{ Message::Attributes::CONTROL,          _TXT("control:")       },
	{ Message::Attributes::RANGE,            _TXT("range:")         },
	{ Message::Attributes::TYPE,             _TXT("type:")          },
	{ Message::Attributes::SOURCE_FILTER,    _TXT("source-filter:") },
	{ Message::Attributes::RTPMAP,           _TXT("rtpmap:")        },
	{ Message::Attributes::FMTP,             _TXT("fmtp:")          },
	{ Message::Attributes::XDIMENSIONS,      _TXT("x-dimensions:")  },
	{ Message::Attributes::FRAMERATE,        _TXT("framerate:")     },
	{ Message::Attributes::XFRAMERATE,       _TXT("x-framerate:")   }
};

static Message::Attributes::AttributeType    g_AttributeTypeTable[] =
{
	{ Message::Attributes::BROADCAST,        _TXT("broadcast")      },
	{ Message::Attributes::MEETING,          _TXT("meeting")        },
	{ Message::Attributes::MODERATED,        _TXT("moderated")      },
	{ Message::Attributes::TEST,             _TXT("test")           },
	{ Message::Attributes::H332,             _TXT("H.332")          },
	{ Message::Attributes::RECVONLY,         _TXT("recvonly")       }
};

Message::Message (const TCHAR lines[]) :
	m_Information(lines, _tcslen(lines)),
	m_Parent(NULL),
	m_SubSessions(NULL),
	m_Attributes(),
	m_MediumName(),
	m_EndPoint(),
	m_SessionName(),
	m_SessionDescription(),
	m_Bandwidth(),
	m_ProtocolType(),
	m_PayLoadFormat(),
	m_ClientPort(),
	m_NumberOfPorts()
{
}

Message::Message (const String& text) :
	m_Information(text),
	m_Parent(NULL),
	m_SubSessions(NULL),
	m_Attributes(),
	m_MediumName(),
	m_EndPoint(),
	m_SessionName(),
	m_SessionDescription(),
	m_Bandwidth(),
	m_ProtocolType(),
	m_PayLoadFormat(),
	m_ClientPort(),
	m_NumberOfPorts()
{
}

void Message::Parse ()
{
	Generics::OptionalType<Generics::TextFragment> currentLine;

	Generics::TextParser blockReader (Generics::TextFragment(m_Information, 0, m_Information.length()));

	// Begin by processing all SDP line
	blockReader.ReadText(currentLine, _T("/n/r/0"));

	Message* current = this;

	while (currentLine.IsSet())
	{
		Generics::TextParser line (currentLine.Value());
		TCHAR command = *line.Data();
		line.Skip(1);

		//##### We should really check for:
		// - "a=control:" attributes (to set the URL for aggregate control)
		// - the correct SDP version (v=0)
		switch (command)
		{
		case 's':	current->Parse_S_Line (line);
					break;
		case 'i':	current->Parse_I_Line (line);
					break;
		case 'C':	current->Parse_C_Line (line);
					break;
		case 'm':	current = CreateSubSession ();
					current->Parse_M_Line (line);
					break;
		case 'b':	current->Parse_B_Line(line);
					break;
		case 'a':	current->Parse_A_Line (line);
					break;
		default:	break;
		}

		currentLine = Generics::TextFragment();

		blockReader.SkipLine();
		blockReader.ReadText(currentLine, _T("/n/r/0"));
	}
}

Message::Message	(Message& parent) : 
	m_Information	(NULL),
	m_Parent		(&parent),
	m_SubSessions	(NULL),
	m_Attributes	()
{
}

Message::~Message ()
{
	if (m_Parent != NULL)
	{
		m_Parent->Detach (this);
	}
	else
	{
		volatile Message* pointer = m_SubSessions;

		while (pointer != NULL)
		{
			delete pointer;

			// Reload the new subsession..
			pointer = m_SubSessions;
		}
	}
}

void Message::Attach(Message* subsession)
{
	// Subsessions can only be attched to parents
	ASSERT (m_Parent == NULL);

	if (m_SubSessions == NULL)
	{
		m_SubSessions = subsession;
	}
	else
	{
		Message* runner =  m_SubSessions;

		while (runner->m_SubSessions != NULL)
		{
			runner = runner->m_SubSessions;
		}

		runner->m_SubSessions = subsession;
	}
}

void Message::Detach(Message* subsession)
{
	// Subsessions cn only be detached from parents
	ASSERT ((m_Parent == NULL) && (m_SubSessions == NULL));

	if (m_SubSessions == subsession)
	{
		m_SubSessions = subsession->m_SubSessions;
	}
	else
	{
		Message* runner =  m_SubSessions;

		while ((runner != NULL) && (runner->m_SubSessions != subsession))
		{
			runner = runner->m_SubSessions;
		}

		ASSERT (runner != NULL);

		runner->m_SubSessions = subsession->m_SubSessions;
	}
}

Message* Message::CreateSubSession() 
{

	Message* master = (m_Parent != NULL ? m_Parent : this);

	// We have a "m=" line, representing a new sub-session:
    Message* subsession = new Message(*master);

	// Insert this subsession in the double linked list:
    master->Attach (subsession) ;

	// Coming lines might be applicable to this session !!
	return (subsession);
}

void Message::Parse_M_Line(const Generics::TextFragment& lines) 
{
	// Parse the line as "m=<medium_name> <client_portNum> RTP/AVP <fmt>"
	// or "m=<medium_name> <client_portNum>/<num_ports> RTP/AVP <fmt>"
	// (Should we be checking for >1 payload format number here?)#####
	Generics::TextParser parser(lines);

	parser.ReadText(m_MediumName, _T(" \t\r\n/"));

	if (parser.OnMarker(_T(" \t")))
	{
		parser.ReadNumber<uint16, false>(m_ClientPort);

		if (parser.OnMarker(_T("/")))
		{
			parser.Skip(1);
			parser.ReadNumber<uint8, false> (m_NumberOfPorts);
		}
	}

	// Now determine the kind of protocol:
	Generics::OptionalType<Generics::TextFragment> nextWord;
	parser.ReadText (nextWord, _T("\n\r\t /"));

	// See what word we extracted..
	if (nextWord.IsSet())
	{
		if ( (nextWord.Value() == Generics::TextFragment(_TXT("RTP"))) && (parser.OnMarker(_T("/"))) )
		{
			parser.Skip(1);
			parser.SkipWhiteSpace();

			if (parser.Validate<false>(Generics::TextFragment(_TXT("AVP"))))
			{
				Generics::OptionalType<uint32> payLoadFormat;

				// Get the payload format
				parser.ReadNumber<uint32, false> (payLoadFormat);

				if ( (payLoadFormat.IsSet()) && (payLoadFormat.Value() <= 127) )
				{
					m_ProtocolType = RTP;
					m_PayLoadFormat = static_cast <uint8>(payLoadFormat.Value());
				}
			}
		}
		else if (nextWord.Value() == Generics::TextFragment (_TXT("UDP")))
		{
			Generics::OptionalType<uint32> payLoadFormat;

			// Get the payload format
			parser.ReadNumber<uint32, false> (payLoadFormat);

			if ( (payLoadFormat.IsSet()) && (payLoadFormat.Value() <= 127) )
			{
				m_ProtocolType = UDP;
				m_PayLoadFormat = static_cast <uint8>(payLoadFormat.Value());
			}
		}
		else if ( (nextWord.Value() == Generics::TextFragment(_TXT("RAW"))) && (parser.OnMarker(_T("/"))) )
		{
			parser.Skip(1);
			parser.SkipWhiteSpace();

			if (parser.Validate<false>(Generics::TextFragment(_TXT("UDP"))))
			{
				Generics::OptionalType<uint32> payLoadFormat;

				// Get the payload format
				parser.ReadNumber<uint32, false> (payLoadFormat);

				if ( (payLoadFormat.IsSet()) && (payLoadFormat.Value() <= 127) )
				{
					m_ProtocolType = UDP;
					m_PayLoadFormat = static_cast <uint8>(payLoadFormat.Value());
				}
			}
		}
	}
}

void Message::Parse_C_Line(const Generics::TextFragment& lines) 
{
	Generics::TextParser parser(lines);

	// Check for "c=IN IP4 <connection-endpoint>"
	// or "c=IN IP4 <connection-endpoint>/<ttl+numAddresses>"
	// (Later, do something with <ttl+numAddresses> also #####)
	// Check for "c=IN IP4" line
	if ( (parser.Validate<false>(Generics::TextFragment(_TXT("IN")))) && (parser.Validate<false>(Generics::TextFragment(_TXT("IP4")))) )
	{
		parser.ReadText(m_EndPoint, "\r\n/");
	}
}

void Message::Parse_S_Line (const Generics::TextFragment& lines)
{
	Generics::TextParser parser(lines);

	// Check for "s=<session name>" line
	parser.ReadText(m_SessionName, "\r\n/");
}

void Message::Parse_I_Line (const Generics::TextFragment& lines)
{
	Generics::TextParser parser(lines);

	// Check for "i=<session description>" line
	parser.ReadText(m_SessionDescription, "\r\n/");
}

void Message::Parse_B_Line (const Generics::TextFragment& lines)
{
	Generics::TextParser parser(lines);

	// Check for "b=<bwtype>:<bandwidth>" line
	// RTP applications are expected to use bwtype="AS"
	if (parser.Validate<false>(Generics::TextFragment(_TXT("AS"))))
	{
		parser.ReadNumber<uint32, false>(m_Bandwidth);
	}
}

void Message::Parse_A_Line (const Generics::TextFragment& lines)
{
	// Check for "a=<option>:******" line
	m_Attributes.ParseLine(lines);
}


Message::Attributes::Attributes()
{
}

Message::Attributes::~Attributes()
{
}

void Message::Attributes::ParseLine (const Generics::TextFragment& lines)
{
	Generics::TextParser parser(lines);

	// Detrmine the type of attribute:
	uint32 index = 0;

	//while ( (index < (sizeof(g_AttributeNameTable)/sizeof(AttributeName))) && (_tcsncmp(line, g_AttributeNameTable[index].Name, g_AttributeNameTable[index].Length) != 0) )
	//{
	//	index++;
	//}

	if (index < (sizeof(g_AttributeNameTable)/sizeof(AttributeName)))
	{
		// Seems like we have a valid attribute, continue parsing...
		switch (g_AttributeNameTable[index].Identifier)
		{
		case CONTROL:		ParseControlAttribute (parser);
							break;
		case RANGE:			ParseRangeAttribute (parser);
							break;
		case TYPE:			ParseTypeAttribute (parser);
							break;
		case SOURCE_FILTER:	ParseSourceFilterAttribute (parser);
							break;
		case RTPMAP:		ParseRTPMapAttribute (parser);
							break;
		case FMTP:			ParseFMTPAttribute (parser);
							break;
		case XDIMENSIONS:	ParseXDimensionAttribute (parser);
							break;
		case FRAMERATE:		ParseFramerateAttribute (parser);
							break;
		case XFRAMERATE:	ParseXFramerateAttribute (parser);
							break;
		default:			break;
		}
	}
}

void Message::Attributes::ParseRTPMapAttribute(const Generics::TextFragment& lines)
{
	Generics::TextParser parser(lines);

	// Check for a "a=rtpmap:<fmt> <codec>/<freq>" line:
	// (Also check without the "/<freq>"; RealNetworks omits this)
	// Also check for a trailing "/<numChannels>".

	// Extract the first number from the stream, it's the PayLoadFormat.
	parser.ReadNumber<uint8, false> (m_PayloadFormat);

	// Next is the CodecName, Find the closing pointer.. 
	parser.ReadText(m_Codec, _T("\n\r/"));

	// If there is a slash, move over it and continue parsing...
	if (parser.OnMarker(_T("/")))
	{
		parser.Skip(1);

		// There might be a TimeStampFrequency, check it.
		parser.ReadNumber<uint32, false> (m_TimestampFrequency);

		// If there is a slash, move over it and continue parsing...
		if (parser.OnMarker(_T("/")))
		{
			parser.Skip(1);

			// There might be a Channels, check it.
			parser.ReadNumber<uint32, false> (m_Channels);
		}
	}
}

void Message::Attributes::ParseControlAttribute(const Generics::TextFragment& lines) 
{
	Generics::TextParser parser(lines);

	// Check for a "a=control:<control-path>" line:
	parser.ReadText (m_ControlPath, _T("\n\r"));
}

void Message::Attributes::ParseRangeAttribute(const Generics::TextFragment& lines)
{
	Generics::TextParser parser(lines);

	// Check for a "a=range:npt=<startTime>-<endTime>" line:
	// (Later handle other kinds of "a=range" attributes also???#####)
	Generics::OptionalType<Generics::KeyValueType<true>> keyValue;

	parser.ReadKeyValuePair<'='> (keyValue, _T("\n\r"));

	if ((keyValue.IsSet() == true) && (keyValue.Value().IsKey(Generics::TextFragment(_TXT("npt"))) == true))
	{
		parser.ReadNumber<double, false>(m_PlayStartTime);

		// Now we should be fcing the '-'
		if (parser.OnMarker(_T("-")))
		{
			parser.Skip(1);
			parser.ReadNumber<double, false>(m_PlayEndTime);
		}
	}
}

void Message::Attributes::ParseFMTPAttribute(const Generics::TextFragment& lines)
{
	Generics::TextParser parser(lines);
	Generics::OptionalType<Generics::KeyValueType<true>> result;

	// Check for a "a=fmtp:" line:
	// TEMP: We check only for a handful of expected parameter names #####
	// Later: (i) check that payload format number matches; #####
	//        (ii) look for other parameters also (generalize?) #####
	do 
	{
		// The remaining "sdpLine" should be a sequence of
		//     <name>=<value>;
		// parameter assignments.  Look at each of these.
		// First, convert the line to lower-case, to ease comparison:
		parser.ReadKeyValuePair<'='>(result, _T(";\n\r"));

		if (result.IsSet())
		{
			if (result.Value().IsKey(Generics::TextFragment(_TXT("crc"))) == true) // Could be boolen as well
			{
				result.Value().Text().Number<uint8, false>(m_CRC);
			}
			else if (result.Value().IsKey(Generics::TextFragment(_TXT("octet-align"))) == true)  // Could be boolen as well
			{
				result.Value().Text().Number<uint8, false>(m_OctetAlign);
			}
			else if (result.Value().IsKey(Generics::TextFragment(_TXT("cpresent"))) == true) // Could be boolen as well
			{
				result.Value().Text().Number<uint8, false>(m_CPresent);
			}
			else if (result.Value().IsKey(Generics::TextFragment(_TXT("robust-sorting"))) == true) // Could be boolen as well
			{
				result.Value().Text().Number<uint8, false>(m_RobustSorting);
			}
			else if (result.Value().IsKey(Generics::TextFragment(_TXT("randomaccessindication"))) == true) // Could be boolen as well
			{
				result.Value().Text().Number<uint8, false>(m_RandomAccessIndication);
			}
			else if (result.Value().IsKey(Generics::TextFragment(_TXT("constantduration"))) == true)
			{
				result.Value().Text().Number<uint32, false>(m_ConstantDuration);
			}
			else if (result.Value().IsKey(Generics::TextFragment(_TXT("constantsize"))) == true)
			{
				result.Value().Text().Number<uint32, false>(m_ConstantSize);
			}
			else if (result.Value().IsKey(Generics::TextFragment(_TXT("ctsdeltalength"))) == true)
			{
				result.Value().Text().Number<uint32, false>(m_CTSDeltaLength);
			}
			else if (result.Value().IsKey(Generics::TextFragment(_TXT("de-interleavebuffersize"))) == true)
			{
				result.Value().Text().Number<uint32, false>(m_InterleavedBufferSize);
			}
			else if (result.Value().IsKey(Generics::TextFragment(_TXT("dtsdeltalength"))) == true)
			{
				result.Value().Text().Number<uint32, false>(m_DTSDeltaLength);
			}
			else if (result.Value().IsKey(Generics::TextFragment(_TXT("indexlength"))) == true)
			{
				result.Value().Text().Number<uint32, false>(m_IndexLength);
			}
			else if (result.Value().IsKey(Generics::TextFragment(_TXT("interleaving"))) == true)
			{
				result.Value().Text().Number<uint32, false>(m_InterLeaving);
			}
			else if (result.Value().IsKey(Generics::TextFragment(_TXT("maxdisplacement"))) == true)
			{
				result.Value().Text().Number<uint32, false>(m_MaxDisplacement);
			}
			else if (result.Value().IsKey(Generics::TextFragment(_TXT("objecttype"))) == true)
			{
				result.Value().Text().Number<uint32, false>(m_ObjectType);
			}
			else if (result.Value().IsKey(Generics::TextFragment(_TXT("profile-level-id"))) == true)
			{
				result.Value().Text().Number<uint32, false>(m_ProfileLevelID, BASE_HEXADECIMAL);
			}
			else if (result.Value().IsKey(Generics::TextFragment(_TXT("sizelength"))) == true)
			{
				result.Value().Text().Number<uint32, false>(m_SizeLength);
			}
			else if (result.Value().IsKey(Generics::TextFragment(_TXT("streamstateindication"))) == true)
			{
				result.Value().Text().Number<uint32, false>(m_StreamStateIndication);
			}
			else if (result.Value().IsKey(Generics::TextFragment(_TXT("streamtype"))) == true)
			{
				result.Value().Text().Number<uint32, false>(m_StreamType);
			}
			else if (result.Value().IsKey(Generics::TextFragment(_TXT("config"))) == true)
			{
				m_Config = result.Value().Text();
			}
			else if (result.Value().IsKey(Generics::TextFragment(_TXT("mode"))) == true)
			{
				m_Mode = result.Value().Text();
			}
			else if (result.Value().IsKey(Generics::TextFragment(_TXT("sprop-parameter-sets"))) == true)
			{
				m_PropParameterSets = result.Value().Text();
			}
		}

	} while (result.IsSet());
}


void Message::Attributes::ParseXDimensionAttribute(const Generics::TextFragment& lines)
{
	Generics::TextParser parser(lines);

	// Check for a "a=x-dimensions:<width>,<height>" line:
	parser.ReadNumber<unsigned short, false>(m_VideoWidth);

	parser.SkipWhiteSpace();

	// Now we should be facing the ','
	if (parser.OnMarker(_T(",")))
	{
		parser.Skip(1);
		parser.ReadNumber<uint16, false>(m_VideoHeight);
	}
}

void Message::Attributes::ParseFramerateAttribute(const Generics::TextFragment& /* lines */)
{
	//float frate;

	//if (sscanf_s(sdpLine, "%f", &frate) == 1) 
	//{
	//	m_FrameRate = static_cast<uint32> (frate);
	//}
}

void Message::Attributes::ParseXFramerateAttribute(const Generics::TextFragment& lines)
{
	Generics::TextParser parser(lines);

	// Check for a "a=x-framerate: <fps>" line:
	parser.ReadNumber<uint32, false>(m_FrameRate);
}

void Message::Attributes::ParseSourceFilterAttribute(const Generics::TextFragment& lines)
{
	Generics::TextParser parser(lines);

	// Check for a "a=source-filter:incl IN IP4 <something> <source>" line.
	// Note: At present, we don't check that <something> really matches
	// one of our multicast addresses.  We also don't support more than
	// one <source> #####
	// Make sure we find the right headers...
	if ( (parser.Validate<false>(Generics::TextFragment(_TXT("INCL"))) == true) && 
		 (parser.Validate<false>(Generics::TextFragment(_TXT("IN")))   == true) &&
		 (parser.Validate<false>(Generics::TextFragment(_TXT("IP4")))  == true) )
	{
		// Now we are up to the words....
		Generics::OptionalType<Generics::TextFragment> word;

		// Read the  first word and drop it..
		parser.ReadText(word, _T("\t\r\n "));

		while (word.IsSet())
		{
			// The next word we need it..
			parser.ReadText(word, _T("\t\r\n "));

			if (word.IsSet())
			{
				m_SourceFilters.push_back(word.Value());
			}
		}
	}
}

void Message::Attributes::ParseTypeAttribute(const Generics::TextFragment& lines)
{
	Generics::TextParser parser(lines);

	// Now we are up to the words....
	Generics::OptionalType<Generics::TextFragment> word;

	// Read the  first word and drop it..
	parser.ReadText(word, _T("\t\r\n "));

	// Check for a "a=type:broadcast|meeting|moderated|test|H.332|recvonly" line:
	// See if we find this word in our vocabulary ;-)
	if (word.IsSet() == true)
	{
		uint32 index = 0;

		while ( (index < (sizeof(g_AttributeTypeTable)/sizeof(AttributeType))) &&
			    (word.Value() != (Generics::TextFragment(g_AttributeTypeTable[index].Name, g_AttributeTypeTable[index].Length))) )
		{
			index++;
		}

		if (index < (sizeof(g_AttributeTypeTable)/sizeof(AttributeType)))
		{
			// Seems like we found it..
			m_Type = g_AttributeTypeTable[index].Identifier;
		}
		
	}
}

} } // namespace Solutions::SDP