#ifndef __PARSER_H
#define __PARSER_H

// ---- Include system wide include files ----

// ---- Include local include files ----
#include "Portability.h"
#include "Number.h"
#include "Enumerate.h"
#include "OptionalType.h"
#include "KeyValueType.h"
#include "RangeType.h"

namespace Solutions { namespace Generics
{
	class EXTERNAL TextParser : public TextFragment
	{
		private:
			TextParser();
			TextParser(const TextParser&);
			TextParser& operator= (const TextParser&);

		public:
			TextParser (const TextFragment& input) :
				TextFragment(input)
			{
			}
			~TextParser()
			{
			}

		public:
			void ReadText (OptionalType<TextFragment>& result, const TCHAR delimiters[]);

			inline void Skip (const uint32 positions)
			{
				Forward (positions);
			}

			inline void Skip (const TCHAR characters[])
			{
				Forward (ForwardSkip(characters));
			}

			inline void Find (const TCHAR characters[])
			{
				Forward (ForwardFind(characters));
			}

			inline void SkipWhiteSpace ()
			{
				Skip (_T("\t "));
			}

			inline void SkipLine ()
			{
				Jump (_T("\n\r\0"));
			}

			template <const bool CASESENSITIVE> 
			bool Validate (const TextFragment& comparison)
			{
				uint32 marker = ForwardSkip (_T("\t "));

				bool result = ( ((CASESENSITIVE == true)  && (comparison == TextFragment(*this, marker, comparison.Length())))        ||
								((CASESENSITIVE == false) && (comparison.EqualText(TextFragment(*this, marker, comparison.Length())))) );

				if (result == true)
				{
					Forward(marker + comparison.Length());
				}
					
				return (result);
			}

			template <typename NUMBER, const bool SIGNED>
			void ReadNumber (OptionalType<NUMBER>& result, const NumberBase type = BASE_DECIMAL)
			{
				NUMBER			value;

				uint32 marker = ForwardSkip (_T("\t "));

				// Read the number..
				uint32 readChars = Number<NUMBER,SIGNED>(value, marker, type);

				// See if we read a number (did we progress??)
				if (readChars != 0)
				{
					Forward(marker + readChars);
					result = value;
				}
			}

			template <typename ENUMERATE, const bool CASESENSITIVE>
			void ReadEnumerate (OptionalType<ENUMERATE>& result, const TCHAR* delimiters)
			{
				OptionalType<TextFragment>	textEnumerate;

				uint32 marker = ForwardSkip (_T("\t "));

				uint32 progressed = ReadText(delimiters, marker);

				// See if we read a number (did we progress??)
				if (progressed != marker)
				{
					EnumerateType<ENUMERATE> enumerate(TextFragment(*this, marker, progressed - marker) , CASESENSITIVE);

					if (enumerate.IsSet())
					{
						// We have an enumerate, forward..
						Skip(progressed);

						result = enumerate.Value();
					}
				}
			}

			template<const TCHAR DIVIDER, const bool CASESENSITIVE>
			void ReadKeyValuePair (OptionalType<KeyValueType<CASESENSITIVE>>& result, const TCHAR delimiters[])
			{
				uint32 readBytes = ForwardSkip (_T("\t "));
				uint32 marker = readBytes;

				const TCHAR* line = &(Data ()[readBytes]);

				// Walk on till we reach the divider 
				while ((readBytes < Length()) && (*line != DIVIDER) && (_tcschr(delimiters, *line) == NULL))
				{
					line++;
					readBytes++;
				}

				// time to remember the key..
				TextFragment key = TextFragment(*this, marker, readBytes - marker);
				OptionalType<TextFragment> value;

				// And did we end up on the divider...
				if (*line == DIVIDER)
				{
					++readBytes;

					marker = readBytes;

					// Now we found the divider, lets find the value..
					readBytes = ReadText(delimiters, readBytes);

					if (readBytes != marker)
					{
						value = TextFragment(*this, marker, readBytes - marker);
					}
				}

				if (key.Length () != 0)
				{
					// All succeeded, move forward.
					Skip (readBytes);

					result = KeyValueType<CASESENSITIVE>(key, value);
				}
			}

			template <const TCHAR DIVIDER, typename NUMBER, const bool SIGNED, const bool BEGININC, const bool ENDINC>
			void ReadRange (OptionalType<RangeType<NUMBER, BEGININC,ENDINC>>& result, const NumberBase type = BASE_DECIMAL)
			{
				NUMBER minimum;
				NUMBER maximum;

				// Read a number, it might be in..
				uint32 readBytes = Number<NUMBER,SIGNED> (minimum, 0, type);

				if (readBytes == 0)
				{
					minimum = NumberType<NUMBER,SIGNED>::Min();
				}

				// Now skip all white space if it is behind..
				readBytes = ForwardSkip (_T("\t "), readBytes);

				// Now check if we have the DIVIDER.
				if (Data()[readBytes] == DIVIDER)
				{
					readBytes++;

					// Now time to get the maximum.
					uint32 handled = Number<NUMBER,SIGNED>(maximum, readBytes, type);

					if (handled == 0)
					{
						maximum = NumberType<NUMBER,SIGNED>::Max();
					}

					// It succeeded, skip this part..
					Skip(readBytes + handled);

					result = RangeType<NUMBER, BEGININC,ENDINC>(minimum, maximum);
				}
			}

		private:
			inline void Jump (const TCHAR characters[])
			{
				uint32 index = ForwardFind(characters);

				if (index >= Length())
				{
					Forward (Length());
				}
				else
				{
					Forward (ForwardSkip (characters, index));
				}
			}

			uint32 ReadText(const TCHAR delimiters[], const uint32 offset = 0) const;
	};

	class EXTERNAL PathParser 
	{
		private:
			PathParser();
			PathParser(const PathParser&);
			PathParser& operator= (const PathParser&);

		public:
			inline PathParser (const TextFragment& input)
			{
				Parse (input);
			}
			~PathParser()
			{
			}

		public:
			inline const OptionalType<TCHAR>& Drive() const
			{
				return (m_Drive);
			}

			inline const OptionalType<TextFragment>& Path() const
			{
				return (m_Path);
			}
			
			inline const OptionalType<TextFragment>& FileName() const
			{
				return (m_FileName);
			}
			
			inline const OptionalType<TextFragment>& BaseFileName() const
			{
				return (m_BaseFileName);
			}

			inline const OptionalType<TextFragment>& Extension () const
			{
				return (m_Extension);			
			}

		private:
			void Parse (const TextFragment& input);

		private:
			OptionalType<TCHAR>				m_Drive;
			OptionalType<TextFragment>		m_Path;
			OptionalType<TextFragment>		m_FileName;
			OptionalType<TextFragment>		m_BaseFileName;
			OptionalType<TextFragment>		m_Extension;
	};

} } // namespace Solutions::Generics

#endif // __PARSER_H
