// ===========================================================================
//
// Filename:    Proxy.h
//
// Description: Header file for the Posix thread functions. This class
//              encapsulates all posix thread functionality defined by the
//              system.
//
// History
//
// Author        Reason                                             Date
// ---------------------------------------------------------------------------
// P. Wielders   Initial creation                                   2002/05/24
//
// ===========================================================================

#ifndef __PROXY_H
#define __PROXY_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "StateTrigger.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----
//template <typename CONTEXT> class Proxy;
//template <typename CONTEXT> class ProxyList;
namespace Solutions { namespace Generics
{
	struct IProxy
	{
		//virtual              ~IProxy      () = 0;
		virtual unsigned int	RefCount	() const = 0;
		virtual void			AddRef		() const = 0;
		virtual void			Release		() const = 0;
	};

	template <typename CONTEXT, const bool EXTRABUFFER>
	class ProxyStore : 
		public CONTEXT,
		public IProxy
	{
		private:
			// ----------------------------------------------------------------
			// Never, ever allow reference counted objects to be assigned.
			// Create a new object and modify it. If the assignement operator
			// is used, give a compile error.
			// ----------------------------------------------------------------
			ProxyStore (const ProxyStore<CONTEXT, EXTRABUFFER>& a_Copy);
			ProxyStore<CONTEXT, EXTRABUFFER>& operator= (const ProxyStore<CONTEXT, EXTRABUFFER>& a_RHS);

		public:
			// Here we claim the buffer, plus some size for the buffer.
			void* 
				operator new ( 
					size_t			stAllocateBlock, 
					unsigned int	AdditionalSize)
			{
				size_t requestedSize = (stAllocateBlock + 3) & (static_cast<size_t>(~0) ^ 0x3);
				uint8* Space = reinterpret_cast<uint8*> (::malloc(requestedSize + sizeof(uint32) + AdditionalSize));
				
				*(reinterpret_cast<unsigned int*>(&Space[requestedSize])) = AdditionalSize;

				return Space;
			}
			// Somehow Purify gets lost if we do not delete it, overide the delete operator
			void 
				operator delete ( 
					void*			stAllocateBlock)
			{
				::free(stAllocateBlock);
			}

		public:
			inline ProxyStore () :
				CONTEXT	      (),
				m_RefCount	  (1)
			{
			}
			template <typename arg1>
				inline ProxyStore (arg1 a_Arg1) :
					CONTEXT	      (a_Arg1),
					m_RefCount	  (1)
			{
			}
			template <typename arg1, typename arg2>
				inline ProxyStore (arg1 a_Arg1, arg2 a_Arg2) :
					CONTEXT	      (a_Arg1, a_Arg2),
					m_RefCount    (1)
			{
			}
			template <typename arg1, typename arg2, typename arg3>
				inline ProxyStore (arg1 a_Arg1, arg2 a_Arg2, arg3 a_Arg3) :
					CONTEXT	      (a_Arg1, a_Arg2, a_Arg3),
					m_RefCount    (1)
			{
			}
			template <typename arg1, typename arg2, typename arg3, typename arg4>
				inline ProxyStore (arg1 a_Arg1, arg2 a_Arg2, arg3 a_Arg3, arg4 a_Arg4) :
					CONTEXT       (a_Arg1, a_Arg2, a_Arg3, a_Arg4),
					m_RefCount    (1)
			{
			}
			template <typename arg1, typename arg2, typename arg3, typename arg4, typename arg5>
				inline ProxyStore (arg1 a_Arg1, arg2 a_Arg2, arg3 a_Arg3, arg4 a_Arg4, arg5 a_Arg5) :
					CONTEXT	      (a_Arg1, a_Arg2, a_Arg3, a_Arg4, a_Arg5),
					m_RefCount    (1)
			{
			}

			template <typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6>
				inline ProxyStore (arg1 a_Arg1, arg2 a_Arg2, arg3 a_Arg3, arg4 a_Arg4, arg5 a_Arg5, arg6 a_Arg6) :
					CONTEXT       (a_Arg1, a_Arg2, a_Arg3, a_Arg4, a_Arg5, a_Arg6),
					m_RefCount    (1)
			{
			}

			template <typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6, typename arg7>
				inline ProxyStore (arg1 a_Arg1, arg2 a_Arg2, arg3 a_Arg3, arg4 a_Arg4, arg5 a_Arg5, arg6 a_Arg6, arg7 a_Arg7) :
					CONTEXT       (a_Arg1, a_Arg2, a_Arg3, a_Arg4, a_Arg5, a_Arg6, a_Arg7),
					m_RefCount    (1)
			{
			}

			template <typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6, typename arg7, typename arg8>
				inline ProxyStore (arg1 a_Arg1, arg2 a_Arg2, arg3 a_Arg3, arg4 a_Arg4, arg5 a_Arg5, arg6 a_Arg6, arg7 a_Arg7, arg8 a_Arg8) :
					CONTEXT	      (a_Arg1, a_Arg2, a_Arg3, a_Arg4, a_Arg5, a_Arg6, a_Arg7, a_Arg8),
					m_RefCount    (1)
			{
			}

			virtual ~ProxyStore ()
			{
				ASSERT (m_RefCount == 0);

				TRACE_L5 ("Destructor ProxyStore <0x%X>", TRACE_POINTER (this));
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
					delete this;
				}
			}

