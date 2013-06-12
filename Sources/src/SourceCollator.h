#ifndef __FRAMECOLLATOR_H
#define __FRAMECOLLATOR_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "ISource.h"
#include "Source.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----

namespace Solutions { namespace Source
{
	typedef enum
	{
		MULTIPLE_FRAMES			= 0x01,
		FRAGMENTED_FRAMES		= 0x02,
		REJECT_OVERSIZED_FRAMES = 0x04

	}	CollatorCharacteristics;

	class EXTERNAL Collator : public ISource
	{
		private:
			Collator ();
			Collator(const Collator&);
			Collator& operator= (const Collator&);

		public:
			Collator (const uint32 type, ISource* realStream);
			virtual ~Collator ();

		public:
			virtual uint64 PresentationTime () const;
			virtual void PresentationTime (const uint64 tickLocation);
			virtual uint32 NextPackage (Generics::DataElementContainer& framePackage);
			virtual const IProperties* Properties () const;

		private:
			bool LoadNewPackage(const uint64 maxPackageSize);

		private:
			ISource*						m_DataStream;
			uint64							m_BitRate;
			uint32							m_CollatedFrames;
			uint32							m_Type;
			Generics::DataElement			m_Fraction;
			uint32							m_FractionIndex;
			Generics::DataElementContainer	m_FractionContainer;
	};

	template <typename BASEITERATOR, const uint32 COLLATORTYPE>
	class CollatorType : public Collator
	{
		private:
			CollatorType ();
			CollatorType(const CollatorType<BASEITERATOR,COLLATORTYPE>&);
			CollatorType<BASEITERATOR,COLLATORTYPE>& operator= (const CollatorType<BASEITERATOR,COLLATORTYPE>&);

		public:
			CollatorType (const BASEITERATOR& indexIterator) :
			    Collator(COLLATORTYPE, &m_BaseSource),
				m_BaseSource(indexIterator)
			{
			}
			virtual ~CollatorType ()
			{
			}

		public:
			inline uint64 PresentationTime () const
			{
				return (Collator::PresentationTime());
			}

			inline uint32 NextPackage (Generics::DataElementContainer& framePackage)
			{
				return (Collator::NextPackage(framePackage));
			}

			inline void PresentationTime (const uint64 tickLocation)
			{
				Collator::PresentationTime(tickLocation);
			}

			inline const IProperties* Properties () const
			{
				return (Collator::Properties());
			}


		private:
			SourceType<BASEITERATOR>			m_BaseSource;
	};

} } // namespace Solutions::Source

#endif // __FRAMECOLLATOR_H
	