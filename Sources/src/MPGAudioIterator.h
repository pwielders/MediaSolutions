#ifndef __MPGAUDIOITERATOR_H
#define __MPGAUDIOITERATOR_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "ISource.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace Source
{
	class EXTERNAL MPGAudioIterator
	{
		private:
			class Info : public IProperties
			{
				private:
					Info& operator= (const Info&);

				public:
					Info() : m_Iterator(NULL)
					{
					}
					Info(const MPEG::MPEGAudio& iterator) : m_Iterator(&iterator)
					{
					}
					Info(const Info& copy) : m_Iterator (copy.m_Iterator)
					{
					}
					virtual ~Info()
					{
					}

				public:
					// Determines the type of this source.
					virtual EnumSourceType SourceType () const
					{
						return (MPEG_STREAM);
					}

					virtual EnumStreamType StreamType () const
					{
						return (AUDIO);
					}

					// The number of samples for each channel per second
					virtual uint32 BitRate () const
					{
						return ((m_Iterator == NULL) ?  0 : m_Iterator->BitRate());
					}

				private:
					const MPEG::MPEGAudio* m_Iterator;
			};

		private:
			MPGAudioIterator operator= (const MPGAudioIterator&);

		public:
			MPGAudioIterator () :
				m_Iterator   (),
				m_Properties ()
			{
			}
			inline MPGAudioIterator (const MPEG::MPEGAudio& data) :
				m_Iterator   (data),
				m_Properties (data)
			{
			}
			inline MPGAudioIterator (const MPGAudioIterator& copy) :
				m_Iterator   (copy.m_Iterator),
				m_Properties (copy.m_Properties)
			{
			}
			inline ~MPGAudioIterator ()
			{
			}

		public:
			// Move on to the next sample/picture/frame
			void Reset ()
			{
				m_Iterator.Reset();
			}
					
			// This is the time that the current appointed data should be presented,
			// relative to the time that the first data was aquired in milliseconds.
			inline uint64 PresentationTime () const
			{
				return (m_Iterator.PlayTime()); // Move to MilliSeconds
			}

			// Move on to the next sample/picture/frame
			bool Next ()
			{
				return (m_Iterator.Next());
			}
					
			// This indicates some data that represent a set of samples/pictures/frames.
			inline uint32 Package (Generics::DataElementContainer& package)
			{
				Generics::DataElement frame (m_Iterator.Frame().Data());

				// We have a valid package...
				package.Back(frame);

				return (static_cast<uint32>(frame.Size()));
			}

			inline const IProperties* Properties() const
			{
				return(&m_Properties);
			}

		private:
			MPEG::MPEGAudio			m_Iterator;
			Info					m_Properties;
	};


} } // namespace Solutions::Source

#endif // __MPGAUDIOITERATOR_H