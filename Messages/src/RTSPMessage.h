#ifndef __RTSPMESSAGE_H
#define __RTSPMESSAGE_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"

// ---- Referenced classes and types ----

// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace RTSP
{
	// ---- Helper types and constants ----
	typedef Generics::RangeType<Generics::Fractional,true,false>		TimeRange;
	typedef Generics::RangeType<uint16, true, false>					PortRange;

	class EXTERNAL Message
	{
		public:
			typedef enum
			{
				CACHE_CONTROL = 0,			// g
				CONNECTION = 1,				// g
				CSEQ = 2,					// g
				DATE = 3,					// g
				VIA = 4,					// g
				ACCEPT = 5,					// R
				ACCEPT_ENCODING = 6,		// R
				ACCEPT_LANGUAGE = 7,		// R
				AUTHORIZATION = 8,			// R
				BANDWIDTH = 9,				// R
				BLOCKSIZE = 10,				// R
				CONFERENCE = 11,			// R
				FROM = 12,					// R
				IF_MODIFIED_SINCE = 13,		// R
				PROXY_REQUIRE = 14,			// R
				REFERER = 15,				// R
				REQUIRE = 16,				// R
				USER_AGENT = 17,			// R
				RANGE = 18,					// Rr
				SCALE = 19,					// Rr
				SESSION = 20,				// Rr
				SPEED = 21,					// Rr
				TRANSPORT = 22,				// Rr
				ALLOW = 23,					// r
				CONTENT_TYPE = 24,			// re
				PUBLIC = 25,				// r
				RETRY_AFTER = 26,			// r
				RTP_INFO = 27,				// r
				SERVER = 28,				// r
				UNSUPPORTED = 29,			// r
				WWW_AUTHENTICATE = 30,		// r
				CONTENT_BASE = 31,			// e
				CONTENT_ENCODING = 32,		// e
				CONTENT_LANGUAGE = 33,		// e
				CONTENT_LENGTH = 34,		// e
				CONTENT_LOCATION = 35,		// e
				EXPIRES = 36,				// e
				LAST_MODIFIED = 37,			// e
				PROXY_AUTHENTICATION = 38	// e

			}	enumHeaderType;

			typedef enum 
			{
				OPTIONS = 0,
				DESCRIBE = 1,
				ANNOUNCE = 2,
				SETUP = 3,
				PLAY = 4,
				PAUSE = 5,
				TEARDOWN = 6,
				GET_PARAMETER = 7,
				SET_PARAMETER = 8,
				REDIRECT = 9,
				RECORD = 10

			}	enumRequestType;

		private:
			Message(const Message&);
			Message& operator= (const Message&);

		public:
			Message ();
			Message (const TCHAR text[]);
			~Message ();

			String Data() const;

			// General parameters
			inline const Generics::OptionalType<uint8>&			  				MajorVersion () const { return (m_Major);           }
			inline const Generics::OptionalType<uint8>&	          				MinorVersion () const { return (m_Minor);           }
			inline const Generics::OptionalType<uint32>&			  			Sequence     () const { return (m_Sequence);        }
			inline const Generics::OptionalType<Generics::Time>&			  	Date	     () const { return (m_DateTime);        }
			inline const Generics::OptionalType<Generics::TextFragment>&		Session      () const { return (m_Session);         }
			inline const Generics::OptionalType<Generics::TextFragment>&		ContentBase  () const { return (m_ContentBase);     }
			inline const Generics::OptionalType<Generics::TextFragment>&		ContentType  () const { return (m_ContentType);     }
			inline const Generics::OptionalType<Generics::TextFragment>&		Content      () const { return (m_Content);		    }
			inline const Generics::OptionalType<Generics::TextFragment>&		Range        () const { return (m_Range);		    }

			inline void MajorVersion (const Generics::OptionalType<uint8>& value)					{ m_Major = value;           }
			inline void MinorVersion (const Generics::OptionalType<uint8>& value)					{ m_Minor = value;           }
			inline void Sequence     (const Generics::OptionalType<uint32>& value)					{ m_Sequence = value;        }
			inline void	Date	     (const Generics::OptionalType<Generics::Time>& value)			{ m_DateTime = value;        }
			inline void	Session	     (const Generics::OptionalType<Generics::TextFragment>& value)	{ m_Session = value;         }
			inline void	ContentBase  (const Generics::OptionalType<Generics::TextFragment>&	value)  { m_ContentBase = value;     }
			inline void	ContentType  (const Generics::OptionalType<Generics::TextFragment>&	value)  { m_ContentType = value;     }
			inline void	Content      (const Generics::OptionalType<Generics::TextFragment>& value)	{ m_Content = value;         }
			inline void	Range        (const Generics::OptionalType<Generics::TextFragment>& value)	{ m_Range = value;           }

			// Request parameters
			inline const Generics::OptionalType<enumRequestType>&	  						RequestType  () const { return (m_Request);         }
			inline const Generics::OptionalType<Generics::TextFragment>&	  				URI          () const { return (m_URI);             }
			inline const Generics::OptionalType<Generics::TextFragment>&       				UserAgent    () const { return (m_UserAgent);       }
			inline const Generics::OptionalType<Generics::TextFragment>&      				Accept       () const { return (m_Accept);          }
			inline const Generics::OptionalType<Generics::TextFragment>&      				Transport    () const { return (m_Transport);       }

			inline void	RequestType  (const Generics::OptionalType<enumRequestType>& value)			  { m_Request = value;         }
			inline void	URI          (const Generics::OptionalType<Generics::TextFragment>& value)    { m_URI = value;             }
			inline void	UserAgent    (const Generics::OptionalType<Generics::TextFragment>& value)    { m_UserAgent = value;       }
			inline void	Accept       (const Generics::OptionalType<Generics::TextFragment>& value)    { m_Accept = value;          }
			inline void	Transport    (const Generics::OptionalType<Generics::TextFragment>& value)    { m_Transport = value;       }

			// Response parameters
			inline const Generics::OptionalType<uint16>&							Status				() const { return (m_Status);				}
			inline const Generics::OptionalType<Generics::TextFragment>&			StatusDescription   () const { return (m_StatusDescription);	}
			inline const Generics::OptionalType<Generics::TextFragment>&			Public				() const { return (m_Public);				}
			inline const Generics::OptionalType<Generics::TextFragment>&			RTPInfo				() const { return (m_RTPInfo);				}

			inline void	Status				(const Generics::OptionalType<uint16>& value)					{ m_Status = value;				}
			inline void	StatusDescription   (const Generics::OptionalType<Generics::TextFragment>& value)	{ m_StatusDescription = value;  }
			inline void	Public				(const Generics::OptionalType<Generics::TextFragment>& value)	{ m_Public = value;				}
			inline void	RTPInfo				(const Generics::OptionalType<Generics::TextFragment>& value)	{ m_RTPInfo = value;			}

			// Helper methods for extracting interesting information in the message
			Generics::OptionalType<Generics::TextFragment> ChannelId () const;
			Generics::OptionalType<Generics::TextFragment> BaseURI   () const;

			// Creator
			static Generics::ProxyType<Message>	Create (const Generics::TextFragment& message);

		private:
			void Parse (const Generics::TextFragment& text);
			const TCHAR* ParseOptionLine (const TCHAR lines[]);

		private:
			Generics::OptionalType<uint32>						m_Sequence;
			Generics::OptionalType<enumRequestType>				m_Request;
			Generics::OptionalType<uint8>						m_Major;
			Generics::OptionalType<uint8>						m_Minor;
			Generics::OptionalType<uint16>						m_Status;
			Generics::OptionalType<Generics::TextFragment>		m_StatusDescription;
			Generics::OptionalType<Generics::TextFragment>		m_URI;
			Generics::OptionalType<Generics::TextFragment>		m_UserAgent;
			Generics::OptionalType<Generics::TextFragment>		m_Accept;
			Generics::OptionalType<Generics::TextFragment>		m_Transport;
			Generics::OptionalType<Generics::TextFragment>		m_Session;
			Generics::OptionalType<Generics::Time>				m_DateTime;
			Generics::OptionalType<Generics::TextFragment>		m_Public;
			Generics::OptionalType<Generics::TextFragment>		m_ContentBase;
			Generics::OptionalType<Generics::TextFragment>		m_ContentType;
			Generics::OptionalType<Generics::TextFragment>		m_Content;
			Generics::OptionalType<Generics::TextFragment>		m_Range;
			Generics::OptionalType<Generics::TextFragment>		m_RTPInfo;
	};

	class EXTERNAL MessageOptions
	{
		public:
			typedef enum
			{
				OPTIONS = 0,
				DESCRIBE = 1,
				ANNOUNCE = 2,
				SETUP = 3,
				PLAY = 4,
				PAUSE = 5,
				TEARDOWN = 6,
				GET_PARAMETER = 7,
				SET_PARAMETER = 8,
				REDIRECT = 9,
				RECORD = 10,
				RTPAVP_TYPE = 11,
				DESTINATION = 12,
				SOURCE = 13,
				CLIENTPORT = 14,
				SERVERPORT = 15,
				UNICAST = 16,
				MULTICAST = 17,
				URLTEXT = 18,
				SEQUENCE = 19,
				RTPTIME = 20,
				NPT = 21

			}	enumKeyWords;

			typedef enum
			{
				RTPAVP = 0

			} ProtocolType;

		private:
			MessageOptions(const MessageOptions&);
			MessageOptions& operator= (const MessageOptions&);

		public:
			MessageOptions ();
			MessageOptions (const Generics::TextFragment& info);
			~MessageOptions ();

		public:
			String Message() const;
		
			inline const Generics::OptionalType<bool>						IsUnicast		() const { return (m_Unicast);          }
			inline const Generics::OptionalType<ProtocolType>				Protocol		() const { return (m_ProtocolType);     }
			inline const Generics::OptionalType<PortRange>					ClientPorts		() const { return (m_ClientPortRange);  }
			inline const Generics::OptionalType<PortRange>					ServerPorts		() const { return (m_ServerPortRange);  }
			inline const Generics::OptionalType<Generics::TextFragment>		Source			() const { return (m_Source);			}
			inline const Generics::OptionalType<Generics::TextFragment>		Destination		() const { return (m_Destination);		}
			inline const Generics::OptionalType<uint32>						AllowedRequest  () const { return (m_RequestsAllowed);	}
			inline const Generics::OptionalType<Generics::TextFragment>		URL				() const { return (m_URL);				}
			inline const Generics::OptionalType<uint32>						Sequence		() const { return (m_Sequence);			}
			inline const Generics::OptionalType<uint32>						RTPTime			() const { return (m_RTPTime);			}
			inline const Generics::OptionalType<TimeRange>					NPTTime			() const { return (m_NPTRange);			}

			inline void	 IsUnicast		(const Generics::OptionalType<bool>& value)						{ m_Unicast	= value;		 }
			inline void	 Protocol		(const Generics::OptionalType<ProtocolType>& value)				{ m_ProtocolType = value;    }
			inline void	 ClientPorts	(const Generics::OptionalType<PortRange>& value)				{ m_ClientPortRange = value; }
			inline void	 ServerPorts	(const Generics::OptionalType<PortRange>& value)				{ m_ServerPortRange = value; }
			inline void	 Source			(const Generics::OptionalType<Generics::TextFragment>& value)	{ m_Source = value;          }
			inline void	 Destination	(const Generics::OptionalType<Generics::TextFragment>& value)	{ m_Destination = value;     }
			inline void	 AllowedRequest (const Generics::OptionalType<uint32>& value)					{ m_RequestsAllowed = value; }
			inline void  URL			(const Generics::OptionalType<Generics::TextFragment> value)	{ m_URL = value;			 }
			inline void  Sequence		(const Generics::OptionalType<uint32>	value)					{ m_Sequence = value;		 }
			inline void  RTPTime		(const Generics::OptionalType<uint32> value)					{ m_RTPTime = value;		 }
			inline void  NPTTime		(const Generics::OptionalType<TimeRange> value)					{ m_NPTRange = value;		 }
					
		private:
			void ParseOptionLine (const Generics::TextFragment& text);

		private:
			const TCHAR*									m_Information;
			Generics::OptionalType<bool>					m_Unicast;
			Generics::OptionalType<ProtocolType>			m_ProtocolType;
			Generics::OptionalType<PortRange>				m_ClientPortRange;
			Generics::OptionalType<PortRange>				m_ServerPortRange;
			Generics::OptionalType<Generics::TextFragment>	m_Source;
			Generics::OptionalType<Generics::TextFragment>	m_Destination;
			Generics::OptionalType<uint32>					m_RequestsAllowed;
			Generics::OptionalType<Generics::TextFragment>	m_URL;
			Generics::OptionalType<uint32>					m_Sequence;
			Generics::OptionalType<uint32>					m_RTPTime;
			Generics::OptionalType<TimeRange>				m_NPTRange;
	};

} } // namespace Solutions::RTSP

#endif // __RTSPMESSAGE_H