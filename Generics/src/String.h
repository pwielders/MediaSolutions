// ===========================================================================
//
// Filename:    String.h
//
// Description: Header file for the String functions. 
//
// History
//
// Author        Reason                                             Date
// ---------------------------------------------------------------------------
// P. Wielders   Initial creation                                   2002/05/24
//
// ===========================================================================

#ifndef __STRING_H
#define __STRING_H

#include <string>

#include "Module.h"

namespace Solutions { namespace Generics
{
#ifdef __WIN32__

#include <TCHAR.h>

#ifdef  _UNICODE
typedef std::wstring	String;
#endif

#ifndef _UNICODE
typedef std::string		String;
#endif

#endif // __WIN32__

#ifdef __LINUX__

#ifdef  _UNICODE
#define _T(x)      L ## x
#define TCHAR	   wchar_t
#endif

#ifndef _UNICODE
#define _T(x)      x
#define TCHAR	   char
#endif

#endif // __LINUX__

#ifdef __DEC__

#ifdef  _UNICODE
#define _T(x)      L ## x
#define TCHAR	   wchar_t
#endif

#ifndef _UNICODE
#define _T(x)      x
#define TCHAR	   char
#endif

#endif // __DEC__

void         EXTERNAL ToString (const TCHAR   realString[], std::string& result);
void         EXTERNAL ToString (const TCHAR   realString[], std::wstring& result);
const String EXTERNAL ToString (const char    realString[]);
const String EXTERNAL ToString (const wchar_t realString[]);
const String EXTERNAL ToString (const char    realString[], const unsigned int length);
const String EXTERNAL ToString (const wchar_t realString[], const unsigned int length);

#ifdef  _UNICODE
inline int ToCharacter (const char* character, TCHAR converted[], unsigned int count)
#else
inline int ToCharacter (const char* character, TCHAR converted[], unsigned int /* count */)
#endif
{
#ifdef  _UNICODE
	return (::mbtowc(converted, character, count));
#else
	converted[0] = *character;
	return(1);
#endif
}

inline int ToCharacter (const wchar_t* character, TCHAR converted[], unsigned int /* DUMMY JUST TO HAVE THE SAME IF */)
{
#ifdef  _UNICODE
	converted[0] = *character;
	return(1);
#else
	#pragma warning(disable : 4996)
	return (::wctomb(converted, character[0]));
	#pragma warning(default : 4996)

#endif
}

//inline int ToCharacter (const TCHAR* character, char converted[], unsigned int /* DUMMY JUST TO HAVE THE SAME IF */)
//{
//#ifdef  _UNICODE
//	return (::wctomb(converted, character[0]));
//#else
//	converted[0] = *character;
//	return(1);
//#endif
//}
//
//#ifdef  _UNICODE
//inline int ToCharacter (const TCHAR* character, wchar_t converted[], unsigned int /* count */ )
//#else
//inline int ToCharacter (const TCHAR* character, wchar_t converted[], unsigned int count)
//#endif
//{
//#ifdef  _UNICODE
//	converted[0] = *character;
//	return(1);
//#else
//	#pragma warning(disable : 4996)
//	return (::mbtowc(converted, character[0], count));
//	#pragma warning(default : 4996)
//
//#endif
//}

#define EMPTY_STRING  _T("")

	class Fractional;

	class EXTERNAL TextFragment
	{
		protected:
			class Index 
			{
				public:
					inline Index (uint32 begin, uint32 length) :
						m_Begin(begin),
						m_Length(length)
					{
					}
					inline Index (const Index& copy, uint32 offset, uint32 length) :
						m_Begin(copy.m_Begin + offset)
					{
						if (length == NUMBER_MAX_UNSIGNED(uint32))
						{
							ASSERT (offset <= copy.m_Length); 
				
							m_Length = copy.m_Length - offset;
						}
						else
						{
							ASSERT ((length+offset) <= copy.m_Length);
							m_Length = length;
						}
					}
					inline Index (const Index& copy) :
						m_Begin(copy.m_Begin),
						m_Length(copy.m_Length)
					{
					}
					inline ~Index()
					{
					}

