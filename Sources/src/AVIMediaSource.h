#ifndef __AVIMEDIASOURCE_H
#define __AVIMEDIASOURCE_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace Source
{
	class EXTERNAL AVIMedia
	{
		public:
			//---------------------------------------------------------------
			// Package AVIStream Interface (IteratorType)
			// 
			// Description: The classname (IteratorType) has to be defined
			//              within this class for the templates to know how
			//              packages can be extracted from this stream.
			//---------------------------------------------------------------
			class EXTERNAL AVIStream
			{
				private:
					class Info : public IProperties
					{
						private:
							Info();
							Info& operator= (const Info&);

						public:
							Info(const AVIStream& iterator, const EnumStreamType type) : m_Iterator(iterator), m_StreamType(type)
							{
							}
							Info(const Info& copy) : m_Iterator (copy.m_Iterator), m_StreamType(copy.m_StreamType)
							{
							}
							virtual ~Info()
							{
							}

						public:
							// Determines the type of this source.
							virtual EnumSourceType SourceType () const
							{
								return (AVI_MEDIA);
							}

							virtual EnumStreamType StreamType () const
							{
								return (m_StreamType);
							}

							// The number of samples for each channel per second
							virtual uint32 BitRate () const
							{
								return (0);
							}

						private:
							const AVIStream&		m_Iterator;
							EnumStreamType			m_StreamType;
					};

				private:
					AVIStream operator= (const AVIStream&);

				public:
					inline AVIStream () :
						m_Indexer(), m_Properties(*this, AUDIO)

					{
					}
					inline AVIStream (const AVI::PackageIterator indexer) : 
						m_Indexer(indexer), 
						m_Properties(*this, AUDIO)
					{
					}
					inline AVIStream (const AVIStream& copy) :
						m_Indexer (copy.m_Indexer),
						m_Properties(copy.m_Properties)
					{
					}

				public:
					// If we need to move back in time for setting the iterator, we need
					// some method to be at the beginning of the stream and then step forward
					// to the requested location. This method will set the iterator at the
					// beginning of the stream.
					inline void Reset ()
					{
						m_Indexer.Reset();
					}

					// This is the time that the current appointed data should be presented,
					// relative to the time that the first data was aquired in milliseconds.
					inline uint64 PresentationTime () const
					{
						return (m_Indexer.PresentationTime());
					}


					// Move on to the next sample/picture/frame
					inline bool Next ()
					{
						return (m_Indexer.Next());
					}
					
					// This indicates some data that represent a set of samples/pictures/frames.
					inline uint32 Package (Generics::DataElementContainer& package)
					{
						return (m_Indexer.Package(package));
					}

					inline const IProperties* Properties() const
					{
						return(&m_Properties);
					}

				private:
					AVI::PackageIterator	m_Indexer;
					Info					m_Properties;
			};

		private:
			AVIMedia(const AVIMedia&);
			AVIMedia operator= (const AVIMedia&);

		public:
			AVIMedia (const Generics::DataElement& dataObject);
			virtual ~AVIMedia();

		public:
			//---------------------------------------------------------------
			// The following method returns a new pointer into the stream.
			//---------------------------------------------------------------
			inline void Reset ()
			{
				m_Iterator.Reset();
			}

			inline bool Next ()
			{				
				return (m_Iterator.Next());
			}

			inline AVIStream Iterator ()
			{
				// Make this is producer consumer algorithm. We produce, The receiver has to consume...
				return (AVIStream(m_Iterator.GetStream().Packages()));
			}

			inline bool IsValid() const
			{
				return (m_Iterator.IsValid());
			}

			//---------------------------------------------------------------
			// The following methods are independent of the location
			// in the stream, no need to "store the states of these methods.
			//---------------------------------------------------------------
			uint64 Duration () const
			{
				return (0);
			}

		private:
			AVI::Main				m_MainHeader;
			AVI::StreamIterator		m_Iterator;
	};

} } // namespace Solutions::Source

#endif // __AVIMEDIASOURCE_H