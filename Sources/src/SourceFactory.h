#ifndef __SOURCEFACTORY_H
#define __SOURCEFACTORY_H

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
	template <typename SESSIONSOURCE>
	class FactoryType : public ISourceFactory
	{
		private:
			FactoryType(const FactoryType<SESSIONSOURCE>&);
			FactoryType<SESSIONSOURCE>& operator= (const FactoryType<SESSIONSOURCE>&);

		public:
			FactoryType()
			{
			}

			~FactoryType()
			{
			}

			virtual bool Supported(const Generics::URL& identification) const
			{
				return (SESSIONSOURCE::IsValidSource(identification));
			}

			virtual Generics::ProxyType<ISources> Create(const Generics::URL& identification)
			{
				Generics::ProxyType<ISources> result;

				if (SESSIONSOURCE::IsValidSource(identification) == true)
				{
					result = Generics::proxy_cast<ISources> (Generics::ProxyType<SESSIONSOURCE>::Create(this, identification));
				}
				
				return (result);
			}
	};

	class EXTERNAL Factory
	{
		friend class Generics::TimerContextType<Factory*>;

		private:
			static const uint32 PERIODIC_CLEANUP_TIME = (5 * 60);	// Every 5 minutes !!! 

		private:
			Factory(const Factory&);
			Factory& operator= (const Factory&);

		public:
			Factory();
			~Factory();

		public:
			void Announce(ISourceFactory* factory);
			Generics::ProxyType<ISources> Factory::Create (const Generics::URL& identfier);

		private:
			void Timed ();

		private:
			Generics::CriticalSection									m_Admin;
			Generics::TimerType<Generics::TimerContextType<Factory*>>	m_CleanupTask;
			std::list<ISourceFactory*>									m_FactoryDefinitions;
			std::map<String, Generics::ProxyType<ISources>>				m_AvailableSources;
	};

} } // namespace Solutions::Source

#endif // __SOURCEFACTORY_H