					inline Index& operator= (const Index& RHS)
					{
						m_Begin  = RHS.m_Begin;
						m_Length = RHS.m_Length;

						return (*this);
					}

					inline uint32 Begin() const
					{
						return (m_Begin);
					}

					inline uint32 End() const
					{
						return (m_Begin + m_Length);
					}

					inline uint32 Length() const
					{
						return (m_Length);
					}

					inline void Increment (const uint32 offset)
					{
						if (offset > m_Length)
						{
							m_Begin += m_Length;
							m_Length = 0;
						}
						else
						{
							m_Begin  += offset;
							m_Length -= offset;
						}
					}

					inline void Decrement (const uint32 offset)
					{
						if (offset > m_Begin)
						{
							m_Length += m_Begin;
							m_Begin   = 0;
						}
						else
						{
							m_Begin  -= offset;
							m_Length += offset;
						}
					}

				private:
					uint32			m_Begin;
					uint32			m_Length;
			};

		public:
			TextFragment () :
				m_Index  (0,0),
				m_Start  (NULL),
				m_Buffer ()
			{
			}
			explicit TextFragment (const TCHAR text[]) :
				m_Index  (0,_tcslen(text)),
				m_Start	 (text),
				m_Buffer ()
			{
			}
			explicit TextFragment (const TCHAR text[], const uint32 length) :
				m_Index  (0,length),
				m_Start	 (text),
				m_Buffer ()
			{
			}
			TextFragment (const TCHAR text[], const uint32 offset, const uint32 length) :
				m_Index  (offset,length),
				m_Start	 (text),
				m_Buffer ()
			{
				ASSERT (m_Index.End() <= _tcslen(text));
			}
			explicit TextFragment (const String& text) :
				m_Index  (0,text.length()),
				m_Start	 (NULL),
				m_Buffer (text)
			{
			}
			TextFragment (const String& text, const uint32 offset, const uint32 length) :
				m_Index  (offset,length),
				m_Start	 (NULL),
				m_Buffer (text)
			{
				ASSERT (m_Index.End() <= text.length());
			}
			TextFragment (const TextFragment& base, const uint32 offset, const uint32 length) :
				m_Index  (base.m_Index, offset, length),
				m_Start	 (base.m_Start),
				m_Buffer (base.m_Buffer)
			{

			}
			TextFragment (const TextFragment& copy) :
				m_Index  (copy.m_Index),
				m_Start	 (copy.m_Start),
				m_Buffer (copy.m_Buffer)
			{
			}
			~TextFragment()
			{
			}

			TextFragment& operator= (const TextFragment& RHS)
			{
				m_Index = RHS.m_Index;
				m_Start = RHS.m_Start;
				m_Buffer = RHS.m_Buffer;

				return (*this);
			}

			inline bool operator== (const TextFragment& RHS) const
			{
				return (equal_case_sensitive(RHS));
			}

			inline bool operator== (const TCHAR RHS[]) const
			{
				return (equal_case_sensitive(TextFragment(RHS, 0, _tcslen(RHS))));
			}

			inline bool operator== (const String& RHS) const
			{
				return (equal_case_sensitive(TextFragment(RHS)));
			}

			inline bool operator != (const TextFragment& RHS) const
			{
				return (!operator==(RHS));
			}

			inline bool operator != (const TCHAR RHS[]) const
			{
				return (!operator==(RHS));
			}

			inline bool OnMarker(const TCHAR characters[]) const
			{
				return (on_given_character(0, characters));
			}


			inline bool IsEmpty() const
			{
				return (m_Index.Length() == 0);
			}

			inline uint32 Length () const
			{
				return (m_Index.Length());
			}

