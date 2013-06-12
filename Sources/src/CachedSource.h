#ifndef __CACHEDSTREAMER_H
#define __CACHEDSTREAMER_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "ISource.h"
#include "Source.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----
namespace Solutions { namespace Source
{
	class EXTERNAL FrameElement
	{
		public:
			FrameElement () :
				m_BitRate (0),
				m_Element ()
			{
			}
			FrameElement (const Generics::DataElement& element, const uint32 bitRate) :
				m_BitRate (bitRate),
				m_Element (element)
			{
			}
			FrameElement (const FrameElement& copy) :
				m_BitRate (copy.m_BitRate),
				m_Element (copy.m_Element)
			{
			}
			~FrameElement()
			{
			}

			FrameElement& operator= (const FrameElement& RHS)
			{
				m_BitRate = RHS.m_BitRate;
				m_Element = RHS.m_Element;

				return (*this);
			}

		public:
			inline uint32 BitRate () const
			{
				return (m_BitRate);
			}

			inline const Generics::DataElement& Element () const
			{
				return (m_Element);
			}

		private:
			uint32					m_BitRate;
			Generics::DataElement	m_Element;
	};

	template <typename SPECIFICSTREAMER, const uint16 LENGTH>
	class CachedType : public ISource
	{
		private:
			template <typename SPECIFICSTREAMER>
			class IteratorWrapper
			{
				private:
					IteratorWrapper(const IteratorWrapper&);
					IteratorWrapper& operator= (const IteratorWrapper&);

				public:
					IteratorWrapper(typename SPECIFICSTREAMER::IteratorType& iterator) :
						m_RealIterator (iterator)
					{
					}
					~IteratorWrapper()
					{
					}

				public:
					bool First(Generics::SlotInfoType<FrameElement>& element)
					{
						// Start at the beginning..
						m_RealIterator.PresentationTime(0);

						// Now Call the next functionality;
						return (Next(element));
					}
					bool Next(Generics::SlotInfoType<FrameElement>& element)
					{
						bool succeeded = m_RealIterator.Next();
						
						if (succeeded == true)
						{
							FrameElement newElement (m_RealIterator.Data(), m_RealIterator.BitRate());
							element = SlotInfoType<FrameElement>(m_RealIterator.PresentationTime(), m_RealIterator.Duration(), newElement);
						}

						return (succeeded);
					}

				private:
					typename SPECIFICSTREAMER::IteratorType		m_RealIterator;
			};

			class Info : public IProperties
			{
				private:
					Info();
					Info& operator= (const Info&);

				public:
					Info(const Generics::SlottedStorageIterator<IteratorWrapper<SPECIFICSTREAMER>,FrameElement,LENGTH>& iterator, const EnumSourceType source, const EnumStreamType type) : m_Iterator(iterator), m_SourceType(source), m_StreamType(type)
					{
					}
					Info(const Info& copy) : m_Iterator (copy.m_Iterator), m_SourceType(copy.m_SourceType), m_StreamType(copy.m_StreamType)
					{
					}
					virtual ~Info()
					{
					}

				public:
					// Determines the type of this source.
					virtual EnumSourceType SourceType () const
					{
						return (m_SourceType);
					}

					virtual EnumStreamType StreamType () const
					{
						return (m_StreamType);
					}

					virtual uint32 BitRate () const
					{
						return (m_DataIterator.Data().BitRate());
					}

				private:
					ISource::EnumStreamType		m_Streamer;
					ISource::EnumSourceType		m_SourceType;
					Generics::SlottedStorageIterator<IteratorWrapper<SPECIFICSTREAMER>,FrameElement,LENGTH>&	m_DataIterator;
			};

		private:
			CachedType(CachedType<SPECIFICSTREAMER,LENGTH>&);
			CachedType<SPECIFICSTREAMER,LENGTH>& operator= (const CachedType<SPECIFICSTREAMER,LENGTH>&);

		public:
			CachedType (SPECIFICSTREAMER& policy, Generics::SlottedStorage<FrameElement,LENGTH>& container, const uint8 channel) :
				m_Channel(channel),
				m_Streamer(policy),
				m_Wrapper(policy.Iterator(channel)),
				m_DataIterator(m_Wrapper, container),
				m_Progress(true)
			{
			}