			// FOR DEBUG PURPOSE ONLY !!!!!!!
			virtual unsigned int RefCount () const
			{
				return (m_RefCount);
			}

			operator CONTEXT& ()
			{
				return (*this);
			}

			operator const CONTEXT& () const
			{
				return (*this);
			}

			uint8* Buffer()
			{
				uint8* result = NULL;

				if (EXTRABUFFER == true)
				{
					uint32 requestedSize = (sizeof(ProxyStore<CONTEXT, EXTRABUFFER>) + 3) & (static_cast<size_t>(~0) ^ 0x3);
					result = &(reinterpret_cast<uint8*>(this)[requestedSize + sizeof(uint32)]);
				}

				return (result);
			}

			uint32 AdditionalSize () const
			{
				uint32 result = 0;

				if (EXTRABUFFER == true)
				{
					uint32 requestedSize = (sizeof(ProxyStore<CONTEXT, EXTRABUFFER>) + 3) & (static_cast<size_t>(~0) ^ 0x3);
					result = *(reinterpret_cast<const uint32*> (&(reinterpret_cast<const uint8*>(this)[requestedSize])));
				}

				return (result);
			}
			//ProxyStore<CONTEXT,EXTRABUFFER> Resize (const uint32 size)
			//{
			//	void* newObject = this;
			//	uint32 bufferSize (AdditionalSize());
			//	if (size > bufferSize)
			//	{
			//		uint32  allocatedSize = ((sizeof(ProxyStore<CONTEXT, EXTRABUFFER>) + 3) & (static_cast<size_t>(~0) ^ 0x3));
			//		newObject = ::resize(this, allocatedSize + size);
			//		*(reinterpret_cast<unsigned int*>(&freeSpace[allocatedSize])) = AdditionalSize;
			//	}

			//	return (*(reinterpret_cast<ProxyStore<CONTEXT,EXTRABUFFER>*>(newObject)));
			//}

		private:
			mutable long						m_RefCount;
	};

	// ------------------------------------------------------------------------------
	// For a better handling of proxy objects, we need a base class for proxys. If
	// the object is a proxy, it, at least, contains a base proxy. This base proxy 
	// is a default will
	class EXTERNAL Proxy
	{
		protected:
			inline void	SetProxy (IProxy* RefInterface)
			{
				// Make sure that parameter object is a proxy counted object.
				ASSERT (RefInterface != NULL);

				// If we are building a proxy from an object, it might be done using the
				// this in the destructor. That is a tricky situation, which is not
				// favourable ;-) ASSERT here if we detct that.
				ASSERT (RefInterface->RefCount () != 0);

				m_RefCount = RefInterface;

				m_RefCount->AddRef ();
			}

		public:
			Proxy			() :
				m_RefCount	(NULL)
			{
			}
			Proxy			(const IProxy* RefInterface) :
				m_RefCount	(const_cast<IProxy*> (RefInterface))
			{
				if (m_RefCount != NULL)
				{
					m_RefCount->AddRef ();
				}
			}
			Proxy			(const Proxy& a_Copy) :
				m_RefCount	(a_Copy.m_RefCount)
			{
				if (m_RefCount != NULL)
				{
					m_RefCount->AddRef ();
				}
			}
			~Proxy()
			{
				if (m_RefCount != NULL)
				{
					m_RefCount->Release ();
				}
			}

