#ifndef __SINKFACTORY_H
#define __SINKFACTORY_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Module.h"
#include "ISink.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----

// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace Sink
{
	template <typename SINK>
	class FactoryType : public ISinkFactory
	{
		private:
			FactoryType(const FactoryType<SINK>&);
			FactoryType<SINK>& operator= (const FactoryType<SINK>&);

		public:
			FactoryType()
			{
			}

			virtual ~FactoryType()
			{
			}


			virtual bool Supported(const Source::ISource* source) const
			{
				return (SINK::Supported(source));
			}

			virtual ISink* Create(const Sink::EnumSinkType type, const Source::ISource* source, Generics::NodeId& linkPoint)
			{
				return (type == SINK::Type() ? SINK::Create(source, linkPoint) : NULL);
			}

			virtual Sink::EnumSinkType Type () const
			{
				return (SINK::Type());
			}	
	};

	class EXTERNAL Factory
	{
		public:
			class EXTERNAL Iterator
			{
				private:
					Iterator& operator= (const Iterator&  RHS);

				public:
					Iterator();
					Iterator(const std::list<ISinkFactory*>& container, const Source::ISource* type);
					Iterator(const Iterator& copy);
					~Iterator();

				public:
					bool IsValid() const;
					bool Next ();
					inline ISinkFactory* Factory() { return (m_Selected); }

				private:
					ISinkFactory*									m_Selected;
					const Source::ISource*							m_Type;
					const std::list<ISinkFactory*>*					m_Container;
					std::list<ISinkFactory*>::const_iterator		m_Iterator;
			};

		private:
			Factory(const Factory&);
			Factory& operator= (const Factory&);

		public:
			Factory();
			~Factory();

		public:	
			void Announce(ISinkFactory* factory);
			ISink* Create (const Sink::EnumSinkType type, const Source::ISource* source, Generics::NodeId& toPoint);
			Factory::Iterator Sinks(const Source::ISource* type) const;

		private:
			std::list<ISinkFactory*>	m_FactoryDefinitions;
	};

} } // namespace Solutions::Sink

#endif // __SINKFACTORY_H