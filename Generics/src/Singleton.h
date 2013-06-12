#ifndef __SINGLETON_H
#define __SINGLETON_H

// ---- Include system wide include files ----
#include <list>

// ---- Include local include files ----
#include "Portability.h"
#include "Sync.h"

// ---- Referenced classes and types ----

// ---- Helper functions ----
namespace Solutions { namespace Generics
{
	class EXTERNAL Singleton
	{
		private:
			class EXTERNAL SingletonList
			{
				private:
					SingletonList(const SingletonList&);
					SingletonList& operator= (const SingletonList&);

				public:
					SingletonList();
					~SingletonList();
					void Register (Singleton* singleton);
					void Unregister (Singleton* singleton);
					void Dispose ();

				private:
					std::list<Singleton*>		m_Singletons;
			};

		private:
			Singleton(const Singleton&);
			Singleton& operator= (const Singleton&);

		public:
			inline Singleton::Singleton()
			{
				g_Singletons.Register(this);
			}

			virtual Singleton::~Singleton()
			{
				g_Singletons.Unregister(this);
			}

			inline static void Dispose ()
			{
				g_Singletons.Dispose();
			}

		public:
			static SingletonList	g_Singletons;
	};

	template<typename SINGLETON>
	class SingletonType : public Singleton
	{
		private:
			SingletonType (const SingletonType<SINGLETON>&);
			SingletonType<SINGLETON> operator= (const SingletonType<SINGLETON>&);

		public:
			SingletonType() : Singleton ()
			{
			}
			virtual ~SingletonType()
			{
			}

		public:
			static SINGLETON& Instance ()
			{
				if (g_TypedSingleton == NULL)
				{
					g_AdminLock.Lock();

					if (g_TypedSingleton == NULL)
					{
						// Create a singleton
						g_TypedSingleton = new SINGLETON();
					}

					g_AdminLock.Lock();
				}

				return (*g_TypedSingleton);
			}

		private:
			static SINGLETON*		g_TypedSingleton;
			static CriticalSection	g_AdminLock;
	};

	template<typename SINGLETON>
	SINGLETON* SingletonType<SINGLETON>::g_TypedSingleton = NULL;

	template<typename SINGLETON>
	CriticalSection SingletonType<SINGLETON>::g_AdminLock;

} } // namespace Solutions::Generics

#endif // __SINGLETON_H