#ifndef __STREAMER_H
#define __STREAMER_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "ISource.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

namespace Solutions { namespace Source
{
	template <typename BASEITERATOR>
	class SourceType : public ISource
	{
		private:
			SourceType<BASEITERATOR>& operator= (const SourceType<BASEITERATOR>&);

		public:
			inline SourceType (const BASEITERATOR& indexIterator) :
				m_DataIterator (indexIterator)
			{
			}
			inline SourceType(const SourceType<BASEITERATOR>& copy) :
				m_DataIterator(copy.m_DataIterator)
			{
			}
			virtual ~SourceType ()
			{
			}

		public:
			virtual uint64 PresentationTime   () const
			{
				return (m_DataIterator.PresentationTime ());
			}

			virtual void PresentationTime (const uint64 tickLocation)
			{
				if (tickLocation < m_DataIterator.PresentationTime())
				{
					// move back to the beginning of the stream so we can move tp the requested time.
					m_DataIterator.Reset();
				}

				while ( (tickLocation < m_DataIterator.PresentationTime()) && (m_DataIterator.Next() == true) )
				{
					// // Initentionally left empty, we stept forward maybe we are 
					// now on the requested location
				}
			}

			virtual uint32 NextPackage (Generics::DataElementContainer& framePackage)
			{
				uint32 result = NUMBER_MAX_UNSIGNED(uint32);

				if (m_DataIterator.Next())
				{
					result = m_DataIterator.Package(framePackage);
				}

				return (result);
			}

			virtual const IProperties* Properties () const
			{
				return (m_DataIterator.Properties());
			}

		private:
			BASEITERATOR	m_DataIterator;
	};

	template <typename DATASOURCE, typename SPECIFICSTREAMER, typename SPECIFICITERATOR>
	class SingleSourceType : public ISources
	{
		private:
			SingleSourceType();
			SingleSourceType(const SingleSourceType<DATASOURCE, SPECIFICSTREAMER, SPECIFICITERATOR>&);
			SingleSourceType<DATASOURCE, SPECIFICSTREAMER, SPECIFICITERATOR> operator= (const SingleSourceType<DATASOURCE, SPECIFICSTREAMER, SPECIFICITERATOR>&);

		public:
			SingleSourceType(ISourceFactory* creator, const Generics::URL& identifier) :
				m_Creator	  (creator),
				m_Source	  (identifier),
				m_Parser	  (m_Source),
				m_Index		  (0)
			{
			}
			template <typename arg1>
			inline SingleSourceType(ISourceFactory* creator, const Generics::URL& identifier, arg1 a_Argument1) :
				m_Creator	  (creator),
				m_Source	  (identifier, a_Argument1),
				m_Parser	  (m_Source),
				m_Index		  (0)
			{
			}
			template <typename arg1, typename arg2>
			inline SingleSourceType(ISourceFactory* creator, const Generics::URL& identifier, arg1 a_Argument1, arg2 a_Argument2) :
				m_Creator	  (creator),
				m_Source	  (identifier, a_Argument1, a_Argument2),
				m_Parser	  (m_Source),
				m_Index		  (0)
			{
			}
			template <typename arg1, typename arg2, typename arg3>
			inline SingleSourceType(ISourceFactory* creator, const Generics::URL& identifier, arg1 a_Argument1, arg2 a_Argument2, arg3 a_Argument3) :
				m_Creator	  (creator),
				m_Source	  (identifier, a_Argument1, a_Argument2, a_Argument3),
				m_Parser	  (m_Source),
				m_Index		  (0)
			{
			}
			template <typename arg1, typename arg2, typename arg3, typename arg4>
			inline SingleSourceType(ISourceFactory* creator, const Generics::URL& identifier, arg1 a_Argument1, arg2 a_Argument2, arg3 a_Argument3, arg4 a_Argument4) :
				m_Creator	  (creator),
				m_Source	  (identifier, a_Argument1, a_Argument2, a_Argument3, a_Argument4),
				m_Parser	  (m_Source),
				m_Index		  (0)
			{
			}
			virtual ~SingleSourceType ()
			{
			}

		public:
			virtual const Generics::OptionalType<Generics::URL> Content () const
			{
				return (m_Source.SourceId());
			}

			virtual const TCHAR* Parser () const
			{
				return &(typeid (m_Parser).name ()[6]);
			}