			Proxy& operator= (const Proxy& a_Copy)
			{
				// Release the current holding object
				if (m_RefCount != NULL)
				{
					m_RefCount->Release ();
				}

				// Get the new object
				m_RefCount = a_Copy.m_RefCount;

				if (m_RefCount != NULL)
				{
					m_RefCount->AddRef ();
				}

				return (*this);
			}

		public:
			inline bool  IsValid () const
			{
				return (m_RefCount != NULL);
			}

			template<typename CONTEXT>
			inline CONTEXT* GetContext ()
			{
				// Only allowed on valid objects.
				ASSERT (m_RefCount != NULL);

				return (dynamic_cast<CONTEXT*>(m_RefCount));
			}

			template<typename CONTEXT>
			inline const CONTEXT* GetContext () const
			{
				// Only allowed on valid objects.
				ASSERT (m_RefCount != NULL);

				return (dynamic_cast<const CONTEXT*>(m_RefCount));
			}

			inline bool operator== (const Proxy& a_RHS) const
			{
				return (m_RefCount == a_RHS.m_RefCount);
			}
			inline bool operator!= (const Proxy& a_RHS) const
			{
				return (!Proxy::operator== (a_RHS));
			}

			// FOR DEBUG PURPOSE ONLY !!!!!!!
			inline unsigned int RefCount () const
			{
				return (m_RefCount != NULL ? m_RefCount->RefCount () : (~0));
			}

			inline void
				Release () const
			{
				// Only allowed on valid objects.
				ASSERT (m_RefCount != NULL);

				m_RefCount->Release ();

				m_RefCount = NULL;
			}

			inline void AddRef () const
			{
				// Only allowed on valid objects.
				ASSERT (m_RefCount != NULL);

				m_RefCount->AddRef ();
			}

			const IProxy* GetProxyInterface () const
			{
				return (m_RefCount);
			}

			inline bool LastRef () const
			{
				return (m_RefCount != NULL ? (m_RefCount->RefCount () == 1) : false);
			}

		private:
			mutable IProxy*	m_RefCount;
	};

	// ------------------------------------------------------------------------------
	// Reference counted object can only exist on heap (if reference count reaches 0)
	// delete is done. To avoid creation on the stack, this object can only be created 
	// via the static create methods on this object.
	template <typename CONTEXT>
	class ProxyType : public Proxy  
	{
		public:
			explicit ProxyType (CONTEXT&    TheObject) :
				Proxy          ()
			{
				IProxy*	Interface = dynamic_cast<IProxy*> (&TheObject);

				SetProxy (Interface);
			}
			// The proxy create method for the static Create metjods in this class.
			ProxyType (
				IProxy*	   RefObject) :
					Proxy (RefObject)
			{
				TRACE_L5 ("Constructor Proxy(ProxyStore) <0x%X>", TRACE_POINTER (this));
			}

			ProxyType () :
				Proxy ()
			{
				TRACE_L5 ("Constructor Proxy() <0x%X>", TRACE_POINTER (this));
			}

			explicit ProxyType (
				const Proxy&	 BaseProxy) :
					Proxy		(BaseProxy)
			{
	#ifdef _DEBUG
				// If it is a valid proxy, find the associated object
				if (BaseProxy.IsValid ())
				{
					// All in favour of speed. But as we know errors are made. In debug check if
					// the Proxy we create here actually has references this object.
					ASSERT (dynamic_cast<const CONTEXT*> (BaseProxy.GetProxyInterface ())!= NULL);
				}
	#endif
			}

			ProxyType     (const ProxyType<CONTEXT>& a_Copy) :
				Proxy (a_Copy)
			{
				TRACE_L5 ("Constructor Proxy(CInfraBaseObject) <0x%X>", TRACE_POINTER (this));
			}

			~ProxyType()
			{
				TRACE_L5 ("Destructor Proxy <0x%X>", TRACE_POINTER (this));
			}

			inline static ProxyType<CONTEXT> Create ()
			{
				return (new (0) ProxyStore<CONTEXT, false> ());
			}

			template <typename arg1>
			inline static ProxyType<CONTEXT> Create (arg1 a_Arg1)
			{
				return (new (0) ProxyStore<CONTEXT, false> (a_Arg1));
			}

