#include "UniformResourceLocator.h"
#include "ParseURL.h"
#include "Enumerate.h"

// StandardURL is for when the scheme is known to be one that has an
// authority (host) like "http". This function will not handle weird ones
// like "about:" and "javascript:", or do the right thing for "file:" URLs.
// void ParseStandardURL(const TCHAR url[], int url_len, Parsed& parsed);

// PathURL is for when the scheme is known not to have an authority (host)
// section but that aren't file URLs either. The scheme is parsed, and
// everything after the scheme is considered as the path. This is used for
// things like "about:" and "javascript:"
// void ParsePathURL(const TCHAR url[], int url_len, Parsed& parsed);

// FileURL is for file URLs. There are some special rules for interpreting
// these.
// void ParseFileURL(const TCHAR url[], int url_len, Parsed& parsed);

// MailtoURL is for mailto: urls. They are made up scheme,path,query
// void ParseMailtoURL(const TCHAR url[], int url_len, Parsed& parsed);

namespace Solutions { namespace Generics
{

typedef void (*ParseFunction)(const TCHAR[], int, Parsed&);
typedef void (*CreateFunction)(TCHAR[], int, const URL& info);

typedef struct
{
	URL::SchemeType		m_Scheme;
	TCHAR*				m_SchemeString;
	unsigned short		m_SchemeLength;
	unsigned short		m_Port;
	ParseFunction		m_ParseFunction;
	CreateFunction		m_CreateFunction;

} SchemeInfo;

const SchemeInfo	g_SchemeOverview[] =
{
	{ URL::SCHEME_FILE,    _TXT("file"),     0, ParseFileURL,     URL::CreateFileURL     },
	{ URL::SCHEMA_MAIL,    _TXT("mailto"),   0, ParseMailtoURL,   URL::CreateMailtoURL   },
	{ URL::SCHEME_HTTP,    _TXT("http"),    80, ParseStandardURL, URL::CreateStandardURL },
	{ URL::SCHEME_HTTPS,   _TXT("https"),  443, ParseStandardURL, URL::CreateStandardURL },
	{ URL::SCHEME_FTP,     _TXT("ftp"),     21, ParseStandardURL, URL::CreateStandardURL },
	{ URL::SCHEME_TELNET,  _TXT("telnet"),  23, ParseStandardURL, URL::CreateStandardURL },
	{ URL::SCHEME_GOPHER,  _TXT("gopher"),  70, ParseStandardURL, URL::CreateStandardURL },
	{ URL::SCHEME_LDAP,    _TXT("ldap"),   389, ParseStandardURL, URL::CreateStandardURL },
	{ URL::SCHEME_RTSP,    _TXT("rtsp"),   554, ParseStandardURL, URL::CreateStandardURL },
	{ URL::SCHEME_RTP,     _TXT("rtp"),    554, ParseStandardURL, URL::CreateStandardURL },
	{ URL::SCHEME_RTCP,    _TXT("rtcp"),   554, ParseStandardURL, URL::CreateStandardURL },
	{ URL::SCHEME_RTP_UDP, _TXT("rtpudp"), 554, ParseStandardURL, URL::CreateStandardURL },
	{ URL::SCHEME_RTP_TCP, _TXT("rtptcp"), 554, ParseStandardURL, URL::CreateStandardURL },
	{ URL::SCHEME_UNKNOWN, _TXT(""),         0, ParseStandardURL, URL::CreateStandardURL }
};

ENUM_CONVERSION_TABLE(URL::SchemeType) 
{
	{ URL::SCHEME_FILE,    _TXT("file")   },
	{ URL::SCHEMA_MAIL,    _TXT("mailto") },
	{ URL::SCHEME_HTTP,    _TXT("http")   },
	{ URL::SCHEME_HTTPS,   _TXT("https")  },
	{ URL::SCHEME_FTP,     _TXT("ftp")    },
	{ URL::SCHEME_TELNET,  _TXT("telnet") },
	{ URL::SCHEME_GOPHER,  _TXT("gopher") },
	{ URL::SCHEME_LDAP,    _TXT("ldap")   },
	{ URL::SCHEME_RTSP,    _TXT("rtsp")   },
	{ URL::SCHEME_RTP,     _TXT("rtp")    },
	{ URL::SCHEME_RTCP,    _TXT("rtcp")   },
	{ URL::SCHEME_RTP_UDP, _TXT("rtpudp") },
	{ URL::SCHEME_RTP_TCP, _TXT("rtptcp") },
	{ URL::SCHEME_UNKNOWN, _TXT("????"),  }
};

TextFragment URL::Text () const
{
	TextFragment result;

	if (m_SchemeInfo != NULL)
	{
		// Seems like we need to construct it...
		// Create somecharacter space...
		TCHAR* buffer = reinterpret_cast<TCHAR*> (alloca((MAX_URL_SIZE + 1) * sizeof(TCHAR)));

		reinterpret_cast<const SchemeInfo*>(m_SchemeInfo)->m_CreateFunction(buffer, MAX_URL_SIZE, *this);

		result = TextFragment(String(buffer));		
	}

	return (result);
}

void URL::SetScheme(const URL::SchemeType type)
{
	uint8 index = 0;
	m_SchemeInfo = NULL;

	while ( (m_SchemeInfo == NULL) && (index < (sizeof(g_SchemeOverview)/sizeof(SchemeInfo))) )
	{
		if (g_SchemeOverview[index].m_Scheme == type)
		{
			m_SchemeInfo = &(g_SchemeOverview[index]);
			m_Scheme = Generics::TextFragment(g_SchemeOverview[index].m_SchemeString, g_SchemeOverview[index].m_SchemeLength);
		}
		else
		{
			++index;
		}
	}
}

URL::SchemeType URL::Type () const
{
	SchemeType result = URL::SCHEME_UNKNOWN;

	if (m_SchemeInfo != NULL)
	{
		result =reinterpret_cast<const SchemeInfo*>(m_SchemeInfo)->m_Scheme;
	}

	return (result);
}

void URL::Parse (const TCHAR urlStr[], int length)
{
	bool caseSensitive = true;
	Component result;

	if (ExtractScheme(urlStr, length, result) == true)
	{
		// SchemeInfo information found! Set it!
		m_Scheme = TextFragment(urlStr, result.begin(), result.length());
		
		unsigned int index = 0;

		// Find additional info:
		while ( (index < (sizeof(g_SchemeOverview)/sizeof(SchemeInfo))) &&
			    ( (g_SchemeOverview[index].m_SchemeLength != result.length()) ||
				((caseSensitive == true)  && (_tcsncmp (g_SchemeOverview[index].m_SchemeString, m_Scheme.Value().Data(), g_SchemeOverview[index].m_SchemeLength) != 0)) ||
				((caseSensitive == false) && (_tcsnicmp(g_SchemeOverview[index].m_SchemeString, m_Scheme.Value().Data(), g_SchemeOverview[index].m_SchemeLength) != 0)) ) )
		{
			index++;
		}

		Parsed parseInfo;

		if (index == (sizeof(g_SchemeOverview)/sizeof(SchemeInfo)))
		{
			m_SchemeInfo = NULL;
			ParseStandardURL(urlStr, length, parseInfo);
		}
		else
		{
			m_SchemeInfo = &(g_SchemeOverview[index]);
			g_SchemeOverview[index].m_ParseFunction (urlStr, length, parseInfo);
			m_Port = OptionalType<unsigned short>(g_SchemeOverview[index].m_Port);
		}

		// Copy all other components
		SetComponent (urlStr, parseInfo.username.begin(), parseInfo.username.length(), m_Username);
		SetComponent (urlStr, parseInfo.password.begin(), parseInfo.password.length(), m_Password);
		SetComponent (urlStr, parseInfo.host.begin(),     parseInfo.host.length(),     m_Host);
		if (parseInfo.path.length() > 0)
		{
			SetComponent (urlStr, parseInfo.path.begin()+1, parseInfo.path.length()-1, m_Path);
		}
		SetComponent (urlStr, parseInfo.query.begin(),    parseInfo.query.length(),    m_Query);
		SetComponent (urlStr, parseInfo.ref.begin(),      parseInfo.ref.length(),      m_Ref);

		if (parseInfo.port.is_valid ())
		{
			m_Port = OptionalType<unsigned short>(static_cast<unsigned short>(_tstoi(&(urlStr[parseInfo.port.begin()]))));
		}
	}
}

void URL::SetComponent (const TCHAR urlStr[], const unsigned int begin, const unsigned int length, OptionalType<TextFragment>& setInfo)
{
	if (length != ~0)
	{
		setInfo = TextFragment(urlStr, begin, length);
	}
}

// StandardURL is for when the scheme is known to be one that has an
// authority (host) like "http". This function will not handle weird ones
// like "about:" and "javascript:", or do the right thing for "file:" URLs.
void URL::CreateStandardURL(TCHAR url[], int url_len ) const
{
	if (IsValid())
	{
		_tcscpy_s(url, url_len, Generics::EnumerateType<URL::SchemeType>::Name(Type()));

		uint32 index = _tcslen(url);

		_tcscpy_s(&url[index], url_len-index, _T("://"));

		index += 3;

		if (m_Username.IsSet())
		{
			_tcscpy_s(&url[index], url_len-index, m_Username.Value().Data());
	
			index += m_Username.Value().Length();

			if (m_Password.IsSet())
			{
				url[index] = ':';
				_tcscpy_s(&url[index+1], url_len - index - 1, m_Password.Value().Data());
				index += m_Password.Value().Length() + 1;
			}

			url[index] = '@';
			index += 1;
		}

		if (m_Host.IsSet())
		{
			_tcscpy_s(&url[index], url_len - index, m_Host.Value().Data());
			index += m_Host.Value().Length();

			if (m_Port.IsSet())
			{
				String number (Generics::NumberUnsigned16(m_Port.Value()));

				url[index] = ':';
				_tcscpy_s(&url[index+1], url_len - index - 1, number.data());
				index += number.length() + 1;
			}
		}

		url[index] = '/';
		index += 1;

		if (m_Path.IsSet())
		{
			_tcscpy_s(&url[index], url_len - index, m_Path.Value().Data());
			index += m_Path.Value().Length();
		}

		if (m_Query.IsSet())
		{
			url[index] = '?';
			_tcscpy_s(&url[index+1], url_len - index - 1, m_Query.Value().Data());
			index += m_Query.Value().Length() + 1;
		}
		if (m_Ref.IsSet())
		{
			url[index] = '#';
			_tcscpy_s(&url[index+1], url_len - index - 1, m_Ref.Value().Data());
			index += m_Ref.Value().Length() + 1;
		}

		url[index] = '\0';
	}
}

// PathURL is for when the scheme is known not to have an authority (host)
// section but that aren't file URLs either. The scheme is parsed, and
// everything after the scheme is considered as the path. This is used for
// things like "about:" and "javascript:"
void URL::CreatePathURL(TCHAR[] /* url */, int /* url_len*/) const
{
	ASSERT (TRUE);
}

// FileURL is for file URLs. There are some special rules for interpreting
// these.
void URL::CreateFileURL(TCHAR url[], int url_len) const
{
	if (IsValid())
	{
		_tcscpy_s(url, url_len, Generics::EnumerateType<URL::SchemeType>::Name(Type()));

		uint32 index = _tcslen(url);

		_tcscpy_s(&url[index], url_len-index, _T("://"));

		index += 3;

		if (m_Host.IsSet())
		{
			_tcscpy_s(&url[index], url_len - index, m_Host.Value().Data());
			index += m_Host.Value().Length();
		}

		url[index] = '/';
		index += 1;

		if (m_Path.IsSet())
		{
			_tcscpy_s(&url[index], url_len - index, m_Path.Value().Data());
			index += m_Path.Value().Length();
		}

		url[index] = '\0';
	}
}

// MailtoURL is for mailto: urls. They are made up scheme,path,query
void URL::CreateMailtoURL(TCHAR[] /* url */, int /* url_len*/) const
{
	ASSERT(TRUE);
}

} } // namespace Solutions::Generics