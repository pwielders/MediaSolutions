#ifndef __MPG3SOURCE_H
#define __MPG3SOURCE_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "MPGAudioIterator.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace Source
{
	class EXTERNAL MPG3Source
	{
		private:
			MPG3Source(const MPG3Source&);
			MPG3Source operator= (const MPG3Source&);

		public:
			MPG3Source (const Generics::DataElement& dataObject);
			~MPG3Source();

		public:
			//---------------------------------------------------------------
			// The following method returns a new pointer into the stream.
			//---------------------------------------------------------------
			MPGAudioIterator Iterator ()
			{
				return (MPGAudioIterator(m_Data));
			}

			inline bool IsValid() const
			{
				return (m_Data.IsValid());
			}

			//---------------------------------------------------------------
			// The following methods are independent of the location
			// in the stream, no need to "store the states of these methods.
			//---------------------------------------------------------------
			uint64 Duration () const
			{
				return (m_Duration);
			}

		private:
			uint64 SkipID3Tag (const Generics::DataElement& data, const uint64 offset) const;

		private:
			MPEG::MPEGAudio		m_Data;
			uint64				m_Duration;
	};

} } // namespace Solutions::Source

#endif // __MPG3SOURCE_H