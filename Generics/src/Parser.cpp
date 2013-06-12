#include "Parser.h"

namespace Solutions { namespace Generics
{

uint32 TextParser::ReadText(const TCHAR delimiters[], const uint32 offset) const
{
	uint32 endPoint = 0;

	if (*Data() == '\"')
	{
		endPoint = ForwardFind (_T("\""), offset + 1);
	}
	else
	{
		endPoint = ForwardFind(delimiters, offset);
	}

	return (endPoint);
}

void TextParser::ReadText (OptionalType<TextFragment>& result, const TCHAR delimiters[])
{
	uint32 marker = ForwardSkip (_T("\t "));

	uint32 endPoint = ReadText(delimiters, marker);

	if ( (endPoint != marker) && (endPoint < Length()) )
	{
		result = TextFragment(*this, marker, endPoint - marker);
		Forward(endPoint);
	}
}

void PathParser::Parse (const TextFragment& input)
{
	TextParser parser (input);
	OptionalType<TextFragment> info;

	parser.ReadText(info, _T(":"));

	if ((info.IsSet()) && (info.Value().Length() == 1))
	{
		m_Drive = *(info.Value().Data());
		parser.Skip(1);
	}

	// Find the last '/ or \ from the back, after the drive
	uint32 index = parser.ReverseFind(_T("\\/"));

	if (index != NUMBER_MAX_UNSIGNED(uint32))
	{
		m_Path = TextFragment(parser, 0, index);
		parser.Skip(index+1);
	}

	// Now we are ate the complete filename
	m_FileName = TextFragment (parser, 0, NUMBER_MAX_UNSIGNED(uint32));

	// Find the extension from the current parser...
	index = parser.ReverseFind(_T("."));

	if (index == NUMBER_MAX_UNSIGNED(uint32))
	{
		// oops there is no extension, BaseFileName == Filename
		m_BaseFileName = m_FileName;
	}
	else
	{
		m_BaseFileName = TextFragment (parser, 0, index);
		m_Extension = TextFragment(parser, index + 1, NUMBER_MAX_UNSIGNED(uint32));
	}
}

} } // namespace Solutions::Generics
