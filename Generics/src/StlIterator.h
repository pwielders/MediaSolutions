// ===========================================================================
// Description: Header file for for the STL wrapper around the iterator.
//// ===========================================================================

#ifndef __ITERATOR_H
#define __ITERATOR_H

#include "Module.h"

namespace Solutions { namespace Generics
{
	template <typename CONTAINER, typename ELEMENT>
	class IteratorType
	{
		private:
			typedef enum
			{
				BEGIN,
				LOCATION,
				END
			}	enumLocation;

		public:
			IteratorType() :
				m_Container	(NULL),
				m_Iterator	(),
				m_Status	(BEGIN)
			{
			}
			IteratorType	(CONTAINER& container) :
				m_Container	(&container),
				m_Iterator	(container.begin()),
				m_Status	(BEGIN)
			{
			}
			IteratorType	(CONTAINER& container, typename CONTAINER::iterator startPoint) :
				m_Container	(&container),
				m_Iterator	(startPoint),
				m_Status	(LOCATION)
			{
			}
			IteratorType	(const IteratorType<CONTAINER, ELEMENT>& copy) :
				m_Container	(copy.m_Container),
				m_Iterator	(copy.m_Iterator),
				m_Status	(copy.m_Status)
			{
			}
			~IteratorType ()
			{
			}

			IteratorType<CONTAINER, ELEMENT>& operator= (const IteratorType<CONTAINER, ELEMENT>& RHS)
			{
				m_Container	= RHS.m_Container;
				m_Iterator	= RHS.m_Iterator;
				m_Status	= RHS.m_Status;

				return (*this);
			}

		public:
			inline bool IsValid () const
			{
				return (m_Status == LOCATION);
			}

			inline void Reset (const bool head)
			{
				if (m_Container != NULL)
				{
					if (head == true)
					{
						m_Iterator = m_Container->begin();
						m_Status = BEGIN;
					}
					else
					{
						m_Iterator = m_Container->end();
						m_Status = END;
					}
				}
			}

			bool Previous ()
			{
				if ( (m_Status != BEGIN) && (m_Container != NULL) )
				{
					if (m_Iterator == m_Container->begin())
					{
						m_Status = BEGIN;
					}
					else
					{
						m_Iterator--;
					}
				}
				return (IsValid ());
			}

			bool Next ()
			{
				if ( (m_Status != END) && (m_Container != NULL) )
				{
					if (m_Iterator == m_Container->end())
					{
						m_Status = END;
					}
					else if (m_Status == BEGIN)
					{
						m_Status = LOCATION;
					}
					else
					{
						m_Iterator++;

						if (m_Iterator == m_Container->end())
						{
							m_Status = END;
						}
					}
				}
				return (IsValid ());
			}

			inline ELEMENT* operator->()
			{
				ASSERT (IsValid());

				return (*m_Iterator);
			}

			inline const ELEMENT* operator->() const
			{
				ASSERT (IsValid());

				return (*m_Iterator);
			}

			inline ELEMENT& operator* ()
			{
				ASSERT (IsValid());

				return (*m_Iterator);
			}
			
			inline const ELEMENT& operator* () const
			{
				ASSERT (IsValid());

				return (*m_Iterator);
			}

			inline uint32 Count () const
			{
				return (m_Container == NULL ? 0 : m_Container->size());
			}

		protected:
			inline void Lock() const
			{
				ASSERT (m_Container != NULL);

				m_Container->Lock ();
			}

			inline void Unlock() const
			{
				ASSERT (m_Container != NULL);

				m_Container->Unlock ();
			}

		private:
			CONTAINER*						m_Container;
			typename CONTAINER::iterator	m_Iterator;
			enumLocation					m_Status;
	};

