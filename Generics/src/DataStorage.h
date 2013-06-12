#ifndef __DATASTORAGE_H
#define __DATASTORAGE_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Portability.h"
#include "Proxy.h"
#include "Singleton.h"

namespace Solutions { namespace Generics
{
	// ---- Referenced classes and types ----

	// ---- Helper types and constants ----
	 
	// ---- Helper functions ----
	struct IBuffer : public IProxy
	{
		virtual uint64				Size	() const = 0;
		virtual uint8*				Buffer	() = 0;
		virtual ProxyType<IBuffer>	Clone	(const uint64 size) = 0;
	};

	class EXTERNAL StandAloneStorage
	{
		private:
			StandAloneStorage(const StandAloneStorage&);
			StandAloneStorage& operator= (const StandAloneStorage&);

		public:
			inline StandAloneStorage() {}
			inline ~StandAloneStorage() {}

		public:
			inline void Create(IBuffer* /* storage */)
			{
			}
			inline void Destroy(IBuffer* storage)
			{
				delete storage;
			}
			template<typename STORAGE>
			ProxyType<IBuffer> Clone (const uint64 size)
			{
				uint32 calculatedSize = ((static_cast<uint32>(size) + 1024) / 1024);
				ProxyType<IBuffer> result (STORAGE::Create(calculatedSize * 1024));
				return (result);	
			}
	};

	class EXTERNAL PoolStorage
	{
		private:
			PoolStorage();
			PoolStorage(const PoolStorage&);
			PoolStorage& operator= (const PoolStorage&);

		public:
			inline PoolStorage(std::list<IBuffer*>& queue) : m_Pool(queue) {}
			inline ~PoolStorage() {}

		public:
			inline void Create(IBuffer* /* storage */)
			{
			}
			inline void Destroy(IBuffer* storage)
			{
				m_Pool.push_front(storage);
			}
			template<typename STORAGE>
			ProxyType<IBuffer> Clone (const uint64 size)
			{
				uint32 calculatedSize = ((static_cast<uint32>(size) + 1024) / 1024);
				ProxyType<IBuffer> result (STORAGE::Create<std::list<IBuffer*>&>(calculatedSize * 1024, m_Pool));
				return (result);	
			}

		private:
			std::list<IBuffer*>& m_Pool;
	};

	template<typename POLICY>
	class ExtendableStorage : public IBuffer 
	{	
		private:
			// ----------------------------------------------------------------
			// Never, ever allow reference counted objects to be assigned.
			// Create a new object and modify it. If the assignement operator
			// is used, give a compile error.
			// ----------------------------------------------------------------
			ExtendableStorage (const ExtendableStorage<POLICY>& a_Copy);
			ExtendableStorage<POLICY>& operator= (const ExtendableStorage<POLICY>& a_RHS);

			ExtendableStorage (uint32 size) : m_RefCount(1), m_Size (size), m_Policy()
			{
				m_Policy.Create(this);
			}
			template<typename Arg1>
			ExtendableStorage (uint32 size, Arg1 parameter) : m_RefCount(1), m_Size (size), m_Policy(parameter)
			{
				m_Policy.Create(this);
			}

		public:
			// Here we claim the buffer, plus some size for the buffer.
			void* 
				operator new ( 
					size_t			stAllocateBlock, 
					unsigned int	AdditionalSize)
			{
				size_t requestedSize = (stAllocateBlock + 3) & (static_cast<size_t>(~0) ^ 0x3);
				return (reinterpret_cast<uint8*> (::malloc(requestedSize + AdditionalSize)));
			}
			// Somehow Purify gets lost if we do not delete it, overide the delete operator
			void 
				operator delete ( 
					void*			stAllocateBlock)
			{
				::free(stAllocateBlock);
			}

			virtual ~ExtendableStorage()
			{
				ASSERT (m_RefCount == 0);
			}

		public:
			inline static ProxyType<IBuffer> Create(const uint32 size)
			{
				return (ProxyType<IBuffer> (new (size) ExtendableStorage<POLICY>(size)));
			}
			template<typename POLICYPARAMETER>
			inline static ProxyType<IBuffer> Create(const uint32 size, POLICYPARAMETER parameter)
			{
				 IBuffer* result = new (size) ExtendableStorage<POLICY>(size, parameter);
				 return (ProxyType<IBuffer>(result));
			}				
			virtual void AddRef () const
			{	
				::InterlockedIncrement(&m_RefCount);
			}

			virtual void Release () const
			{
				LONG Result;

				if ( (Result = ::InterlockedDecrement (&m_RefCount)) == 0)
				{
					m_Policy.Destroy(const_cast<IBuffer*>(static_cast<const IBuffer*>(this)));
				}
			}

			// FOR DEBUG PURPOSE ONLY !!!!!!!
			virtual unsigned int RefCount () const
			{
				return (m_RefCount);
			}

			virtual uint8* Buffer()
			{
				size_t requestedSize = (sizeof(ExtendableStorage<POLICY>) + 3) & (static_cast<size_t>(~0) ^ 0x3);
				return &(reinterpret_cast<uint8*>(this)[requestedSize]);
			}

			virtual uint64 Size () const
			{
				return (m_Size);
			}
	
			ProxyType<IBuffer> Clone (const uint64 size)
			{
				ProxyType<IBuffer> result (m_Policy.Clone<ExtendableStorage<POLICY>> (size));

				if (result.IsValid())
				{
					::memcpy (result->Buffer(), Buffer(), static_cast<size_t>(m_Size));
				}

				return (result);	
			}
		