			const TCHAR* Data() const
			{
				return (m_Start == NULL ? &(m_Buffer[m_Index.Begin()]) : &m_Start[m_Index.Begin()]);
			}

			const String Text () const
			{
				return (m_Start == NULL ? (((m_Index.Begin() == 0) && (m_Index.Length() == m_Buffer.length())) ? m_Buffer : m_Buffer.substr(m_Index.Begin(), m_Index.Length())) :
										  String(&(m_Start[m_Index.Begin()]), m_Index.Length()));
			}

			inline bool EqualText(const TextFragment& RHS) const
			{
				return (equal_case_insensitive(RHS));
			}

			template <typename NUMBER, bool SIGNED>
			inline uint32 Number (NUMBER& Value, const uint32 offset = 0, const NumberBase Type = BASE_UNKNOWN) const
			{
				return (Convert<NUMBER> (offset, Value, Type, TemplateIntToType<SIGNED>()));
			}

			inline uint32 ForwardFind (const TCHAR delimiter[], const uint32 offset = 0) const
			{
				return (find_first_of (offset, delimiter));
			}

			inline uint32 ForwardSkip (const TCHAR delimiter[], const uint32 offset = 0) const
			{
				return (find_first_not_of (offset, delimiter));
			}

			inline uint32 ReverseFind (const TCHAR delimiter[], const uint32 offset = 0) const
			{
				return (find_last_of (offset, delimiter));
			}

			inline uint32 ReverseSkip (const TCHAR delimiter[], const uint32 offset = 0) const
			{
				return (find_last_not_of (offset, delimiter));
			}

		protected:
			inline void Forward (const uint32 forward)
			{
				m_Index.Increment(forward);
			}

			inline void Reverse (const uint32 reverse)
			{
				m_Index.Decrement(reverse);
			}

			inline const Index& PartIndex () const
			{
				return (m_Index);
			}

		private:
			template <typename NUMBER>
			uint32
				Convert (
					const uint32					offset,
					NUMBER&							Value,
					const NumberBase				Type,
					const TemplateIntToType<true>&	/* For compile time diffrentiation */) const
			{			
				/* Do the conversion from string to the proper number. */
				bool			Success   = true;
				const TCHAR*	Start     = &(Data()[offset]);
				const TCHAR*	Text      = Start;

				// We start at 0
				Value = 0;

				if (offset < m_Index.Length())
				{
					bool			Sign      = false;
					NumberBase		Base	  = Type;
					NUMBER			Max		  = NUMBER_MAX_SIGNED(NUMBER);

					// Convert the number until we reach the 0 character.
					while ( (Success == true) && (*Text != '\0') )
					{
						if ( (Value == 0) && (*Text == '0') && (Base == BASE_UNKNOWN) )
						{
							// Base change, move over to an OCTAL conversion
							Base = BASE_OCTAL;
						}
						else if ( (Value == 0) && (_totupper(*Text) == 'X') && (Base == BASE_OCTAL) )
						{
							// Base change, move over to an HEXADECIMAL conversion
							Base = BASE_HEXADECIMAL;
						}
						else if ( (Value == 0) && ((*Text == '+') || ((*Text == '-')) || (*Text == ' ') || (*Text == '\t') || (*Text == '0')) )
						{
							// Is it a sign change ???
							if ( *Text == '-' ) { Sign = true; Max = NUMBER_MIN_SIGNED(NUMBER); }
						}
						else 
						{
							if (Base == BASE_UNKNOWN)
							{
								Base = BASE_DECIMAL;
							}

							if ( (*Text >= '0') && (*Text <='7') )
							{
								if (Sign)
								{
									sint8	Digit = ('0' - *Text);

									if ( (Value >= (Max / Base)) && (Digit >= (Max - (Value * Base))) )
									{
										Value = (Value * Base) + Digit;
									}
									else
									{
										Success = false;
									}
								}
								else
								{
									sint8	Digit = (*Text - '0');

									if ( (Value <= (Max / Base)) && (Digit <= (Max - (Value * Base))) )
									{
										Value = (Value * Base) + Digit;
									}
									else
									{
										Success = false;
									}
								}
							}
							else if ( (*Text >= '8') && (*Text <='9') && (Base != BASE_OCTAL) )
							{
								if (Sign)
								{
									sint8	Digit = ('0' - *Text);

									if ( (Value >= (Max / Base)) && (Digit >= (Max - (Value * Base))) )
									{
										Value = (Value * Base) + Digit;
									}
									else
									{
										Success = false;
									}
								}
								else
								{
									sint8	Digit = (*Text - '0');

									if ( (Value <= (Max / Base)) && (Digit <= (Max - (Value * Base))) )
									{
										Value = (Value * Base) + Digit;
									}
									else
									{
										Success = false;
									}
								}
							}
							else if ( (_totupper(*Text) >= 'A') && (_totupper(*Text) <='F') && (Base == BASE_HEXADECIMAL) )
							{
								if (Sign)
								{
									sint8	Digit = static_cast<sint8>('A' - _totupper(*Text) - 10);

									if ( (Value >= (Max / Base)) && (Digit >= (Max - (Value * Base))) )
									{
										Value = (Value * Base) + Digit;
									}
									else
									{
										Success = false;
									}
								}
								else
								{
									sint8	Digit = static_cast<sint8>(_totupper(*Text) - 'A' + 10);

									if ( (Value <= (Max / Base)) && (Digit <= (Max - (Value * Base))) )
									{
										Value = (Value * Base) + Digit;
									}
									else
									{
										Success = false;
									}
								}
							}
							else
							{
								Success = false;
							}
						}

						// Get the next character in line.
						if (Success)
						{
							Text++;
						}
					}
				}

				return (Text - Start);
			}

