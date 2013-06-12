#ifndef __SDPMESSAGE_H
#define __SDPMESSAGE_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace SDP
{
	class EXTERNAL Message
	{
		public:
			typedef Generics::IteratorType<std::list<Generics::TextFragment>, Generics::TextFragment> SourceFilterItertor; 

			class Attributes
			{
				friend class Message;

				public:
					enum enumAttributeName
					{
						CONTROL,
						RANGE,
						TYPE,
						SOURCE_FILTER,
						RTPMAP,
						FMTP,
						XDIMENSIONS,
						FRAMERATE,
						XFRAMERATE,
						UNKNOWN
					};

					enum enumAttributeType
					{
						BROADCAST,
						MEETING,
						MODERATED,
						TEST,
						H332,
						RECVONLY
					};

					typedef struct 
					{
						enumAttributeName	Identifier;
						TCHAR*				Name;
						uint32				Length;

					} AttributeName;

					typedef struct 
					{
						enumAttributeType	Identifier;
						TCHAR*				Name;
						uint32				Length;

					} AttributeType;

				private:
					Attributes (const Attributes&);
					Attributes& operator= (const Attributes&);

				public:
					Attributes();
					~Attributes();

				public:
					inline const Generics::OptionalType<uint8>&						PayLoadFormat			() const { return (m_PayloadFormat);						}
					inline const Generics::OptionalType<uint32>&					TimestampFrequency		() const { return (m_TimestampFrequency);					}
					inline const Generics::OptionalType<uint32>&					Channels				() const { return (m_Channels);								}
					inline const Generics::OptionalType<Generics::TextFragment>&	Codec					() const { return (m_Codec);								}
					inline const Generics::OptionalType<Generics::TextFragment>&	ControlPath				() const { return (m_ControlPath);							}
					inline const Generics::OptionalType<double>&					PlayStartTime			() const { return (m_PlayStartTime);						}
					inline const Generics::OptionalType<double>&					PlayEndTime				() const { return (m_PlayEndTime);							}
					inline const Generics::OptionalType<uint16>&					VideoWidth				() const { return (m_VideoWidth);							}
					inline const Generics::OptionalType<uint16>&					VideoHeight				() const { return (m_VideoHeight);							}
					inline const Generics::OptionalType<uint32>&					FrameRate				() const { return (m_FrameRate);							}
					inline const Generics::OptionalType<uint8>&						CRC						() const { return (m_CRC);									}
					inline const Generics::OptionalType<uint8>&						OctetAlign				() const { return (m_OctetAlign);							}
					inline const Generics::OptionalType<uint8>&						CPresent				() const { return (m_CPresent);								}
					inline const Generics::OptionalType<uint8>&						RobustSorting			() const { return (m_RobustSorting);						}
					inline const Generics::OptionalType<uint8>&						RandomAccessIndication	() const { return (m_RandomAccessIndication);				}
					inline const Generics::OptionalType<uint32>&					ConstantDuration		() const { return (m_ConstantDuration);						}
					inline const Generics::OptionalType<uint32>&					ConstantSize			() const { return (m_ConstantSize);							}
					inline const Generics::OptionalType<uint32>&					CTSDeltaLength			() const { return (m_CTSDeltaLength);						}
					inline const Generics::OptionalType<uint32>&					DTSDeltaLength			() const { return (m_DTSDeltaLength);						}
					inline const Generics::OptionalType<uint32>&					IndexLength				() const { return (m_IndexLength);							}
					inline const Generics::OptionalType<uint32>&					InterLeaving			() const { return (m_InterLeaving);							}
					inline const Generics::OptionalType<uint32>&					MaxDisplacement			() const { return (m_MaxDisplacement);						}
					inline const Generics::OptionalType<uint32>&					ObjectType				() const { return (m_ObjectType);							}
					inline const Generics::OptionalType<uint32>&					ProfileLevelID			() const { return (m_ProfileLevelID);						}
					inline const Generics::OptionalType<uint32>&					SizeLength				() const { return (m_SizeLength);							}
					inline const Generics::OptionalType<uint32>&					StreamStateIndication	() const { return (m_StreamStateIndication);				}
					inline const Generics::OptionalType<uint32>&					StreamType				() const { return (m_StreamType);							}
					inline const Generics::OptionalType<uint32>&					InterleavedBufferSize	() const { return (m_InterleavedBufferSize);				}
					inline const Generics::OptionalType<Generics::TextFragment>&	Config					() const { return (m_Config);								}
					inline const Generics::OptionalType<Generics::TextFragment>&	Mode					() const { return (m_Mode);									}
					inline const Generics::OptionalType<Generics::TextFragment>&	PropParameterSets		() const { return (m_PropParameterSets);					}
					inline const Generics::OptionalType<enumAttributeType>&			Type					() const { return (m_Type);									}
	//				inline SourceFilterItertor										SourceFilters			() const { return (SourceFilterItertor(m_SourceFilters));	}

				private:
					void ParseLine (const Generics::TextFragment& lines);
					void ParseRTPMapAttribute(const Generics::TextFragment& lines);
					void ParseControlAttribute(const Generics::TextFragment& lines);
					void ParseRangeAttribute(const Generics::TextFragment& lines);
					void ParseFMTPAttribute(const Generics::TextFragment& lines);
					void ParseXDimensionAttribute(const Generics::TextFragment& lines);
					void ParseFramerateAttribute(const Generics::TextFragment& lines);
					void ParseXFramerateAttribute(const Generics::TextFragment& lines);
					void ParseSourceFilterAttribute(const Generics::TextFragment& lines);
					void ParseTypeAttribute (const Generics::TextFragment& lines);

				private:
					Generics::OptionalType<uint8>						m_PayloadFormat;
					Generics::OptionalType<uint32>						m_TimestampFrequency;
					Generics::OptionalType<uint32>						m_Channels;
					Generics::OptionalType<Generics::TextFragment>		m_Codec;
					Generics::OptionalType<Generics::TextFragment>		m_ControlPath;
					Generics::OptionalType<double>						m_PlayStartTime;
					Generics::OptionalType<double>						m_PlayEndTime;
					Generics::OptionalType<uint16>						m_VideoWidth;
					Generics::OptionalType<uint16>						m_VideoHeight;
					Generics::OptionalType<uint32>						m_FrameRate;
					Generics::OptionalType<uint8>						m_CRC;
					Generics::OptionalType<uint8>						m_OctetAlign;
					Generics::OptionalType<uint8>						m_CPresent;
					Generics::OptionalType<uint8>						m_RobustSorting;
					Generics::OptionalType<uint8>						m_RandomAccessIndication;
					Generics::OptionalType<uint32>						m_ConstantDuration;
					Generics::OptionalType<uint32>						m_ConstantSize;
					Generics::OptionalType<uint32>						m_CTSDeltaLength;
					Generics::OptionalType<uint32>						m_DTSDeltaLength;
					Generics::OptionalType<uint32>						m_IndexLength;
					Generics::OptionalType<uint32>						m_InterLeaving;
					Generics::OptionalType<uint32>						m_MaxDisplacement;
					Generics::OptionalType<uint32>						m_ObjectType;
					Generics::OptionalType<uint32>						m_ProfileLevelID;
					Generics::OptionalType<uint32>						m_SizeLength;
					Generics::OptionalType<uint32>						m_StreamStateIndication;
					Generics::OptionalType<uint32>						m_StreamType;
					Generics::OptionalType<uint32>						m_InterleavedBufferSize;
					Generics::OptionalType<Generics::TextFragment>		m_Config;
					Generics::OptionalType<Generics::TextFragment>		m_Mode;
					Generics::OptionalType<Generics::TextFragment>		m_PropParameterSets;
					Generics::OptionalType<enumAttributeType>			m_Type;
					std::list<Generics::TextFragment>					m_SourceFilters;
			};

			typedef enum enumProtocolType
			{
				RTP,
				UDP
			};

		private:
			Message(Message& Parent);
			Message& operator= (Message& Parent);

		public:
			Message(const TCHAR text[]);
			Message(const String& text);
			~Message();

		public:
			inline const Attributes&										Attribute			() const { return (m_Attributes);			}
			inline const Generics::OptionalType<Generics::TextFragment>&	MediumName			() const { return (m_MediumName);			}
			inline const Generics::OptionalType<Generics::TextFragment>&	EndPoint			() const { return (m_EndPoint);				}
			inline const Generics::OptionalType<Generics::TextFragment>&	SessionName			() const { return (m_SessionName);			}
			inline const Generics::OptionalType<Generics::TextFragment>&	SessionDescription	() const { return (m_SessionDescription);	}
			inline const Generics::OptionalType<uint32>&					Bandwidth			() const { return (m_Bandwidth);			}
			inline const Generics::OptionalType<enumProtocolType>&			ProtocolType		() const { return (m_ProtocolType);			}
			inline const Generics::OptionalType<uint8>&						PayLoadFormat		() const { return (m_PayLoadFormat);		}
			inline const Generics::OptionalType<uint16>&					ClientPort			() const { return (m_ClientPort);			}
			inline const Generics::OptionalType<uint8>&						NumberOfPorts		() const { return (m_NumberOfPorts);		}

		private:
			void Parse ();
			uint32 GuessTimestampFrequency(const TCHAR mediumName[], const TCHAR codecName[]);
			void Attach(Message* subsession);
			void Detach(Message* subsession);
			Message* ParseLine (TCHAR command, const TCHAR description[]);
			Message* CreateSubSession();

			void Parse_M_Line (const Generics::TextFragment& lines);
			void Parse_C_Line (const Generics::TextFragment& lines);
			void Parse_S_Line (const Generics::TextFragment& lines);
			void Parse_I_Line (const Generics::TextFragment& lines);
			void Parse_B_Line (const Generics::TextFragment& lines);
			void Parse_A_Line (const Generics::TextFragment& lines);

		private:
			String											m_Information;
			Message*										m_Parent;
			Message*										m_SubSessions;
			Attributes										m_Attributes;
			Generics::OptionalType<Generics::TextFragment>	m_MediumName;
			Generics::OptionalType<Generics::TextFragment>	m_EndPoint;
			Generics::OptionalType<Generics::TextFragment>	m_SessionName;
			Generics::OptionalType<Generics::TextFragment>	m_SessionDescription;
			Generics::OptionalType<uint32>					m_Bandwidth;
			Generics::OptionalType<enumProtocolType>		m_ProtocolType;
			Generics::OptionalType<uint8>					m_PayLoadFormat;
			Generics::OptionalType<uint16>					m_ClientPort;
			Generics::OptionalType<uint8>					m_NumberOfPorts;
	};

} } // namespace Solutions::SDP

#endif // __SDPMESSAGE_H