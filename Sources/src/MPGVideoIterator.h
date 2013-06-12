#ifndef __MPGVIDEOITERATOR_H
#define __MPGVIDEOITERATOR_H

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
	//---------------------------------------------------------------
	// Package Iterator Interface (IteratorType)
	// 
	// Description: The classname (IteratorType) has to be defined
	//              within this class for the templates to know how
	//              packages can be extracted from this stream.
	//---------------------------------------------------------------
	class EXTERNAL MPGVideoIterator
	{
		private:
			class Info : public IVideoProperties
			{
				private:
					Info& operator= (const Info&);

				public:
					Info() : m_Iterator(NULL)
					{
					}
					Info(const MPEG::MPEGVideo& iterator) : m_Iterator(&iterator)
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
						return (VIDEO);
					}

					// Determines the number of frames available in this GOP.
					virtual uint16 GOPSize () const
					{
						return (0);
					}

					// Determines the index of the frame in GOP
					virtual uint16 FrameInGOP () const
					{
						return (m_Iterator->CurrentPictureHeader().SequenceNumber());
					}

					// The number of samples for each channel per second
					virtual uint32 BitRate () const
					{
						return (0);
					}

				private:
					const MPEG::MPEGVideo*		m_Iterator;
			};

		private:
			MPGVideoIterator operator= (const MPGVideoIterator&);

		public:
			MPGVideoIterator () :
				m_Iterator   (),
				m_Properties ()
			{
			}
			MPGVideoIterator (const MPEG::MPEGVideo& streamIterator) :
				m_Iterator   (streamIterator),
				m_Properties (streamIterator)
			{
			}
			MPGVideoIterator (const MPGVideoIterator& copy) :
				m_Iterator   (copy.m_Iterator),
				m_Properties (copy.m_Properties)
			{
			}
			~MPGVideoIterator()
			{
			}

		public:
			// Move on to the next sample/picture/frame
			void Reset ()
			{
				m_Iterator.Reset();
			}
			
			inline bool Next ()
			{
				return (m_Iterator.NextSlice());
			}

			// This is the time that the current appointed data should be presented,
			// relative to the time that the first data was aquired in milliseconds.
			inline uint64 PresentationTime () const
			{
				return (m_Iterator.PlayTime()); // Move to MilliSeconds
			}

			// This indicates some data that represent a set of samples/pictures/frames.
			inline uint32 Package (Generics::DataElementContainer& package)
			{
				Generics::DataElement frame (m_Iterator.CurrentSlice().Data());

				// We have a valid package...
				package.Back(frame);

				return (static_cast<uint32>(frame.Size()));
			}

			inline const IProperties* Properties() const
			{
				return(&m_Properties);
			}

		private:
			MPEG::MPEGVideo		m_Iterator;
			Info				m_Properties;
	};

} } // namespace Solutions::Source

#endif // __MPGVIDEOITERATOR_H