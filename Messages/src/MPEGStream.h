#ifndef __MPEGSTREAM_H
#define __MPEGSTREAM_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "MPEGHeader.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace MPEG
{
	class EXTERNAL MPEGStream
	{	
		public:
			MPEGStream ();
			MPEGStream (const MPEGStream& copy);
			MPEGStream (const Generics::DataElement& buffer, const uint64 offset, const uint8 streamId = 0);
			MPEGStream (const Generics::DataElement& buffer, const uint64 offset, const uint16 pid, const uint8 frameSize);
			MPEGStream (const Generics::DataElement& buffer, const uint64 offset, const uint64 referenceClock, const uint64 wallClock);
			inline ~MPEGStream() {}

			MPEGStream& operator= (const MPEGStream& RHS);

		public:
			inline uint64 WallClock () const
			{
				return (m_WallClock);
			}
			inline uint64 ReferenceClock () const
			{
				return (m_RefClock);
			}
			inline Generics::DataElement Section ()
			{
				return (m_Current);
			}

			void Reset();
			bool Next();

		private:
			bool FindNextElementaryStreamFromStream ();
			bool FindNextElementaryStreamFromBuffer ();

		private:
			Generics::DataElement	m_Buffer;
			Generics::DataElement	m_Current;
			uint64					m_Offset;
			uint8					m_StreamId;
			uint16					m_Pid;
			uint64					m_RefClock;
			uint64					m_WallClock;
	};

} } // namespace Solutions::MPEG

#endif // __MPEGSTREAM_H