			template <typename NUMBER>
			uint32
				Convert (
					const uint32					offset,
					NUMBER&							Value,
					const NumberBase				Type,
					const TemplateIntToType<false>&	/* For compile time diffrentiation */) const
			{
				/* Do the conversion from string to the proper number. */
				bool			Success   = true;
				const TCHAR*	Start     = &(Data()[offset]); 
				const TCHAR*	Text      = Start;

				// We start at 0
				Value = 0;

				if (offset < m_Index.Length())
				{
					NumberBase		Base	  = Type;
					NUMBER			Max		  = NUMBER_MAX_UNSIGNED(NUMBER);

					// Convert the number until we reach the 0 character.
					while ( (Success == true) && (*Text != '\0') )
					{
						if ( (Value == 0) && (*Text == '0') && (Base == BASE_UNKNOWN) )
						{
							// Base change, move over to an OCTAL conversion
							Base = BASE_OCTAL;
						}
						else if ( (Value == 0) && (_totupper(*Text) == 'X') && (Base == BASE_OCTAL) )
						{
							// Base change, move over to an HEXADECIMAL conversion
							Base = BASE_HEXADECIMAL;
						}
						else if ( (Value == 0) && ((*Text == '+') || (*Text == ' ') || (*Text == '\t') || (*Text == '0')) )
						{
							// Skip all shit and other white spaces
						}
						else
						{
							if (Base == BASE_UNKNOWN)
							{
								Base = BASE_DECIMAL;
							}

							if ( (*Text >= '0') && (*Text <='7') )
							{
								uint8	Digit = (*Text - '0');

								if ( (Value <= (Max / Base)) && (Digit <= (Max - (Value * Base))) )
								{
									Value = (Value * static_cast<uint8>(Base)) + Digit;
								}
								else
								{
									Success = false;
								}
							}
							else if ( (*Text >= '8') && (*Text <='9') && (Base != BASE_OCTAL)  )
							{
								uint8	Digit = (*Text - '0');

								if ( (Value <= (Max / Base)) && (Digit <= (Max - (Value * Base))) )
								{
									Value = (Value * static_cast<uint8>(Base)) + Digit;
								}
								else
								{
									Success = false;
								}
							}
							else if ( (_totupper(*Text) >= 'A') && (_totupper(*Text) <='F') && (Base == BASE_HEXADECIMAL) )
							{
								uint8	Digit = static_cast<uint8>(_totupper(*Text) - 'A' + 10);

								if ( (Value <= (Max / Base)) && (Digit <= (Max - (Value * Base))) )
								{
									Value = (Value * static_cast<uint8>(Base)) + Digit;
								}
								else
								{
									Success = false;
								}
							}
							else
							{
								Success = false;
							}
						}

						// Get the next character in line.
						if (Success)
						{
							Text++;
						}
					}
				}

				return (Text - Start);
			}

