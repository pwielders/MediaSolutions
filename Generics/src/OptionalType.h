#ifndef __OPTIONALTYPE_H
#define __OPTIONALTYPE_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Portability.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----
namespace Solutions { namespace Generics
{
	template<typename TYPE>
	class OptionalType
	{
		public:
			OptionalType () :
				m_Value  (),
				m_Set    (false)
			{
			}

			OptionalType (const TYPE& value) :
				m_Value  (value),
				m_Set    (true)
			{
			}

			OptionalType (const OptionalType<TYPE>& value) :
				m_Value  (value.m_Value),
				m_Set    (value.m_Set)
			{
			}

			~OptionalType()
			{
			}

			inline OptionalType<TYPE>& operator= (const OptionalType<TYPE>& value)
			{
				m_Value = value.m_Value;
				m_Set = value.m_Set;

				return (*this);
			}

			inline OptionalType<TYPE>& operator= (const TYPE& value)
			{
				m_Value = value;
				m_Set = true;

				return (*this);
			}

			inline bool operator== (const TYPE& value) const
			{
				return (value == m_Value);
			}

			inline bool operator== (const OptionalType<TYPE>& value) const
			{
				return ((value.m_Set == m_Set) && operator== (value.m_Value));
			}

			inline bool operator!= (const TYPE& value) const
			{
				return (!operator==(value));
			}

			inline bool operator!= (const OptionalType<TYPE>& value) const
			{
				return (!operator==(value));
			}

		public:
			bool IsSet() const
			{
				return (m_Set);
			}

			operator TYPE& ()
			{
				return (m_Value);
			}

			operator const TYPE& () const
			{
				return (m_Value);
			}

			TYPE& Value ()
			{
				return (m_Value);
			}

			const TYPE& Value() const
			{
				return (m_Value);
			}

		private:
			TYPE	m_Value;
			bool	m_Set;
	};

} } // namespace Solutions::Generics

#endif // __OPTIONALTYPE_H