	template <typename CONTAINER>
	class LockingContainer : public CONTAINER
	{
		public:
			inline LockingContainer () :
				CONTAINER	        (),
				m_Lock	            ()
			{
			}
			template <typename arg1>
				inline LockingContainer (arg1 a_Arg1) :
					CONTAINER	        (a_Arg1),
					m_Lock	            ()
			{
			}
			template <typename arg1, typename arg2>
				inline LockingContainer (arg1 a_Arg1, arg2 a_Arg2) :
					CONTAINER	        (a_Arg1, a_Arg2),
					m_Lock              ()
			{
			}
			template <typename arg1, typename arg2, typename arg3>
				inline LockingContainer (arg1 a_Arg1, arg2 a_Arg2, arg3 a_Arg3) :
					CONTAINER	        (a_Arg1, a_Arg2, a_Arg3),
					m_Lock              ()
			{
			}
			template <typename arg1, typename arg2, typename arg3, typename arg4>
				inline LockingContainer (arg1 a_Arg1, arg2 a_Arg2, arg3 a_Arg3, arg4 a_Arg4) :
					CONTAINER           (a_Arg1, a_Arg2, a_Arg3, a_Arg4),
					m_Lock              ()
			{
			}
			template <typename arg1, typename arg2, typename arg3, typename arg4, typename arg5>
				inline LockingContainer (arg1 a_Arg1, arg2 a_Arg2, arg3 a_Arg3, arg4 a_Arg4, arg5 a_Arg5) :
					CONTAINER	        (a_Arg1, a_Arg2, a_Arg3, a_Arg4, a_Arg5),
					m_Lock              ()
			{
			}

			template <typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6>
				inline LockingContainer (arg1 a_Arg1, arg2 a_Arg2, arg3 a_Arg3, arg4 a_Arg4, arg5 a_Arg5, arg6 a_Arg6) :
					CONTAINER           (a_Arg1, a_Arg2, a_Arg3, a_Arg4, a_Arg5, a_Arg6),
					m_Lock              ()
			{
			}

			template <typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6, typename arg7>
				inline LockingContainer (arg1 a_Arg1, arg2 a_Arg2, arg3 a_Arg3, arg4 a_Arg4, arg5 a_Arg5, arg6 a_Arg6, arg7 a_Arg7) :
					CONTAINER           (a_Arg1, a_Arg2, a_Arg3, a_Arg4, a_Arg5, a_Arg6, a_Arg7),
					m_Lock              ()
			{
			}

			template <typename arg1, typename arg2, typename arg3, typename arg4, typename arg5, typename arg6, typename arg7, typename arg8>
				inline LockingContainer (arg1 a_Arg1, arg2 a_Arg2, arg3 a_Arg3, arg4 a_Arg4, arg5 a_Arg5, arg6 a_Arg6, arg7 a_Arg7, arg8 a_Arg8) :
					CONTAINER	        (a_Arg1, a_Arg2, a_Arg3, a_Arg4, a_Arg5, a_Arg6, a_Arg7, a_Arg8),
					m_Lock              ()
			{
			}

		public:
			inline void Lock () const
			{
				m_Lock.Lock();
			}
			inline void Unlock () const
			{
				m_Lock.Unlock();
			}

	private:
			mutable CriticalSection m_Lock;
	};

	template <typename CONTAINER, typename ELEMENT>
	class LockingIterator : public IteratorType<CONTAINER, ELEMENT>
	{
		public:
			LockingIterator	(CONTAINER& container) :
				IteratorType	(container)
			{
				IteratorType<CONTAINER, ELEMENT>::Lock();
			}
			LockingIterator	(CONTAINER& container, typename CONTAINER::iterator startPoint) :
				IteratorType	(container, startPoint)
			{
				IteratorType<CONTAINER, ELEMENT>::Lock();
			}

			LockingIterator	(const LockingIterator<CONTAINER, ELEMENT>& copy) :
				IteratorType	(copy)
			{
				IteratorType<CONTAINER, ELEMENT>::Lock();
			}

			~LockingIterator ()
			{
				IteratorType<CONTAINER, ELEMENT>::Unlock();
			}
	};

} } // namespace Solutions::Generics

#endif __ITERATOR_H
