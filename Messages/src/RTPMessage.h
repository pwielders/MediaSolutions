#ifndef __RTPHEADER_H
#define __RTPHEADER_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----
const uint8	RTP_MINIMUM_HEADER_SIZE	  = 12;
const uint8 RTP_HEADER_EXTENSION_SIZE = 16;
const uint8	RTP_VERSION				  = 2;

namespace Solutions { namespace RTP
{
	// ---- Class Definition ----
	class EXTERNAL Message
	{
		private:
			Message();
			Message(const Message& copy);
			Message& operator= (const Message& RHS);

		public:
			//Message(Generics::DataElement& buffer);
			Message(const uint8 version);
			~Message ();

		public:
			const Generics::DataElement Data() const;
			uint32 Size () const;

			bool HasPadding () const;
			void HasPadding (const bool padding);

			bool HasExtension () const;
			void HasExtension (const bool extension);

			bool HasMarker () const;
			void HasMarker (const bool marker);

			uint8 Version () const;
			void Version (const uint8 version);

			uint8 ContributingSourceCount () const;

			uint8 PayLoadType () const;
			void PayLoadType (const uint8 type);

			uint16 Sequence () const;
			void Sequence (const uint16 sequenceNumber);

			uint32 TimeStamp () const;
			void TimeStamp (const uint32 timeStamp);

			uint32 SessionSource () const;
			void SessionSource (const uint32 sourceId);

			uint32 ContributingSourceEntry (const uint8 index) const;
			void AddContributingSourceEntry (const uint32 contributingSource);

			const Generics::DataElement Extension () const;
			void Extension (const Generics::DataElement& data);

			inline uint32 HeaderSize () const
			{
				return (m_MaximumHeaderSize);
			}

		private:
			Generics::DataElement	m_Buffer;
			uint32					m_ExtensionOffset;
			uint32					m_ExtensionSize;
			uint32					m_MaximumHeaderSize;
	};

} } // namespace Solutions::RTP

#endif // __RTPHEADER_H