			template <typename arg1, typename arg2>
			inline static ProxyType<CONTEXT> Create (arg1 a_Arg1, arg2 a_Arg2)
			{
				return (new (0) ProxyStore<CONTEXT, false> (a_Arg1, a_Arg2));
			}

			template <typename arg1, typename arg2, typename arg3>
			inline static ProxyType<CONTEXT> Create (arg1 a_Arg1, arg2 a_Arg2, arg3 a_Arg3)
			{
				return (new (0) ProxyStore<CONTEXT, false> (a_Arg1, a_Arg2, a_Arg3));
			}

			template <typename arg1, typename arg2, typename arg3, typename arg4>
			inline static ProxyType<CONTEXT> Create (arg1 a_Arg1, arg2 a_Arg2, arg3 a_Arg3, arg4 a_Arg4)
			{
				return (new (0) ProxyStore<CONTEXT, false> (a_Arg1, a_Arg2, a_Arg3, a_Arg4));
			}

			template <typename arg1, typename arg2, typename arg3, typename arg4, typename arg5>
			inline static ProxyType<CONTEXT> Create (arg1 a_Arg1, arg2 a_Arg2, arg3 a_Arg3, arg4 a_Arg4, arg5 a_Arg5)
			{
				return (new (0) ProxyStore<CONTEXT, false> (a_Arg1, a_Arg2, a_Arg3, a_Arg4, a_Arg5));
			}

			template <typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6>
			inline static ProxyType<CONTEXT> Create (arg1 a_Arg1, arg2 a_Arg2, arg3 a_Arg3, arg4 a_Arg4, arg5 a_Arg5, arg6 a_Arg6)
			{
				return (new (0) ProxyStore<CONTEXT, false> (a_Arg1, a_Arg2, a_Arg3, a_Arg4, a_Arg5, a_Arg6));
			}

			inline static ProxyType<CONTEXT> CreateEx (const uint32 size)
			{
				return (new (size) ProxyStore<CONTEXT, true> ());
			}

			template <typename arg1>
			inline static ProxyType<CONTEXT> CreateEx (const uint32 size, arg1 a_Arg1)
			{
				return (new (size) ProxyStore<CONTEXT, true> (a_Arg1));
			}

			template <typename arg1, typename arg2>
			inline static ProxyType<CONTEXT> CreateEx (const uint32 size, arg1 a_Arg1, arg2 a_Arg2)
			{
				return (new (size) ProxyStore<CONTEXT, true> (a_Arg1, a_Arg2));
			}

			template <typename arg1, typename arg2, typename arg3>
			inline static ProxyType<CONTEXT> CreateEx (const uint32 size, arg1 a_Arg1, arg2 a_Arg2, arg3 a_Arg3)
			{
				return (new (size) ProxyStore<CONTEXT, true> (a_Arg1, a_Arg2, a_Arg3));
			}

			template <typename arg1, typename arg2, typename arg3, typename arg4>
			inline static ProxyType<CONTEXT> CreateEx (const uint32 size, arg1 a_Arg1, arg2 a_Arg2, arg3 a_Arg3, arg4 a_Arg4)
			{
				return (new (size) ProxyStore<CONTEXT, true> (a_Arg1, a_Arg2, a_Arg3, a_Arg4));
			}

			template <typename arg1, typename arg2, typename arg3, typename arg4, typename arg5>
			inline static ProxyType<CONTEXT> CreateEx (const uint32 size, arg1 a_Arg1, arg2 a_Arg2, arg3 a_Arg3, arg4 a_Arg4, arg5 a_Arg5)
			{
				return (new (size) ProxyStore<CONTEXT, true> (a_Arg1, a_Arg2, a_Arg3, a_Arg4, a_Arg5));
			}

			template <typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6>
			inline static ProxyType<CONTEXT> CreateEx (const uint32 size, arg1 a_Arg1, arg2 a_Arg2, arg3 a_Arg3, arg4 a_Arg4, arg5 a_Arg5, arg6 a_Arg6)
			{
				return (new (size) ProxyStore<CONTEXT, true> (a_Arg1, a_Arg2, a_Arg3, a_Arg4, a_Arg5, a_Arg6));
			}

			inline ProxyType<CONTEXT>& operator= (const ProxyType<CONTEXT>& a_Copy)
			{
				Proxy::operator= (a_Copy);

				return (*this);
			}

