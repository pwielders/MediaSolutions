#ifndef __KEYVALUETYPE_H
#define __KEYVALUETYPE_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Portability.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----
namespace Solutions { namespace Generics
{
	template<const bool KEY_CASESENSITIVE>
	class KeyValueType
	{
		public:
			KeyValueType () :
				m_Key    (),
				m_Value  ()
			{
			}
			KeyValueType (const TextFragment& key, const TextFragment& value) :
				m_Key    (key),
				m_Value  (value)
			{
			}
			KeyValueType (const  KeyValueType<KEY_CASESENSITIVE>& copy) :
				m_Key    (copy.m_Key),
				m_Value  (copy.m_Value)
			{
			}
			~KeyValueType()
			{
			}

			KeyValueType<KEY_CASESENSITIVE>& operator= (const KeyValueType<KEY_CASESENSITIVE>& RHS)
			{
				m_Key = RHS.m_Key;
				m_Value = RHS.m_Value;

				return (*this);
			}

			bool operator== (const KeyValueType<KEY_CASESENSITIVE>& RHS) const
			{
				return ( (RHS.m_Key == m_Key) && (RHS.m_Value == m_Value) );
			}

			inline bool operator != (const KeyValueType<KEY_CASESENSITIVE>& RHS) const
			{
				return (!operator==(RHS));
			}

			inline bool HasKey () const
			{
				return (m_Key.IsEmpty() == false);
			}

			inline bool HasValue () const
			{
				return (m_Value.IsEmpty() == false);
			}

			inline bool IsKey (const TextFragment& key) const
			{
				return (KEY_CASESENSITIVE == true ? key == m_Key : key.EqualText(m_Key)) ;
			}

			inline const TextFragment& Key () const
			{
				return (m_Key);
			}

			inline const TextFragment& Text () const
			{
				return (m_Value);
			}

		private:
			TextFragment	m_Key;
			TextFragment	m_Value;
	};

} } // namespace Solutions::Generics

#endif // __KEYVALUETYPE_H