			virtual ~CachedType ()
			{
			}

		public:
			virtual uint64 PresentationTime   () const
			{
				return (m_DataIterator.BeginSlot ());
			}

			virtual void PresentationTime (const uint64 tickLocation)
			{
				m_DataIterator.SetToSlot(tickLocation);

				m_Progress = true;
			}

			virtual uint32 NextFrame (Generics::DataElementContainer& framePackage)
			{
				uint32 result = NUMBER_MAX_UNSIGNED(uint32);

				if (m_Progress == true)
				{
					m_Progress = !m_DataIterator.Next();
				}
				
				// If we stopped on this location, we have data to distribute
				if (m_Progress == false)
				{
					if (framePackage.PackageSize() > framePackage.Space())
					{
						result = 0;
					}
					else
					{
						DataElement newFrame (m_DataIterator.Frame());
						framePackage.Back(newFrame);
						result = newFrame.Size();
						m_Progress = true;
					}
				}

				return (result);

				return (LinkedDataElement (m_DataIterator.Data().Element()));
			}

			virtual const IProperties* Properties () const
			{
				return (&m_Properties);
			}

		private:
			bool																					m_Progress;
			uint8																					m_Channel;
			SPECIFICSTREAMER&																		m_Streamer;
			IteratorWrapper<SPECIFICSTREAMER>														m_Wrapper;
			Generics::SlottedStorageIterator<IteratorWrapper<SPECIFICSTREAMER>,FrameElement,LENGTH>	m_DataIterator;
			Info																					m_Properties;
	};

	template <typename DATASOURCE, typename SPECIFICSTREAMER, typename SPECIFICITERATOR, const uint32 LENGTH>
	class CachedSourcesType : public SingleSourceType<DATASOURCE, SPECIFICSTREAMER, SPECIFICITERATOR>
	{
		private:
			CachedSourcesType();
			CachedSourcesType(const CachedSourcesType<DATASOURCE, SPECIFICSTREAMER, SPECIFICITERATOR, LENGTH>&);
			CachedSourcesType<DATASOURCE, SPECIFICSTREAMER, SPECIFICITERATOR, LENGTH> operator= (const CachedSourcesType<DATASOURCE, SPECIFICSTREAMER, SPECIFICITERATOR, LENGTH>&);

			typedef SingleSourceType<DATASOURCE, SPECIFICSTREAMER, SPECIFICITERATOR> BaseClass;

		public:
			inline CachedSourcesType (const Generics::TextFragment& identifier) :
				BaseClass				  (identifier)
				m_Container				  ()
			{
			}
			template <typename arg1>
			inline CachedSourcesType (const Generics::TextFragment& identifier, arg1 a_Argument1) :
				BaseClass				  (identifier, a_Argument1),
				m_Container				  ()
			{
			}
			template <typename arg1, typename arg2>
			inline CachedSourcesType (const Generics::TextFragment& identifier, arg1 a_Argument1, arg2 a_Argument2) :
				BaseClass			(identifier, a_Argument1, a_Argument2),
				m_Container			()
			{
			}
			template <typename arg1, typename arg2, typename arg3>
			inline CachedSourcesType (const Generics::TextFragment& identifier, arg1 a_Argument1, arg2 a_Argument2, arg3 a_Argument3) :
				BaseClass			(identifier, a_Argument1, a_Argument2, a_Argument3),
				m_Container			()
			{
			}
			template <typename arg1, typename arg2, typename arg3, typename arg4>
			inline CachedSourcesType (const Generics::TextFragment& identifier, arg1 a_Argument1, arg2 a_Argument2, arg3 a_Argument3, arg4 a_Argument4) :
				BaseClass			(identifier, a_Argument1, a_Argument2, a_Argument3, a_Argument4),
				m_Container			()
			{
			}
			virtual ~CachedSourcesType ()
			{
			}

		public:
			virtual ISource* Source (const uint8 channelId)
			{
				return (channelId < Channels() ? new CachedType<SPECIFICSTREAMER,LENGTH>(BaseClass::Parser(), m_Container, channelId) : NULL);
			}

		private:
			Generics::SlottedStorage<FrameElement,LENGTH>		m_Container;
	};

} } // namespace Solutions::Source

#endif // __CACHEDSTREAMER_H