		private:
			mutable long	m_RefCount;
			uint64			m_Size;
			mutable POLICY	m_Policy;
	};

	template<const unsigned int BLOCKSIZE>
	class ScopedStorage : public IBuffer 
	{	
		private:
			// ----------------------------------------------------------------
			// Never, ever allow reference counted objects to be assigned.
			// Create a new object and modify it. If the assignement operator
			// is used, give a compile error.
			// ----------------------------------------------------------------
			ScopedStorage (const ScopedStorage<BLOCKSIZE>& a_Copy);
			ScopedStorage<BLOCKSIZE>& operator= (const ScopedStorage<BLOCKSIZE>& a_RHS);

		public:
			ScopedStorage () : m_RefCount(0)
			{
			}
			virtual ~ScopedStorage()
			{
				// If we reach this point and the refcount is not "0", the objects still
				// referencing this object will be pointing to "dangling" space.
				// Very Dangerous. Resolve this or use the DataStorage class !!!!
				ASSERT (m_RefCount == 0);
			}

		public:
			inline operator ProxyType<IBuffer> ()
			{
				return (ProxyType<IBuffer> (this));
			}
				
			virtual void AddRef () const
			{	
				::InterlockedIncrement(&m_RefCount);
			}

			virtual void Release () const
			{
				::InterlockedDecrement (&m_RefCount);
			}

			// FOR DEBUG PURPOSE ONLY !!!!!!!
			virtual unsigned int RefCount () const
			{
				return (m_RefCount);
			}

			virtual uint8* Buffer()
			{
				return &(m_Buffer[0]);
			}

			virtual uint64 Size () const
			{
				return (BLOCKSIZE);
			}
	
			ProxyType<IBuffer> Clone (const uint64 /* size */)
			{
				// Scbuffer opedStorage should not be changed into dynamic resizable buffers.
				// Use a dynamic buffer e.g. DataStorage  
				ASSERT (false);

				return (ProxyType<IBuffer>());	
			}
		
		private:
			mutable long	m_RefCount;
			uint8			m_Buffer[BLOCKSIZE];
	};

	template<const unsigned int BLOCKSIZE, const unsigned int DEFAULTSET>
 	class DataStoragePoolType
	{
		private:
			template<const unsigned int BLOCKSIZE, const unsigned int DEFAULTSET>
 			class Element : public IBuffer
			{	
				private:
					Element(const Element&);
					Element& operator= (const Element&);

				public:
 					Element () : m_RefCount(0)
					{
					}
					virtual ~Element()
					{
					}

				public:
					virtual void AddRef () const
					{	
						::InterlockedIncrement(&m_RefCount);
					}

					virtual void Release () const
					{
						LONG Result;

						if ( (Result = ::InterlockedDecrement (&m_RefCount)) == 0)
						{
							DataStoragePoolType<BLOCKSIZE,DEFAULTSET>::Instance().Release (this);
						}
					}

					// FOR DEBUG PURPOSE ONLY !!!!!!!
					virtual unsigned int RefCount () const
					{
						return (m_RefCount);
					}

					virtual uint8* Buffer()
					{
						return &(m_Data[0]);
					}

					virtual uint64 Size () const
					{
						return (BLOCKSIZE);
					}

					virtual uint64 Size (const uint64 size)
					{
 						return (size < BLOCKSIZE ? size : BLOCKSIZE);
					}

					virtual ProxyType<IBuffer> Clone (const uint64 /* size */)
					{
						return (ProxyType<IBuffer>());
					}

				private:
					mutable long	m_RefCount;
					uint8			m_Data[BLOCKSIZE];

			};

		public:
			DataStoragePoolType()
			{
				for (uint32 teller = 0; teller != DEFAULTSET; teller++)
				{
					m_FreeBlocks.push_front(&(*(ExtendableStorage<PoolStorage>::Create<std::list<IBuffer*>&>(BLOCKSIZE, m_FreeBlocks))));
				}
			}
			~DataStoragePoolType()
			{
				while (m_FreeBlocks.empty() == false)
				{
					delete m_FreeBlocks.front();
					m_FreeBlocks.pop_front();
				}
			}

		public:
			ProxyType<IBuffer>	Aquire ()
			{
				ProxyType<IBuffer> result;

				m_AdminLock.Lock();
				
				ASSERT (block.RefCount == 0);

				if (m_FreeBlocks.empty() == true)
				{
					result = new Element<BLOCKSIZE,DEFAULTSIZE>();
				}
				else
				{
					result = m_FreeBlocks.pop_front();
				}

				m_AdminLock.Unlock();
				
				return (ProxyType<IBuffer>(result));
			}

			void Release (const ProxyType<IBuffer>& block)
			{
				m_AdminLock.Lock();
				
				ASSERT (block.RefCount == 0);

				m_FreeBlocks.push_front(&(*block));

				m_AdminLock.Unlock();
			}

		private:
			std::list<IBuffer*>		m_FreeBlocks;
			CriticalSection			m_AdminLock;
	};

	typedef Generics::ExtendableStorage<Generics::StandAloneStorage>	DataStorage;
	typedef Generics::ProxyType<Generics::IBuffer>						DataProxy;

} } // namespace Solutions::Generics

#endif // __DATAELEMENT_H