			inline bool operator== (const ProxyType<CONTEXT>& a_RHS) const
			{
				return (Proxy::operator== (a_RHS));
			}

			inline bool operator!= (const ProxyType<CONTEXT>& a_RHS) const
			{
				return (Proxy::operator!= (a_RHS));
			}

			inline bool operator== (const CONTEXT& a_RHS) const
			{
				// ASSERT is triggered (if this is an invalid opbject in GetBaseObject().
				return (GetContext<CONTEXT>() == &a_RHS);
			}

			inline bool operator!= (const CONTEXT& a_RHS) const
			{
				return (!ProxyType<CONTEXT>::operator== (a_RHS));
			}

			inline const CONTEXT* operator->() const
			{
				const CONTEXT* Object = GetContext<CONTEXT>();

				ASSERT (Object != NULL);

				return (Object);
			}

			inline CONTEXT* operator->()
			{
				CONTEXT* Object = GetContext<CONTEXT>();

				ASSERT (Object != NULL);

				return (Object);
			}

			inline const CONTEXT& operator* () const
			{
				const CONTEXT* Object = GetContext<CONTEXT>();

				ASSERT (Object != NULL);

				return (*Object);
			}

			inline CONTEXT& operator* ()
			{
				CONTEXT* Object = GetContext<CONTEXT>();

				ASSERT (Object != NULL);

				return (*Object);
			}

			inline uint8 *Buffer()
			{
				return (GetContext<ProxyStore<CONTEXT, true>>()->Buffer());
			}
			inline uint32 AdditionalSize() const
			{
				return (GetContext<ProxyStore<CONTEXT, true>>()->AdditionalSize());
			}
			inline ProxyType<CONTEXT> Resize (const uint32 size)
			{
				return (GetContext<ProxyStore<CONTEXT, true>>->Rezize(size));
			}		
	};

	template<typename CASTOBJECT>
	ProxyType<CASTOBJECT> proxy_cast(const Proxy& castObject)
	{
		return (ProxyType<CASTOBJECT> (castObject));
	}

	const unsigned int PROXY_DEFAULT_ALLOC_SIZE   = 20;
	const unsigned int PROXY_LIST_ERROR			  = static_cast<unsigned int> (-1);

	template <typename CONTEXT>
	class ProxyList
	{
		// http://www.ictp.trieste.it/~manuals/programming/sun/c-plusplus/c++_ug/Templates.new.doc.html

		// The design goal of this template class is speed. To speed up the
		// processing of the list leaves out the concurrency aspects. This template
		// is NOT thread safe, but fast. So if it is used by more threads, make it
		// thread safe. If not use it.
		// Further more the assumption is made that the template is used by smart
		// software engineers. Do not request an object from an empty list or from
		// an index that is out of scope. The code will assert in TRACE_LEVEL >= 1.

		public:
			explicit ProxyList (unsigned int a_Size) :
			  m_List           (NULL),
			  m_Max            (a_Size),
			  m_Current        (0)
			{
			}

			~ProxyList ()
			{
			  if (m_List != NULL)
			  {
				#ifdef DEBUG
				for (unsigned int l_Teller = 0; l_Teller != m_Current; l_Teller++)
				{
					if (m_List[l_Teller] != NULL)
					{
						TRACE (_T("Possible memory leak detected on object queue"));
					}
				}
				#endif

				delete m_List;
			  }
			}

			unsigned int Find (const ProxyType<CONTEXT>& a_Entry)
			{
			  // Remeber the item on the location.
			  unsigned int  l_Index  = 0;

			  ASSERT (a_Entry.IsValid () == true);

			  if (m_List != NULL)
			  {
				// Find the given pointer
				while ( (l_Index < m_Current) && (m_List[l_Index] != a_Entry.GetContext<IProxy> ()) )
				{
				  l_Index++;
				}
			  }

			  // If we found it, return the index.
			  return (l_Index != m_Current ? l_Index : PROXY_LIST_ERROR);
			}

