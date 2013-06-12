#ifndef __DATAELEMENTITERATOR_H
#define __DATAELEMENTITERATOR_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Portability.h"
#include "DataElement.h"
#include "SlottedStorage.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----

// ---- Class Definition ----
namespace Solutions { namespace Generics
{
	template <typename DATATYPE, typename POLICY>
	class DataElementIteratorType
	{
		private:
			enum
			{
				BEGIN,
				END,
				VALID

			} m_Location;

		public:
			inline DataElementIteratorType () :
				m_Current(),
				m_Location (BEGIN),
				m_Policy ()
			{
			}
			template <typename arg1>
			inline DataElementIteratorType (arg1& a_Argument1) :
				m_Current(),
				m_Location (BEGIN),
				m_Policy (a_Argument1)
			{
			}
			template <typename arg1, typename arg2>
			inline DataElementIteratorType (arg1& a_Argument1, arg2& a_Argument2) :
				m_Current(),
				m_Location (BEGIN),
				m_Policy (a_Argument1, a_Argument2)
			{
			}
			template <typename arg1, typename arg2, typename arg3>
			inline DataElementIteratorType (arg1& a_Argument1, arg2& a_Argument2, arg3& a_Argument3) :
				m_Current(),
				m_Location (BEGIN),
				m_Policy (a_Argument1, a_Argument2, a_Argument3)
			{
			}
			template <typename arg1, typename arg2, typename arg3, typename arg4>
			inline DataElementIteratorType (arg1& a_Argument1, arg2& a_Argument2, arg3& a_Argument3, arg4 a_Argument4) :
				m_Current(),
				m_Location (BEGIN),
				m_Policy (a_Argument1, a_Argument2, a_Argument3, a_Argument4)
			{
			}
			inline DataElementIteratorType (const DataElementIteratorType<DATATYPE,POLICY>& RHS) :
				m_Current (RHS.m_Current),
				m_Location (RHS.m_Location),
				m_Policy(RHS.m_Policy)
			{
			}
			virtual ~DataElementIteratorType()
			{
			}

			DataElementIteratorType& operator= (const DataElementIteratorType<DATATYPE,POLICY>& RHS)
			{
				m_Current = RHS.m_Current;
				m_Location = RHS.m_Location;
				m_Policy = RHS.m_Policy;

				return (*this);
			}

			bool IsValid () const
			{
				return (m_Location == VALID);
			}

			void Reset ()
			{
				m_Location = BEGIN;
			}

			bool Next ()
			{
				if (m_Location == BEGIN)
				{
					if (m_Policy.First(m_Current) == false)
					{
						m_Location = END;
					}
					else
					{
						m_Location = VALID;
					}
				}
				else if (m_Location == VALID)
				{
					// See if we should continue with a new block, and if so 
					// what the block should look like..
					if (m_Policy.Next(m_Current) == false)
					{
						// Oops we are out of items..
						m_Location = END;
					}
				}

				return (m_Location == VALID);
			}

			inline DataElement& Data ()
			{
				return (m_Current);
			}

		private:
			DATATYPE			m_Root;
			DATATYPE			m_Current;

			POLICY				m_Policy;		
	};

	class EXTERNAL LinkedDataElementIteratorPolicy
	{
		private:

		public:  
			inline LinkedDataElementIteratorPolicy () : m_Root ()
			{
			}
			inline LinkedDataElementIteratorPolicy (const LinkedDataElement& firstOne) : 
				m_Root (firstOne),
				m_Size (NUMBER_MAX_UNSIGNED(uint64)), 
				m_SizeLeft (NUMBER_MAX_UNSIGNED(uint64))
			{
			}
			inline LinkedDataElementIteratorPolicy (const LinkedDataElement& firstOne, const uint64 offset, const uint64 size = NUMBER_MAX_UNSIGNED(uint64)) : 
				m_Root (firstOne, offset),
				m_Size (size),
				m_SizeLeft (size)
			{
			}
			LinkedDataElementIteratorPolicy(const LinkedDataElementIteratorPolicy& RHS) :
				m_Root(RHS.m_Root),
				m_Size(RHS.m_Size),
				m_SizeLeft(RHS.m_SizeLeft)
			{
			}
			inline ~LinkedDataElementIteratorPolicy()
			{
			}

			LinkedDataElementIteratorPolicy& operator= (const LinkedDataElementIteratorPolicy& RHS)
			{
				m_Root = RHS.m_Root;
				m_Size = RHS.m_Size;
				m_SizeLeft = RHS.m_SizeLeft;

				return (*this);
			}

		public:
			inline bool First (LinkedDataElement& firstOne)
			{
				firstOne = m_Root;
				m_SizeLeft = m_Size;

				return (m_Root.Size() != 0);
			}
			inline bool Next (LinkedDataElement& current)
			{
				bool result = ( (current.Enclosed() != NULL) && (current.Size() < m_SizeLeft));

				if (result == true)
				{
					m_SizeLeft -= current.Size();
					current = *(current.Enclosed());

					if (current.Size () > m_SizeLeft)
					{
						current = LinkedDataElement (current, 0,m_SizeLeft);
					}
				}
				return (result);
			}

		private:
			uint64				m_Size;
			uint64				m_SizeLeft;
			LinkedDataElement	m_Root;
	};

	// This is a shallow iterator, the producer is responsible for keeping the data alive while we 
	// iterate over the source..
	typedef DataElementIteratorType<LinkedDataElement, LinkedDataElementIteratorPolicy> DataElementIterator;

} } // namespace Solutions::Generics

#endif // __DATAELEMENTITERATOR_H