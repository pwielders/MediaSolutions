#ifndef __MPG12TRANSPORT_H
#define __MPG12TRANSPORT_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "MPGAudioIterator.h"
#include "MPGVideoIterator.h"
#include "MPGIterator.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace Source
{
	class EXTERNAL MPG12Transport
	{
		private:
			MPG12Transport(const MPG12Transport&);
			MPG12Transport operator= (const MPG12Transport&);

		public:
			MPG12Transport (const Generics::DataElement& dataObject);
			~MPG12Transport();

		public:
			//---------------------------------------------------------------
			// The following method returns a new pointer into the stream.
			//---------------------------------------------------------------
			inline void Reset ()
			{
				m_ChannelIterator.Reset();
			}

			inline bool Next ()
			{				
				return (m_ChannelIterator.Next());
			}

			inline MPGIterator Iterator ()
			{
				switch (m_ChannelIterator.Type())
				{
				case MPEG::AUDIO:	return (MPGIterator(MPGAudioIterator(m_ChannelIterator.Audio())));
									break;
				case MPEG::VIDEO:	return (MPGIterator(MPGVideoIterator(m_ChannelIterator.Video())));
									break;
				}

				// Make this is producer consumer algorithm. We produce, The receiver has to consume...
				return (MPGIterator());
			}

			inline bool IsValid() const
			{
				return (m_MPEGSource.IsValid());
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
			uint64						m_Duration;
			MPEG::MPEGSource			m_MPEGSource;
			MPEG::MPEGSource::Iterator	m_ChannelIterator;
	};

} } // namespace Solutions::Source

#endif // __MPG12TRANSPORT_H