			unsigned int Add (const ProxyType<CONTEXT>& a_Entry)
			{
			  ASSERT (a_Entry.IsValid () == true);

			  if (m_List == NULL)
			  {
				  m_List = new IProxy* [m_Max];

			#ifdef DEBUG
				memset (&m_List[0], 0, (m_Max * sizeof(IProxy*)));
			#endif

			  }

			  // That's easy, if there is space left..
			  if (m_Current >= m_Max)
			  {
				// Time to expand. Double the capacity. Allocate the capacity.
				IProxy**  l_NewList = new IProxy* [(m_Max << 1)];

				// Copy the old list in (Dirty but quick !!!!)
				memcpy ( l_NewList, &m_List[0], (m_Max * sizeof (IProxy*)));

				// Update the capcacity counter.
				m_Max = m_Max << 1;

				// Delete the old buffer.
				delete [] m_List;

				// Get the new copy installed.
				m_List = l_NewList;
			  }

			  // Now there is space enough, insert the new pointer.
			  SetProxy (m_Current, a_Entry);
			  a_Entry.AddRef ();

			  // Make sure the next one is placed on the next spot.
			  m_Current++;

			  // return the index of the newly added element.
			  return (m_Current-1);
			}

			void Remove (unsigned int a_Index, ProxyType<CONTEXT>& a_Entry)
			{
				ASSERT (a_Index < m_Current);
				ASSERT (m_List  != NULL);

				// Remember the item on the location, It should be a relaes and an add for
				// the new one, To optimize for speed, just copy the count.
				a_Entry = GetProxy (a_Index);

				// If it is taken out, release the reference that we took during the add
				m_List[a_Index]->Release ();

				// Delete one element.
				m_Current--;

				// If it is not the last one, we have to move...
				if ( a_Index < m_Current )
				{
					// Kill the entry, (again dirty but quick).
					memcpy (&(m_List[a_Index]), &(m_List[a_Index+1]), (m_Current - a_Index) * sizeof (IProxy*));
				}

			#ifdef DEBUG
				m_List[m_Current] = NULL;
			#endif
			}

			void Remove (unsigned int a_Index)
			{
				ASSERT (a_Index < m_Current);
				ASSERT (m_List  != NULL);

				// If it is taken out, release the reference that we took during the add
				m_List[a_Index]->Release ();

				// Delete one element.
				m_Current--;

				// If it is not the last one, we have to move...
				if ( a_Index < m_Current )
				{
					// Kill the entry, (again dirty but quick).
					memcpy (&(m_List[a_Index]), &(m_List[a_Index+1]), (m_Current - a_Index) * sizeof (IProxy*));
				}

			#ifdef DEBUG
				m_List[m_Current] = NULL;
			#endif
			}

			bool Remove (const ProxyType<CONTEXT>& a_Entry)
			{
			  ASSERT (a_Entry.IsValid () != false);
			  ASSERT (m_List  != NULL);

			  // Remeber the item on the location.
			  unsigned int  l_Index  = Find (a_Entry);

			  // If it is found, remove it.
			  if (l_Index != PROXY_LIST_ERROR)
			  {
				Proxy<CONTEXT>  Entry;

				// Remove this index.
				Remove (l_Index, Entry);
			  }

			  // And did we succeed ?
			  return (l_Index != PROXY_LIST_ERROR);
			}

			void Clear (const unsigned int a_Start, const unsigned int a_Count)
			{
			  ASSERT ((a_Start + a_Count) <= m_Current);

			  if (m_List != NULL)
			  {
				// Lower the ref count of all elements to be destructed.
				for (unsigned int l_Teller = a_Start; l_Teller != a_Start + a_Count; l_Teller++)
				{
				  ASSERT (m_List[l_Teller] != NULL);

				  // Relinguish our reference to this element.
				  m_List[l_Teller]->Release ();
				}

				// If it is not the last one, we have to move...
				if ( (a_Start + a_Count) < m_Current )
				{
				  // Kill the entry, (again dirty but quick).
				  memcpy (&(m_List[a_Start]), &(m_List[a_Start+a_Count+1]), (a_Count * sizeof (IProxy*)));

				#ifdef DEBUG
				  // Set all no longer used element to NULL
				  memset (&(m_List[m_Current-a_Count]), 0, (a_Count * sizeof (IProxy*)));
				}
				else
				{
				  // Set all no longer used element to NULL
				  memset (&(m_List[a_Start]), 0, (a_Count * sizeof (IProxy*)));
				#endif
				}

				// Delete the given elements.
				m_Current = m_Current - a_Count;
			  }
			}

