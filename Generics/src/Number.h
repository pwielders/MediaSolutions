#ifndef __NUMBER_H
#define __NUMBER_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Portability.h"
#include "String.h"

// ---- Referenced classes and types ----

// ---- Helper types and constants ----
 
// ---- Helper functions ----
namespace Solutions { namespace Generics
{
	class EXTERNAL Fractional
	{
		public:
			Fractional();
			Fractional(const sint32& integer, const uint32& remainder);
			Fractional(const Fractional& copy);
			virtual ~Fractional();

			Fractional& operator= (const Fractional& RHS);

		public:
			template<typename FLOATINGPOINTTYPE>
			FLOATINGPOINTTYPE Composit () const
			{
				uint16 count = static_cast<uint16>(log10 ((float) m_Remainder)) + 1;
				uint32 base  = static_cast<uint32>(pow ((float) 10, count)); 

				return( static_cast<FLOATINGPOINTTYPE>(m_Integer) + 
						static_cast<FLOATINGPOINTTYPE>(m_Remainder/base));
			}
			
			inline sint32 Integer () const
			{
				return (m_Integer);
			}

			inline uint32 Remainder () const
			{
				return (m_Remainder);
			}

			String Text (const uint8 decimalPlaces) const;
			bool operator == (const Fractional& RHS) const;
			bool operator != (const Fractional& RHS) const;
			bool operator >= (const Fractional& RHS) const;
			bool operator <= (const Fractional& RHS) const;
			bool operator > (const Fractional& RHS) const;
			bool operator < (const Fractional& RHS) const;

		private:
			sint32		m_Integer;
			uint32		m_Remainder;
	};


	template <class TYPE, bool SIGNED>
	class NumberType
	{
		public:
			NumberType					() : 
					m_Value				(0) 
			{
			}
			NumberType					(
				const TYPE				 Value) : 
					m_Value				(Value) 
			{
			}
			NumberType(
				const TCHAR				 Value[]) : 
					m_Value				(0) 
			{
				if (::TextToNumber<TYPE,SIGNED>::Convert (Value, m_Value) == false)
				{
					NUMBER_VIOLATION (_T("Invalid Number passed !!!!"));
				}
			}
			NumberType(
				const NumberType<TYPE,SIGNED>&  rhs) :
					m_Value						(rhs.m_Value)
			{
			}
			~NumberType() 
			{
			}

			inline NumberType<TYPE, SIGNED>&
				operator= (
					const NumberType<TYPE,SIGNED>& Value) 
			{
				return (NumberType<TYPE, SIGNED>::operator = (Value.m_Value)); 
			}
			inline NumberType<TYPE, SIGNED>&
				operator= (const TYPE Value) 
			{ 
				m_Value = Value; 

				return (*this);
			}

		public:
			operator String () const
			{
				// Max size needed to dreate 
				TCHAR	Buffer[21];
				TCHAR*	Location = &Buffer[20];
				TYPE	Value    = NumberType<TYPE, SIGNED> (m_Value).Abs ();

				// Close it with a terminating character!!
				*Location-- = '\0';

				// Convert the number to a string
				do
				{
					*Location-- = (TCHAR) ((Value % 10) + '0');
					Value = Value / 10;

				} while (Value != 0);

				if (Negative ())
				{
					*Location = '-';
				}
				else
				{
					Location++;
				}

				return (String (Location));
			}
			String Text () const
			{
				return (operator String ());
			}
			inline operator TYPE () const { return (m_Value); }
			inline bool operator== (const NumberType<TYPE, SIGNED>& rhs) const { return (m_Value == rhs.m_Value); }
			inline bool operator!= (const NumberType<TYPE, SIGNED>& rhs) const { return (m_Value != rhs.m_Value); }
			inline bool operator<= (const NumberType<TYPE, SIGNED>& rhs) const { return (m_Value <= rhs.m_Value); }
			inline bool operator>= (const NumberType<TYPE, SIGNED>& rhs) const { return (m_Value >= rhs.m_Value); }
			inline bool operator<  (const NumberType<TYPE, SIGNED>& rhs) const { return (m_Value <  rhs.m_Value); }
			inline bool operator>  (const NumberType<TYPE, SIGNED>& rhs) const { return (m_Value >  rhs.m_Value); }