			template<>
			uint32
				Convert<Fractional> (
					const uint32			offset,
					Fractional&				Value,
					const NumberBase		Type,
					const TemplateIntToType<false>&	/* For compile time diffrentiation */) const;

			template<>
			uint32
				Convert<Fractional> (
					const uint32			offset,
					Fractional&				Value,
					const NumberBase		Type,
					const TemplateIntToType<true>&	/* For compile time diffrentiation */) const;

			bool on_given_character (const uint32 offset, const TCHAR characters[]) const
			{
				bool equal = false;

				if (offset < m_Index.Length())
				{
					if (m_Start == NULL)
					{
						equal = (_tcschr(characters, m_Buffer[m_Index.Begin() + offset]) != NULL);
					}
					else
					{
						equal = (_tcschr(characters, m_Start[m_Index.Begin() + offset]) != NULL);
					}
				}

				return (equal);
			}

			bool equal_case_sensitive (const TextFragment& RHS) const
			{
				bool equal = false;

				if (RHS.m_Index.Length() == m_Index.Length())
				{
					if (m_Start != NULL)
					{
						if (RHS.m_Start == NULL)
						{
							equal = (RHS.m_Buffer.compare(RHS.m_Index.Begin(), RHS.m_Index.Length(), &(m_Start[m_Index.Begin()]), m_Index.Length()) == 0);
						}
						else
						{
							equal = (_tcsncmp(&(RHS.m_Start[RHS.m_Index.Begin()]),  &(m_Start[m_Index.Begin()]),  RHS.m_Index.Length()) == 0);
						}
					}
					else
					{
						if (RHS.m_Start == NULL)
						{
							equal = (m_Buffer.compare(m_Index.Begin(), m_Index.Length(), RHS.m_Buffer, RHS.m_Index.Begin(), RHS.m_Index.Length()) == 0);
						}
						else
						{
							equal = (m_Buffer.compare(m_Index.Begin(), m_Index.Length(), &(RHS.m_Start[RHS.m_Index.Begin()]), RHS.m_Index.Length()) == 0);
						}
					}
				}

				return (equal);
			}

			bool equal_case_insensitive (const TextFragment& RHS) const
			{
				bool equal = false;

				if (RHS. m_Index.Length() ==  m_Index.Length())
				{
					if (m_Start != NULL)
					{
						if (RHS.m_Start == NULL)
						{
							equal = (_tcsnicmp(&(RHS.m_Buffer[RHS.m_Index.Begin()]), &(m_Start[m_Index.Begin()]),  m_Index.Length()) == 0);
						}
						else
						{
							equal = (_tcsnicmp(&(RHS.m_Start[RHS.m_Index.Begin()]), &(m_Start[m_Index.Begin()]),   m_Index.Length()) == 0);
						}
					}
					else
					{
						if (RHS.m_Start == NULL)
						{
							equal = (_tcsnicmp(&(RHS.m_Buffer[RHS.m_Index.Begin()]), &(m_Buffer[m_Index.Begin()]),  m_Index.Length()) == 0);
						}
						else
						{
							equal = (_tcsnicmp(&(RHS.m_Start[RHS.m_Index.Begin()]), &(m_Buffer[m_Index.Begin()]),  m_Index.Length()) == 0);
						}
					}
				}

				return (equal);
			}