			inline unsigned int
				Count () const
			{ 
				return (m_Current); 
			}

			inline ProxyType<CONTEXT>
				operator[] (unsigned int a_Index) const
			{
				ASSERT ( a_Index < m_Current );
				ASSERT ( m_List[a_Index] != NULL );

				return (GetProxy(a_Index));
			}

		private:
			inline ProxyType<CONTEXT>
				GetProxy (
					unsigned int Index) const
			{
				return (ProxyType<CONTEXT>(m_List[Index]));
			}

			inline void
				SetProxy (
					unsigned int							Index,
					const ProxyType<CONTEXT>&	proxy)
			{
				m_List[Index] = proxy.GetProxyInterface ();
			}

			//------------------------------------------------------------------------
			// Protected Attributubes
			//------------------------------------------------------------------------
			private:
				IProxy**	 m_List;
				unsigned int m_Max;
				unsigned int m_Current;
	};

	template <typename CONTEXT>
	class ProxyQueue  
	{
		private:
			// -------------------------------------------------------------------
			// This object should not be copied or assigned. Prevent the copy 
			// constructor and assignment constructor from being used. Compiler 
			// generated assignment and copy methods will be blocked by the 
			// following statments.
			// Define them but do not implement them, compile error/link error.
			// -------------------------------------------------------------------
			ProxyQueue ();
			ProxyQueue (const ProxyQueue<CONTEXT>&);
			ProxyQueue& operator= (const ProxyQueue<CONTEXT>&);

		public:
			explicit ProxyQueue	(
				const unsigned int					 a_HighWaterMark) :
					m_Queue							(a_HighWaterMark),
					m_State							(EMPTY),
					m_MaxEntries					(a_HighWaterMark)
			{
				// A highwatermark of 0 is bullshit.
				ASSERT (a_HighWaterMark != 0);

				TRACE_L5 ("Constructor ProxyQueue <0x%X>", TRACE_POINTER (this));
			}

			~ProxyQueue	()
			{
				TRACE_L5 ("Destructor ProxyQueue <0x%X>", TRACE_POINTER (this));

				// Disable the queue and flush all entries.
				Disable ();
			}

			typedef enum
			{
				EMPTY	 = 0x0001,
				ENTRIES  = 0x0002,
				LIMITED	 = 0x0004,
				DISABLED = 0x0008

			}	enumQueueState;

			// -------------------------------------------------------------------
			// This queue enforces a Producer-Consumer pattern. It takes a 
			// pointer on the heap. This pointer is created by the caller 
			// (producer) of the Post method. It should be destructed by the 
			// receiver (consumer). The consumer is the one that calls the 
			// Receive method.
			// -------------------------------------------------------------------
			bool
				Remove (
					const ProxyType<CONTEXT>&	a_Entry);

			bool	
				Post (
					ProxyType<CONTEXT>&	a_Entry);

			bool	
				Insert (
					ProxyType<CONTEXT>&	a_Entry,
					DWORD				a_WaitTime = INFINITE);

			bool
				Extract (
					ProxyType<CONTEXT>&	a_Entry,
					DWORD				a_WaitTime = INFINITE);

			void Enable ()
			{
				// This needs to be atomic. Make sure it is.
				m_State.Lock ();

				if ( m_State == DISABLED )
				{
					m_State.SetState (EMPTY);
				}

				// Done with the administration. Release the lock.
				m_State.Unlock ();
			}

			void Disable ()
			{
				// This needs to be atomic. Make sure it is.
				m_State.Lock ();

				if (m_State != DISABLED)
				{
					// Change the state
					//lint -e{534}
					m_State.SetState (DISABLED);

					// Clear all entries !!
					m_Queue.Clear (0,  m_Queue.Count ());
				}

				// Done with the administration. Release the lock.
				m_State.Unlock ();
			}

			inline void Lock ()
			{
				// Lock the Queue.
				m_State.Lock ();
			}

			inline void Unlock ()
			{
				// Unlock the queue
				m_State.Unlock ();
			}

			inline void FreeSlot () const
			{
				m_State.WaitState (false, DISABLED|ENTRIES|EMPTY, INFINITE);
			}

			inline bool IsEmpty () const
			{
				return (m_Queue.Count () == 0);
			}

			inline bool IsFull () const
			{
				return (m_Queue.Count () >= m_MaxEntries);
			}