			inline NumberType<TYPE, SIGNED>  operator+  (const NumberType<TYPE, SIGNED>& rhs) const { return (NumberType<TYPE, SIGNED> (m_Value +  rhs.m_Value)); }
			inline NumberType<TYPE, SIGNED>& operator+= (const NumberType<TYPE, SIGNED>& rhs)		  { m_Value += rhs.m_Value; return (*this); }
			inline NumberType<TYPE, SIGNED>  operator-  (const NumberType<TYPE, SIGNED>& rhs) const { return (NumberType<TYPE, SIGNED> (m_Value -  rhs.m_Value)); }
			inline NumberType<TYPE, SIGNED>& operator-= (const NumberType<TYPE, SIGNED>& rhs)		  { m_Value -= rhs.m_Value; return (*this); }
			inline NumberType<TYPE, SIGNED>  operator*  (const NumberType<TYPE, SIGNED>& rhs) const { return (NumberType<TYPE, SIGNED> (m_Value *  rhs.m_Value)); }
			inline NumberType<TYPE, SIGNED>& operator*= (const NumberType<TYPE, SIGNED>& rhs)		  { m_Value *= rhs.m_Value; return (*this); }
			inline NumberType<TYPE, SIGNED>  operator/  (const NumberType<TYPE, SIGNED>& rhs) const { return (NumberType<TYPE, SIGNED> (m_Value /  rhs.m_Value)); }
			inline NumberType<TYPE, SIGNED>& operator/= (const NumberType<TYPE, SIGNED>& rhs)		  { m_Value /= rhs.m_Value; return (*this); }
			inline NumberType<TYPE, SIGNED>  operator+  (const TYPE Number)					const { return (NumberType<TYPE, SIGNED> (m_Value +  Number)); }
			inline NumberType<TYPE, SIGNED>& operator+= (const TYPE Number)						  { m_Value += Number; return (*this); }
			inline NumberType<TYPE, SIGNED>  operator-  (const TYPE Number)					const { return (NumberType<TYPE, SIGNED> (m_Value -  Number)); }
			inline NumberType<TYPE, SIGNED>& operator-= (const TYPE Number)						  { m_Value -= Number; return (*this); }
			inline NumberType<TYPE, SIGNED>  operator*  (const TYPE Number)					const { return (NumberType<TYPE, SIGNED> (m_Value *  Number)); }
			inline NumberType<TYPE, SIGNED>& operator*= (const TYPE Number)						  { m_Value *= Number; return (*this); }
			inline NumberType<TYPE, SIGNED>  operator/  (const TYPE Number)					const { return (NumberType<TYPE, SIGNED> (m_Value /  Number)); }
			inline NumberType<TYPE, SIGNED>& operator/= (const TYPE Number)						  { m_Value /= Number; return (*this); }

			static const TYPE Min () { return (TypedMin (TemplateIntToType<SIGNED>())); }
			static const TYPE Max () { return (TypedMax (TemplateIntToType<SIGNED>())); }
			inline const bool Negative () const { return (TypedNegative (TemplateIntToType<SIGNED>())); }
			inline const TYPE Abs () const { return (TypedAbs (TemplateIntToType<SIGNED>())); }

		private:
			inline const TYPE TypedAbs (const TemplateIntToType<true>&	/* For compile time diffrentiation */) const 
			{ 
				return (m_Value < 0 ? -m_Value : m_Value);  
			}
			inline const TYPE TypedAbs (const TemplateIntToType<false>&	/* For compile time diffrentiation */) const 
			{ 
				return (m_Value); 
			}
			inline const bool TypedNegative (const TemplateIntToType<true>&	/* For compile time diffrentiation */) const 
			{ 
				return (m_Value < 0);  
			}
			inline const bool TypedNegative (const TemplateIntToType<false>&	/* For compile time diffrentiation */) const 
			{ 
				return (false); 
			}
			static const TYPE TypedMin (const TemplateIntToType<false>&	/* For compile time diffrentiation */) 
			{ 
				return (NUMBER_MIN_UNSIGNED(TYPE));  
			}
			static const TYPE TypedMax (const TemplateIntToType<false>&	/* For compile time diffrentiation */) 
			{ 
				return (NUMBER_MAX_UNSIGNED(TYPE)); 
			}
			static const TYPE TypedMin (const TemplateIntToType<true>&	/* For compile time diffrentiation */) 
			{ 
				return (NUMBER_MIN_SIGNED(TYPE));  
			}
			static const TYPE TypedMax (const TemplateIntToType<true>&	/* For compile time diffrentiation */) 
			{ 
				return (NUMBER_MAX_SIGNED(TYPE)); 
			}

		private:
			TYPE		m_Value;
	};

	template <const bool SIGNED>
	class NumberType<Fractional, SIGNED>
	{
		public:
			NumberType					() : 
					m_Value				(0) 
			{
			}
			NumberType					(
				const Fractional		 Value) : 
					m_Value				(Value) 
			{
			}
			~NumberType() 
			{
			}

		public:
			static const Fractional Min () { return (TypedMin (TemplateIntToType<SIGNED>())); }
			static const Fractional Max () { return (TypedMax (TemplateIntToType<SIGNED>())); }

		private:
			static const Fractional TypedMin (const TemplateIntToType<false>&	/* For compile time diffrentiation */) 
			{ 
				return (Fractional(0,0));  
			}
			static const Fractional TypedMax (const TemplateIntToType<false>&	/* For compile time diffrentiation */) 
			{ 
				return (Fractional(NUMBER_MAX_SIGNED(sint32),NUMBER_MAX_UNSIGNED(uint32))); 
			}
			static const Fractional TypedMin (const TemplateIntToType<true>&	/* For compile time diffrentiation */) 
			{ 
				return (Fractional(NUMBER_MIN_SIGNED(sint32),NUMBER_MAX_UNSIGNED(uint32)));  
			}
			static const Fractional TypedMax (const TemplateIntToType<true>&	/* For compile time diffrentiation */) 
			{ 
				return (Fractional(NUMBER_MAX_SIGNED(sint32),NUMBER_MAX_UNSIGNED(uint32))); 
			}

		private:
			Fractional		m_Value;
	};

	typedef NumberType<uint8,false>    NumberUnsigned8;
	typedef NumberType<sint8,true>     NumberSigned8;
	typedef NumberType<uint16,false>   NumberUnsigned16;
	typedef NumberType<sint16,true>    NumberSigned16;
	typedef NumberType<uint32,false>   NumberUnsigned32;
	typedef NumberType<sint32,true>    NumberSigned32;
	typedef NumberType<uint64,false>   NumberUnsigned64;
	typedef NumberType<sint64,true>    NumberSigned64;

} } // namespace Solutions::Generics

#endif // __NUMBER_H