			uint32 find_first_of (const uint32 offset, const TCHAR delimiter[]) const
			{
				// If we do not find it, we end up at the end.
				uint32 index = m_Index.Length();

				if (offset < m_Index.Length())
				{
					if (m_Start != NULL)
					{
						const TCHAR* pointer = &(Data()[offset]);
						uint32 length = m_Index.Length() - offset;

						while ((length != 0) && ((_tcschr(delimiter, *pointer) == NULL)))
						{
							pointer++;
							length--;
						}

						index = m_Index.Length() - length;
					}
					else
					{
						uint32 found = m_Buffer.find_first_of(delimiter, m_Index.Begin() + offset);

						if (found <=  m_Index.End())
						{
							index = found - m_Index.Begin();
						}
					}
				}

				return (index);
			}

			uint32 find_last_of (const uint32 offset, const TCHAR delimiter[]) const
			{
				// If we do not find it, we end up at the end.
				uint32 index = NUMBER_MAX_UNSIGNED(uint32);

				if (offset < m_Index.Length())
				{
					if (m_Start != NULL)
					{
						const TCHAR* pointer = &(Data()[m_Index.Length()]);
						uint32 count = m_Index.Length()-offset;

						while ((count != 0) && ((_tcschr(delimiter, *(--pointer)) == NULL)))
						{
							count--;
						}
						if (count != 0)
						{
							index = count - 1;
						}
					}
					else
					{
						uint32 found = m_Buffer.find_last_of(delimiter, m_Index.End());

						if ( (found != m_Index.End()) && (found >= (m_Index.Begin() + offset)) )
						{
							index = found - m_Index.Begin();
						}
					}
				}

				return (index);
			}

			uint32 find_first_not_of (const uint32 offset, const TCHAR delimiter[]) const
			{
				// If we do not find it, we end up at the end.
				uint32 index = m_Index.Length();

				if (offset < m_Index.Length())
				{
					if (m_Start != NULL)
					{
						const TCHAR* pointer = &(Data()[offset]);
						uint32 length = m_Index.Length() - offset;

						while ((length != 0) && ((_tcschr(delimiter, *pointer) != NULL)))
						{
							pointer++;
							length--;
						}

						index = m_Index.Length() - length;
					}
					else
					{
						uint32 found = m_Buffer.find_first_not_of(delimiter, m_Index.Begin() + offset);

						if (found <=  m_Index.End())
						{
							index = found - m_Index.Begin();
						}
					}
				}

				return (index);
			}

			uint32 find_last_not_of (const uint32 offset, const TCHAR delimiter[]) const
			{
				// If we do not find it, we end up at the end.
				uint32 index = NUMBER_MAX_UNSIGNED(uint32);

				if (offset < m_Index.Length())
				{
					if (m_Start != NULL)
					{
						const TCHAR* pointer = &(Data()[m_Index.Length()]);
						uint32 count = m_Index.Length()-offset;

						while ((count != 0) && ((_tcschr(delimiter, *(--pointer)) != NULL)))
						{
							count--;
						}
						if (count != 0)
						{
							index = count - 1;
						}
					}
					else
					{
						uint32 found = m_Buffer.find_last_not_of(delimiter, m_Index.End());

						if ((found >= (m_Index.Begin() + offset)) && (found != m_Index.End()))
						{
							index = found - m_Index.Begin();
						}
					}
				}

				return (index);
			}

		private:
			String			m_Buffer;
			const TCHAR*	m_Start;
			Index			m_Index;
	};

} } // namespace Solutions::Generics

#endif // __STRING_H