		private:
			ProxyList<CONTEXT>			 m_Queue;
			StateTrigger<enumQueueState> m_State;
			unsigned int                 m_MaxEntries;
	};

	// ---------------------------------------------------------------------------
	// PROXY QUEUE
	// ---------------------------------------------------------------------------

	template <class CONTEXT>
	bool
		ProxyQueue<CONTEXT>::Remove (
			const ProxyType<CONTEXT>&	a_Entry)
	{
		bool l_Removed = false;

		// This needs to be atomic. Make sure it is.
		m_State.Lock ();

		if (m_State != DISABLED)
		{
			// Yep, let's fill it
			l_Removed = m_Queue.Remove (a_Entry);

			// Determine the new state.
			m_State.SetState ( IsEmpty () ? EMPTY : ENTRIES );
		}

		// Done with the administration. Release the lock.
		m_State.Unlock ();

		return (l_Removed);
	}

	template <class CONTEXT>
	bool
		ProxyQueue<CONTEXT>::Post (
			ProxyType<CONTEXT>&	a_Entry)
	{
		bool Result = false;

		// This needs to be atomic. Make sure it is.
		m_State.Lock ();

		if (m_State != DISABLED)
		{
			// Yep, let's fill it
			//lint -e{534}
			m_Queue.Add (a_Entry);

			// Determine the new state.
			m_State.SetState ( IsFull () ? LIMITED : ENTRIES );

			Result = true;
		}

		// Done with the administration. Release the lock.
		m_State.Unlock ();

		return (Result);
	}

	template <class CONTEXT>
	bool
		ProxyQueue<CONTEXT>::Insert (
			ProxyType<CONTEXT>&	a_Entry,
			DWORD						a_WaitTime)
	{
		bool	l_Posted	= false;
		bool	l_Triggered	= true;

		// This needs to be atomic. Make sure it is.
		m_State.Lock ();

		if (m_State != DISABLED)
		{
			do
			{
				// And is there a slot available to us ?
				if (m_State != LIMITED)
				{
					// We have posted it.
					l_Posted = true;

					// Yep, let's fill it
					//lint -e{534}
					m_Queue.Add (a_Entry);

					// Determine the new state.
					m_State.SetState ( IsFull () ? LIMITED : ENTRIES );
				}
				else
				{
					// We are moving into a wait, release the lock.
					m_State.Unlock ();

					// Wait till the status of the queue changes.
					l_Triggered = m_State.WaitState (false, DISABLED|ENTRIES|EMPTY, a_WaitTime);

					// Seems something happend, lock the administration.
					m_State.Lock ();

					// If we were reset, that is assumed to be also a timeout
					l_Triggered = l_Triggered && (m_State != DISABLED);
				}

			} while ( (l_Posted == false) && (l_Triggered != false) );
		}

		// Done with the administration. Release the lock.
		m_State.Unlock ();

		return (l_Posted);
	}

	template <class CONTEXT>
	bool
		ProxyQueue<CONTEXT>::Extract (
			ProxyType<CONTEXT>&	a_Result,
			DWORD				a_WaitTime)
	{
		bool	l_Received  = false;
		bool	l_Triggered = true;

		// This needs to be atomic. Make sure it is.
		m_State.Lock ();

		if (m_State != DISABLED)
		{
			do
			{
				// And is there a slot to read ?
				if (m_State != EMPTY)
				{
					l_Received = true;

					// Get the first entry from the first spot..
					m_Queue.Remove (0, a_Result);

					// Determine the new state.
					//lint -e{534}
					m_State.SetState ( IsEmpty () ? EMPTY : ENTRIES );
				}
				else
				{
					// We are moving into a wait, release the lock.
					m_State.Unlock ();

					// Wait till the status of the queue changes.
					l_Triggered = m_State.WaitState (false, DISABLED|ENTRIES|LIMITED, a_WaitTime);

					// Seems something happend, lock the administration.
					m_State.Lock ();

					// If we were reset, that is assumed to be also a timeout
					l_Triggered = l_Triggered && (m_State != DISABLED);
				}

			} while ( (l_Received == false) && (l_Triggered != false));
		}

		// Done with the administration. Release the lock.
		m_State.Unlock ();

		return (l_Received);
	}

} } // namespace Solutions::Generics

#endif // __INFRAPROXY_H
