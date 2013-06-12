#ifndef __MPGITERATOR_H
#define __MPGITERATOR_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "MPGAudioIterator.h"
#include "MPGVideoIterator.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace Source
{
	class EXTERNAL MPGIterator
	{
		private:
			MPGIterator operator= (const MPGIterator&);

		public:
			MPGIterator () : m_Video (), m_Audio ()
			{
			}
			MPGIterator (const MPGAudioIterator& data) : m_StreamType(Source::AUDIO), m_Audio(data), m_Video ()
			{
			}
			MPGIterator (const MPGVideoIterator& data) : m_StreamType(Source::VIDEO), m_Video(data), m_Audio ()
			{
			}
			MPGIterator (const MPGIterator& copy) : m_StreamType(copy.m_StreamType), m_Video (m_Video), m_Audio (m_Audio)
			{
			}
			~MPGIterator ()
			{
			}

		public:
			void Reset ()
			{
				switch (m_StreamType)
				{
					case Source::AUDIO:
						 m_Audio.Reset ();
						 break;
					case Source::VIDEO:
						 m_Video.Reset ();
						 break;
					default:
						 ASSERT (false);
				}
			}

			inline uint64 PresentationTime () const
			{
				return (m_StreamType == Source::AUDIO ? m_Audio.PresentationTime () : 
					    m_StreamType == Source::VIDEO ? m_Video.PresentationTime () : 0);
			}

			inline bool Next ()
			{
				return (m_StreamType == Source::AUDIO ? m_Audio.Next () : 
					    m_StreamType == Source::VIDEO ? m_Video.Next () : NULL);
			}
					
			inline uint32 MPGIterator::Package (Generics::DataElementContainer& package)
			{
				return (m_StreamType == Source::AUDIO ? m_Audio.Package (package) : 
					    m_StreamType == Source::VIDEO ? m_Video.Package (package) : NULL);
			}

			inline const IProperties* Properties() const
			{
				return (m_StreamType == Source::AUDIO ? m_Audio.Properties () : 
					    m_StreamType == Source::VIDEO ? m_Video.Properties () : NULL);
			}

		private:
			Source::EnumStreamType	m_StreamType;
			MPGVideoIterator		m_Video;
			MPGAudioIterator		m_Audio;
	};


} } // namespace Solutions::Source

#endif // __MPGITERATOR_H