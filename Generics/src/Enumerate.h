#ifndef __ENUMERATE_H
#define __ENUMERATE_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Number.h"
#include "String.h"
#include "OptionalType.h"

namespace Solutions { namespace Generics
{
	// ---- Referenced classes and types ----

	// ---- Helper types and constants ----
#define ENUM_CONVERSION_TABLE(ENUMERATE)	const Generics::EnumerateConversion<ENUMERATE> Generics::EnumerateType<ENUMERATE>::g_InfraEnumConversion[] =

	// ---- Helper functions ----

	// ---- Class Definition ----
	template <class ENUMERATE>
	struct EnumerateConversion
	{
		ENUMERATE		value;
		const TCHAR*	name;
		uint32			length;
	};

	template <class ENUMERATE>
	class EnumerateType 
	{
		public:
			EnumerateType () : m_Value()
			{
			}
			explicit EnumerateType(const ENUMERATE	Value) : m_Value (Value) 
			{
			}
			explicit EnumerateType(const TCHAR value[], const bool caseSensitive = true) : m_Value () 
			{
				if (caseSensitive)
				{				
					 operator=<true>(value);
				}
				else
				{
					 operator=<false>(value);
				}
			}
			explicit EnumerateType(const Generics::TextFragment& value, const bool caseSensitive = true) : m_Value () 
			{
				if (caseSensitive)
				{				
					 operator=<true>(value);
				}
				else
				{
					 operator=<false>(value);
				}
			}
			explicit EnumerateType(const EnumerateType<ENUMERATE>& copy) : m_Value(copy.m_Value)
			{
			}
			explicit EnumerateType(const uint32	Value) : m_Value()
			{
				operator= (Value);
			}

			virtual ~EnumerateType() 
			{
			}

			EnumerateType<ENUMERATE>& operator= (const EnumerateType<ENUMERATE>& RHS)
			{ 
				m_Value = RHS.m_Value;

				return ( *this );
			}

			EnumerateType<ENUMERATE>& operator= (const ENUMERATE Value)
			{ 
				m_Value = Value;

				return (*this);
			}

			EnumerateType<ENUMERATE>& operator= (const uint32 Value)
			{ 
				const EnumerateConversion<ENUMERATE>*	pEntry = Find(Value);

				if (pEntry == NULL)
				{
					m_Value = OptionalType<ENUMERATE>();
				}
				else
				{
					m_Value = pEntry->value;
				}

				return (*this);
			}

			template<bool CASESENSITIVE>
			EnumerateType<ENUMERATE>& operator= (const Generics::TextFragment& text)
			{ 
				const EnumerateConversion<ENUMERATE>*	pEntry = Find<CASESENSITIVE>(text);

				if (pEntry == NULL)
				{
					m_Value = OptionalType<ENUMERATE>();
				}
				else
				{
					m_Value = pEntry->value;
				}
				return (*this);
 			}

			inline bool IsSet () const
			{  
				return (m_Value.IsSet());
			}

			inline ENUMERATE Value () const
			{
				return (m_Value.Value());
			}

			inline const TCHAR* Text () const
			{
				return (operator const TCHAR* ());
			}

			operator const TCHAR* () const
			{
				const EnumerateConversion<ENUMERATE>*	runner = NULL;

				if (IsSet())
				{
					runner = Find(m_Value.Value());
				}

				return ((runner == NULL) ? _T("") : runner->name);
			}	
			static const TCHAR* Name(const uint32 index)
			{
				return (index < sizeof(g_InfraEnumConversion)/sizeof(EnumerateConversion<ENUMERATE>) ? g_InfraEnumConversion[index].name : NULL);
			}

		public:
			inline bool operator== (const EnumerateType<ENUMERATE>& rhs) const { return (m_Value == rhs.m_Value); }
			inline bool operator!= (const EnumerateType<ENUMERATE>& rhs) const { return (!operator== (rhs)); }
			inline bool operator== (const ENUMERATE& rhs) const { return (m_Value.IsSet()) && (m_Value.Value == rhs); }
			inline bool operator!= (const ENUMERATE& rhs) const { return (!operator== (rhs)); }


		private:
			// Attach a table to this global parameter to get string conversions
			static const EnumerateConversion<ENUMERATE>	g_InfraEnumConversion[];
			OptionalType<ENUMERATE>						m_Value;

		private:
			template<bool CASESENSITIVE>
			const EnumerateConversion<ENUMERATE>* Find(const Generics::TextFragment& value) const
			{
				uint16 length = sizeof(g_InfraEnumConversion)/sizeof(EnumerateConversion<ENUMERATE>);
				const EnumerateConversion<ENUMERATE>* runner = &g_InfraEnumConversion[0];

				if (CASESENSITIVE == true)
				{
					while ((length != 0) && (value != Generics::TextFragment(runner->name, 0, runner->length)))
					{
						runner++;
						length--;
					}
				}
				else
				{
					while ((length != 0) && (value.EqualText(Generics::TextFragment(runner->name, runner->length)) == false))
					{
						runner++;
						length--;
					}
				}

				return ((length != 0) ? runner : NULL);
			}

			const EnumerateConversion<ENUMERATE>* Find(const uint32 value) const
			{
				uint32 length = sizeof(g_InfraEnumConversion)/sizeof(EnumerateConversion<ENUMERATE>);
				const EnumerateConversion<ENUMERATE>*	runner = &g_InfraEnumConversion[0];

				while ((length != 0) && (runner->value != static_cast<ENUMERATE>(value)))
				{
					runner++;
					length--;
				}

				return ((length != 0) ? runner : NULL);
			}
	};

} } // namespace Solutions::Generics

#endif // __ENUMERATE_H