			virtual uint64 Duration	() const
			{
				return (m_Parser.Duration ());
			}

			virtual void Reset ()
			{
				// Prior, to the source, source is index 1
				m_Index = 0;
			}

			virtual bool Next ()
			{
				if (m_Index < 2)
				{
					m_Index++;
				}

				// Prior, to the source, source is index 1
				return ((m_Index == 1) && (m_Source.IsValid()));
			}

			virtual ISource* Source ()
			{
				return (m_Index == 1 ? new SPECIFICITERATOR(m_Parser.Iterator()) : NULL);
			}

			virtual ISourceFactory* Factory()
			{
				return (m_Creator);
			}

			inline static bool IsValidSource (const Generics::URL& sourceId)
			{
				return (DATASOURCE::IsValidSource(sourceId));
			}

		private:
			ISourceFactory*			m_Creator;
			DATASOURCE				m_Source;
			SPECIFICSTREAMER		m_Parser;
			uint32					m_Index;
	};

	template <typename DATASOURCE, typename SPECIFICSTREAMER, typename SPECIFICITERATOR>
	class MultipleSourceType : public ISources
	{
		private:
			MultipleSourceType(const MultipleSourceType<DATASOURCE, SPECIFICSTREAMER,SPECIFICITERATOR>&);
			MultipleSourceType<DATASOURCE, SPECIFICSTREAMER,SPECIFICITERATOR> operator= (const MultipleSourceType<DATASOURCE, SPECIFICSTREAMER,SPECIFICITERATOR>&);

		public:
			inline MultipleSourceType(ISourceFactory* creator, const Generics::URL& identifier) :
				m_Creator	  (creator),
				m_Source	  (identifier),
				m_Parser	  (m_Source)
			{
			}
			template <typename arg1>
			inline MultipleSourceType(ISourceFactory* creator, const Generics::URL& identifier, arg1 a_Argument1) :
				m_Creator	  (creator),
				m_Source	  (identifier, a_Argument1),
				m_Parser	  (m_Source)
			{
			}
			template <typename arg1, typename arg2>
			inline MultipleSourceType(ISourceFactory* creator, const Generics::URL& identifier, arg1 a_Argument1, arg2 a_Argument2) :
				m_Creator	  (creator),
				m_Source	  (identifier, a_Argument1, a_Argument2),
				m_Parser	  (m_Source)
			{
			}
			template <typename arg1, typename arg2, typename arg3>
			inline MultipleSourceType(ISourceFactory* creator, const Generics::URL& identifier, arg1 a_Argument1, arg2 a_Argument2, arg3 a_Argument3) :
				m_Creator	  (creator),
				m_Source	  (identifier, a_Argument1, a_Argument2, a_Argument3),
				m_Parser	  (m_Source)
			{
			}
			template <typename arg1, typename arg2, typename arg3, typename arg4>
			inline MultipleSourceType(ISourceFactory* creator, const Generics::URL& identifier, arg1 a_Argument1, arg2 a_Argument2, arg3 a_Argument3, arg4 a_Argument4) :
				m_Creator	  (creator),
				m_Source	  (identifier, a_Argument1, a_Argument2, a_Argument3, a_Argument4),
				m_Parser	  (m_Source)
			{
			}
			virtual ~MultipleSourceType ()
			{
			}

		public:
			virtual const Generics::OptionalType<Generics::URL> Content		() const
			{
				return (m_Source.SourceId());
			}

			virtual const TCHAR* Parser () const
			{
				return &(typeid (m_Parser).name ()[6]);
			}

			virtual uint64 Duration				() const
			{
				return (m_Parser.Duration());
			}

			virtual void Reset ()
			{
				m_Parser.Reset();
			}

			virtual bool Next ()
			{				
				return (m_Parser.Next());
			}

			virtual ISource* Source ()
			{
				return (new SPECIFICITERATOR(m_Parser.Iterator()));
			}

			virtual ISourceFactory* Factory()
			{
				return (m_Creator);
			}

			inline static bool IsValidSource (const Generics::URL& sourceId)
			{
				return (DATASOURCE::IsValidSource(sourceId));
			}

		private:
			ISourceFactory*			m_Creator;
			DATASOURCE				m_Source;
			SPECIFICSTREAMER		m_Parser;
	};


} } // namespace Solutions::Source

#endif // __STREAMER_H